"""
Battery Management - Transformer Model Definition & Windowed Dataset Builder
======================================================================================
Small Transformer encoder for 24-hour dSocdt trajectory prediction, following
the architecture notes: Embedding -> Positional Encoding -> N x (Multi-Head
Self-Attention -> Add&Norm -> Feed-Forward -> Add&Norm) -> pooling -> Dense.

Sequence definition (same 14-day window used throughout the XGBoost work):
  input  = 14 days x 48 features/day (24 dSocdt_h + 24 Soc_h - NO per-day date
           features; attention over an identical 4-number block repeated 14x
           carries no sequence information, so date context is instead
           appended once, AFTER pooling, for the day actually being predicted)
  target = 24 dSocdt values for the day immediately after the window

Requires: pip install torch
"""

import numpy as np
import pandas as pd
import torch
import torch.nn as nn

HOURS = list(range(24))
DSOCDT_COLS = [f"dSocdt_h{h}" for h in HOURS]
SOC_COLS    = [f"Soc_h{h}"    for h in HOURS]
N_DAY_FEATS = len(DSOCDT_COLS) + len(SOC_COLS)   # 48
N_TARGET_DATE_FEATS = 4                          # dow, dom, month, year of the PREDICTED day
SEQ_LEN = 14
OUTPUT_DIM = 24


# ─────────────────────────────────────────────────────────────
# MODEL
# ─────────────────────────────────────────────────────────────

class PositionalEncoding(nn.Module):
    """Standard fixed sinusoidal positional encoding - gives the model a
    sense of day-order within the 14-day window, since attention itself
    has none (per the notes: 'attention has no sense of order')."""
    def __init__(self, d_model, max_len=SEQ_LEN):
        super().__init__()
        pe = torch.zeros(max_len, d_model)
        position = torch.arange(0, max_len, dtype=torch.float32).unsqueeze(1)
        div_term = torch.exp(torch.arange(0, d_model, 2).float() * (-np.log(10000.0) / d_model))
        pe[:, 0::2] = torch.sin(position * div_term)
        pe[:, 1::2] = torch.cos(position * div_term)
        self.register_buffer("pe", pe.unsqueeze(0))  # (1, max_len, d_model)

    def forward(self, x):
        return x + self.pe[:, :x.size(1), :]


class BatteryTransformer(nn.Module):
    """
    Pipeline (matches the notes' architecture exactly):
      Input (batch, 14, 48)
        -> Embedding Linear(48 -> d_model)
        -> Positional Encoding (added)
        -> TransformerEncoder (num_layers x [self-attn -> add&norm -> FFN -> add&norm])
        -> Mean-pool over the 14-day sequence dimension -> (batch, d_model)
        -> concat with the predicted-day's 4 date features -> (batch, d_model+4)
        -> Dense head -> (batch, 24)
    """
    def __init__(self, n_features=N_DAY_FEATS, seq_len=SEQ_LEN, d_model=64,
                 n_heads=4, num_layers=2, dim_feedforward=128, dropout=0.1,
                 output_dim=OUTPUT_DIM):
        super().__init__()
        self.embedding = nn.Linear(n_features, d_model)
        self.pos_encoding = PositionalEncoding(d_model, max_len=seq_len)
        encoder_layer = nn.TransformerEncoderLayer(
            d_model=d_model, nhead=n_heads, dim_feedforward=dim_feedforward,
            dropout=dropout, batch_first=True, activation="relu",
        )
        self.encoder = nn.TransformerEncoder(encoder_layer, num_layers=num_layers)
        self.output_head = nn.Sequential(
            nn.Linear(d_model + N_TARGET_DATE_FEATS, dim_feedforward),
            nn.ReLU(),
            nn.Dropout(dropout),
            nn.Linear(dim_feedforward, output_dim),
        )

    def forward(self, x_seq, x_target_date):
        # x_seq: (batch, 14, 48)   x_target_date: (batch, 4)
        h = self.embedding(x_seq)              # (batch, 14, d_model)
        h = self.pos_encoding(h)
        h = self.encoder(h)                    # (batch, 14, d_model)
        pooled = h.mean(dim=1)                 # (batch, d_model) - mean pooling over the 14 days
        combined = torch.cat([pooled, x_target_date], dim=1)
        return self.output_head(combined)      # (batch, 24)


# ─────────────────────────────────────────────────────────────
# WINDOWED DATASET BUILDER
# ─────────────────────────────────────────────────────────────

def date_feats(date_str):
    ts = pd.Timestamp(date_str)
    return [float(ts.dayofweek), float(ts.day), float(ts.month), float(ts.year)]


def build_windows(wide, seq_len=SEQ_LEN):
    """
    Slides a `seq_len`-day window across the device's full history.
    Returns lists of (date, X_seq[seq_len,48], x_target_date[4], y[24]).
    A window is skipped entirely if any day inside it (context OR target)
    has any NaN value - same NaN-guard philosophy as the XGBoost pipeline
    (no fabricated/imputed values feeding the model).
    """
    n_days = len(wide)
    windows = []
    for target_idx in range(seq_len, n_days):
        ctx_idxs = list(range(target_idx - seq_len, target_idx))

        seq_rows = []
        valid = True
        for i in ctx_idxs:
            day_vals = wide.iloc[i][DSOCDT_COLS + SOC_COLS].values.astype(float)
            if np.isnan(day_vals).any():
                valid = False
                break
            seq_rows.append(day_vals)
        if not valid:
            continue

        y = wide.iloc[target_idx][DSOCDT_COLS].values.astype(float)
        if np.isnan(y).any():
            continue

        X_seq = np.stack(seq_rows, axis=0)                       # (14, 48)
        x_target_date = np.array(date_feats(wide.index[target_idx]), dtype=float)  # (4,)

        windows.append({
            "date": wide.index[target_idx],
            "X_seq": X_seq,
            "x_target_date": x_target_date,
            "y": y,
        })
    return windows
