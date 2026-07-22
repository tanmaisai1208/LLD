"""
Battery Management - Transformer Trajectory Prediction: Train & Validate
======================================================================================
Trajectory-projection use case, Transformer version (parallel to
xgboost_final_validation.py, same window definition AND same walk-forward
protocol: day1-14 trains, predicts day15; day2-15 trains, predicts day16;
window slides forward by 1 day each time, retraining fresh every day -
exactly matching the original XGBoost pipeline's structure.

HONEST PERFORMANCE CAVEAT: retraining a Transformer from scratch ~every
single day (rather than XGBoost's millisecond-fast tree fits) is
meaningfully slower - expect real wall-clock time here. EPOCHS_PER_DAY is
kept modest (30) and there's no internal early-stopping validation split
(only ~13 samples per day, too few to split further) to keep this as
tractable as possible while still matching the requested structure. Lower
EPOCHS_PER_DAY further if this is too slow on your machine.

Requires: pip install torch
"""

import os
import glob
import warnings
import numpy as np
import pandas as pd
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import torch
import torch.nn as nn

from transformer_model import BatteryTransformer, build_sequence_for_day, date_feats, HOURS, SEQ_LEN, N_DAY_FEATS, DSOCDT_COLS

warnings.filterwarnings("ignore")

# ─────────────────────────────────────────────────────────────
# HYPERPARAMETERS
# ─────────────────────────────────────────────────────────────
MODEL_PARAMS = {
    "d_model"         : 64,
    "n_heads"         : 4,
    "num_layers"      : 2,
    "dim_feedforward" : 128,
    "dropout"         : 0.1,
}
TRAIN_PARAMS = {
    "learning_rate": 1e-3,
    "weight_decay" : 1e-4,   # L2 regularization, same purpose as XGBoost's reg_lambda
    "batch_size"   : 8,
    "epochs"       : 30,     # kept modest - see module docstring re: per-day retrain cost
}
TRAIN_DAYS = 14   # identical meaning to xgboost_final_validation.py's TRAIN_DAYS

# ─────────────────────────────────────────────────────────────
# SEQUENTIAL HYPERPARAMETER SWEEP (same coordinate-wise pattern used
# throughout the XGBoost tuning) - set TUNE_MODE=True and TUNE_PARAM to
# sweep ONE parameter at a time against overall test cosine similarity.
# ─────────────────────────────────────────────────────────────
TUNE_MODE  = False
TUNE_PARAM = "d_model"

PARAM_CANDIDATES = {
    "d_model"        : [32, 64, 96, 128],
    "n_heads"        : [2, 4, 8],
    "num_layers"     : [1, 2, 3, 4],
    "dropout"        : [0.0, 0.1, 0.2, 0.3],
    "weight_decay"   : [0, 1e-5, 1e-4, 1e-3, 1e-2],
    "learning_rate"  : [3e-4, 1e-3, 3e-3, 1e-2],
}

WIDE_CSV_FOLDER = "results/wide_csv"
OUTPUT_FOLDER   = "results/transformer/final_validation"
os.makedirs(OUTPUT_FOLDER, exist_ok=True)

THRESHOLDS     = [0.0015, 0.003, 0.0045, 0.006]
DEFAULT_PLOT_T = 0.003

DEVICE = torch.device("cuda" if torch.cuda.is_available() else "cpu")


# ─────────────────────────────────────────────────────────────
# METRICS (identical definitions to xgboost_final_validation.py, so
# results are directly comparable across the two models)
# ─────────────────────────────────────────────────────────────

def accuracy_at_threshold(actual, predicted, threshold):
    return float(np.mean(np.abs(actual - predicted) <= threshold)) * 100.0


def cosine_similarity(actual, predicted):
    norm_a = np.linalg.norm(actual)
    norm_p = np.linalg.norm(predicted)
    if norm_a == 0.0 or norm_p == 0.0:
        return 0.0
    return float(np.dot(actual, predicted) / (norm_a * norm_p))


def metrics_at_threshold(results, threshold):
    valid = [r for r in results if not np.isnan(r["mae"])]
    daily_accs = [accuracy_at_threshold(r["y_actual"], r["y_pred"], threshold) for r in valid]
    return {
        "mean_accuracy_%": round(float(np.mean(daily_accs)), 4) if daily_accs else np.nan,
        "mean_mae"       : round(float(np.mean([r["mae"] for r in valid])), 6) if valid else np.nan,
        "mean_rmse"      : round(float(np.mean([r["rmse"] for r in valid])), 6) if valid else np.nan,
    }


# ─────────────────────────────────────────────────────────────
# NORMALIZATION (Transformers need normalized inputs, unlike tree
# models - computed from TRAIN windows only, applied to train+test to
# avoid leaking test statistics into the scaling)
# ─────────────────────────────────────────────────────────────

def compute_norm_stats(X_seq_train):
    flat = X_seq_train.reshape(-1, N_DAY_FEATS)
    mean = flat.mean(axis=0)
    std = flat.std(axis=0)
    std[std == 0] = 1.0
    return mean, std


def train_model(X_seq_train, x_date_train, y_train, model_params, train_params):
    """Fresh model, trained from scratch on this single day's ~13 training
    samples, fixed epoch count (no internal val split - too little data
    to split further)."""
    model = BatteryTransformer(**model_params).to(DEVICE)
    optimizer = torch.optim.Adam(model.parameters(), lr=train_params["learning_rate"],
                                  weight_decay=train_params["weight_decay"])
    loss_fn = nn.MSELoss()

    n_samples = X_seq_train.shape[0]
    batch_size = min(train_params["batch_size"], n_samples)

    model.train()
    for epoch in range(train_params["epochs"]):
        perm = torch.randperm(n_samples)
        for start in range(0, n_samples, batch_size):
            idx = perm[start:start + batch_size]
            optimizer.zero_grad()
            pred = model(X_seq_train[idx].to(DEVICE), x_date_train[idx].to(DEVICE))
            loss = loss_fn(pred, y_train[idx].to(DEVICE))
            loss.backward()
            optimizer.step()
    return model


def run_device(wide, device_name, model_params=None, train_params=None):
    """Walk-forward, day by day - mirrors xgboost_final_validation.py's
    run_predictions loop structure exactly: day1-14 trains -> predicts
    day15, day2-15 trains -> predicts day16, sliding by 1 day each time,
    with a FRESH model retrained every day (see module docstring for the
    performance trade-off this implies)."""
    model_params = dict(MODEL_PARAMS, **(model_params or {}))
    train_params = dict(TRAIN_PARAMS, **(train_params or {}))

    n_days = len(wide)
    results = []
    total = n_days - TRAIN_DAYS
    print(f"  Predicting {total} days...")

    for predict_day_idx in range(TRAIN_DAYS, n_days):
        train_start = predict_day_idx - TRAIN_DAYS

        X_train_list, x_date_train_list, y_train_list = [], [], []
        for k in range(1, TRAIN_DAYS):
            target_idx = train_start + k
            y = wide.iloc[target_idx][DSOCDT_COLS].values.astype(float)
            if np.isnan(y).any():
                continue   # only the TARGET needs to be clean, matching XGBoost's NaN guard
            X_seq = build_sequence_for_day(wide, target_idx, seq_len=TRAIN_DAYS)
            x_date = np.array(date_feats(wide.index[target_idx]), dtype=float)
            X_train_list.append(X_seq)
            x_date_train_list.append(x_date)
            y_train_list.append(y)

        if len(X_train_list) < 2:
            results.append({
                "predict_date": wide.index[predict_day_idx],
                "y_actual": wide.iloc[predict_day_idx][DSOCDT_COLS].values.astype(float),
                "y_pred": np.full(24, np.nan), "mae": np.nan, "rmse": np.nan, "cosine_sim": np.nan,
            })
            continue

        X_train_np = np.stack(X_train_list, axis=0)
        seq_mean, seq_std = compute_norm_stats(X_train_np)
        X_train_norm = (X_train_np - seq_mean) / seq_std

        X_train_t = torch.tensor(X_train_norm, dtype=torch.float32)
        x_date_train_t = torch.tensor(np.stack(x_date_train_list), dtype=torch.float32)
        y_train_t = torch.tensor(np.stack(y_train_list), dtype=torch.float32)

        model = train_model(X_train_t, x_date_train_t, y_train_t, model_params, train_params)

        X_pred_seq = build_sequence_for_day(wide, predict_day_idx, seq_len=TRAIN_DAYS)
        X_pred_norm = (X_pred_seq - seq_mean) / seq_std
        x_pred_date = np.array(date_feats(wide.index[predict_day_idx]), dtype=float)

        model.eval()
        with torch.no_grad():
            X_pred_t = torch.tensor(X_pred_norm, dtype=torch.float32).unsqueeze(0).to(DEVICE)
            x_pred_date_t = torch.tensor(x_pred_date, dtype=torch.float32).unsqueeze(0).to(DEVICE)
            y_pred = model(X_pred_t, x_pred_date_t).cpu().numpy()[0]

        y_actual = wide.iloc[predict_day_idx][DSOCDT_COLS].values.astype(float)
        if np.isnan(y_actual).any():
            mae, rmse, cos_sim = np.nan, np.nan, np.nan
        else:
            mae = float(np.mean(np.abs(y_actual - y_pred)))
            rmse = float(np.sqrt(np.mean((y_actual - y_pred) ** 2)))
            cos_sim = cosine_similarity(y_actual, y_pred)

        results.append({
            "predict_date": wide.index[predict_day_idx], "y_actual": y_actual, "y_pred": y_pred,
            "mae": round(mae, 6) if not np.isnan(mae) else np.nan,
            "rmse": round(rmse, 6) if not np.isnan(rmse) else np.nan,
            "cosine_sim": round(cos_sim, 6) if not np.isnan(cos_sim) else np.nan,
        })

        if (predict_day_idx - TRAIN_DAYS) % 10 == 0 or predict_day_idx == n_days - 1:
            print(f"  Day {predict_day_idx - TRAIN_DAYS + 1:3d}/{total} | {wide.index[predict_day_idx]} "
                  f"| MAE: {mae if not np.isnan(mae) else 'nan'} | CosSim: {cos_sim if not np.isnan(cos_sim) else 'nan'}")

    return [r for r in results]


# ─────────────────────────────────────────────────────────────
# PLOTS (same visual style as xgboost_final_validation.py)
# ─────────────────────────────────────────────────────────────

def plot_device(results, device_name, plot_path, threshold=DEFAULT_PLOT_T):
    SAMPLE_STEP, MAX_PANELS = 7, 6
    sample_idx = list(range(0, min(len(results), SAMPLE_STEP * MAX_PANELS), SAMPLE_STEP))[:MAX_PANELS]
    n_panels = len(sample_idx)
    n_cols = 3
    n_rows = int(np.ceil(n_panels / n_cols))
    fig, axes = plt.subplots(n_rows, n_cols, figsize=(18, 5 * n_rows))
    axes = np.array(axes).reshape(-1)
    fig.suptitle(f"Actual vs Predicted dSocdt (Transformer) — {device_name}\n"
                 f"d_model={MODEL_PARAMS['d_model']}  heads={MODEL_PARAMS['n_heads']}  "
                 f"layers={MODEL_PARAMS['num_layers']}", fontsize=12, fontweight="bold")
    for panel_i, idx in enumerate(sample_idx):
        ax = axes[panel_i]; r = results[idx]
        ax.plot(HOURS, r["y_actual"], color="steelblue", linewidth=2.0, marker="o", markersize=3, label="Actual")
        ax.plot(HOURS, r["y_pred"], color="crimson", linewidth=2.0, linestyle="--", marker="s", markersize=3, label="Predicted")
        day_acc = accuracy_at_threshold(r["y_actual"], r["y_pred"], threshold)
        ax.set_title(f"{r['predict_date']}   |   Acc(T={threshold})={day_acc:.1f}%   |   "
                     f"MAE={r['mae']}   |   CosSim={r['cosine_sim']}", fontsize=9.5)
        ax.set_xlabel("Hour of day", fontsize=9); ax.set_ylabel("dSocdt", fontsize=9)
        ax.set_xticks(range(0, 24, 4)); ax.legend(fontsize=8, loc="lower left")
        ax.grid(linestyle="--", alpha=0.35)
    for empty_i in range(n_panels, len(axes)):
        axes[empty_i].axis("off")
    fig.tight_layout(); fig.savefig(plot_path, dpi=150, bbox_inches="tight"); plt.close(fig)
    print(f"  Device plot saved  →  {plot_path}")


def plot_cosine_summary(cosine_df):
    dev_data = cosine_df[cosine_df["device"] != "OVERALL_MEAN"]
    overall = cosine_df[cosine_df["device"] == "OVERALL_MEAN"]["mean_cosine_sim"].values
    fig, ax = plt.subplots(figsize=(max(10, len(dev_data) * 1.2), 6))
    colors = ["steelblue" if v >= 0.5 else ("orange" if v >= 0 else "salmon") for v in dev_data["mean_cosine_sim"]]
    bars = ax.bar(range(len(dev_data)), dev_data["mean_cosine_sim"], color=colors, edgecolor="white", linewidth=0.8, zorder=3)
    if len(overall) > 0:
        ax.axhline(overall[0], color="crimson", linestyle="-", linewidth=2.0, label=f"Overall mean: {overall[0]:.4f}", zorder=5)
    ax.axhline(0, color="black", linestyle="-", linewidth=0.8)
    for bar, val in zip(bars, dev_data["mean_cosine_sim"]):
        ax.text(bar.get_x() + bar.get_width() / 2, bar.get_height() + 0.01, f"{val:.3f}", ha="center", va="bottom", fontsize=8, fontweight="bold")
    short_names = [d[:12] + "…" if len(d) > 12 else d for d in dev_data["device"]]
    ax.set_xticks(range(len(dev_data))); ax.set_xticklabels(short_names, rotation=30, ha="right", fontsize=8)
    ax.set_ylabel("Mean Cosine Similarity", fontsize=12); ax.set_ylim(-1.1, 1.2)
    ax.set_title("Mean Cosine Similarity (Transformer) — per device", fontsize=12, fontweight="bold")
    ax.legend(fontsize=10); ax.grid(axis="y", linestyle="--", alpha=0.4)
    plot_path = os.path.join(OUTPUT_FOLDER, "cosine_similarity_plot.png")
    fig.tight_layout(); fig.savefig(plot_path, dpi=150, bbox_inches="tight"); plt.close(fig)
    print(f"  Cosine similarity plot saved  →  {plot_path}")


# ─────────────────────────────────────────────────────────────
# SINGLE-PARAMETER SWEEP
# ─────────────────────────────────────────────────────────────

def run_param_sweep(wide_files):
    candidates = PARAM_CANDIDATES[TUNE_PARAM]
    is_model_param = TUNE_PARAM in MODEL_PARAMS
    print(f"TRANSFORMER SWEEP — parameter: {TUNE_PARAM}\nCandidates: {candidates}")
    sweep_rows = []
    for candidate in candidates:
        m_params = dict(MODEL_PARAMS); t_params = dict(TRAIN_PARAMS)
        if is_model_param:
            m_params[TUNE_PARAM] = candidate
        else:
            t_params[TUNE_PARAM] = candidate
        all_cos = []
        for wf in wide_files:
            device_name = os.path.splitext(os.path.basename(wf))[0].replace("_wide", "")
            wide = pd.read_csv(wf, index_col="Date")
            results = run_device(wide, device_name, model_params=m_params, train_params=t_params)
            if results:
                valid_cos = [r["cosine_sim"] for r in results if not np.isnan(r["cosine_sim"])]
                if valid_cos:
                    all_cos.append(float(np.mean(valid_cos)))
        overall = round(float(np.mean(all_cos)), 6) if all_cos else np.nan
        print(f"  {TUNE_PARAM}={candidate}  →  overall_mean_cos={overall}")
        sweep_rows.append({TUNE_PARAM: candidate, "overall_mean_cosine_sim": overall})
    sweep_df = pd.DataFrame(sweep_rows).sort_values("overall_mean_cosine_sim", ascending=False)
    sweep_df.to_csv(os.path.join(OUTPUT_FOLDER, f"transformer_sweep_{TUNE_PARAM}.csv"), index=False)
    print(sweep_df.to_string(index=False))


# ─────────────────────────────────────────────────────────────
# MAIN
# ─────────────────────────────────────────────────────────────

if __name__ == "__main__":
    wide_files = sorted(glob.glob(os.path.join(WIDE_CSV_FOLDER, "*_wide.csv")))
    if not wide_files:
        raise FileNotFoundError(f"No *_wide.csv files in '{WIDE_CSV_FOLDER}'.")

    if TUNE_MODE:
        run_param_sweep(wide_files)
        raise SystemExit(0)

    print(f"TRANSFORMER TRAJECTORY VALIDATION — model={MODEL_PARAMS}  train={TRAIN_PARAMS}")

    all_threshold_rows, cosine_rows = [], []

    for wf in wide_files:
        device_name = os.path.splitext(os.path.basename(wf))[0].replace("_wide", "")
        pred_path = os.path.join(OUTPUT_FOLDER, f"final_predictions_{device_name}.csv")
        plot_path = os.path.join(OUTPUT_FOLDER, f"final_validation_plot_{device_name}.png")
        print(f"Device : {device_name}")
        try:
            wide = pd.read_csv(wf, index_col="Date")
            results = run_device(wide, device_name)
            if not results:
                continue

            pred_rows = []
            for r in results:
                for h in HOURS:
                    pred_rows.append({
                        "predict_date": r["predict_date"], "hour": h,
                        "actual_dSocdt": round(float(r["y_actual"][h]), 6),
                        "pred_dSocdt": round(float(r["y_pred"][h]), 6),
                        "abs_error": round(abs(float(r["y_actual"][h]) - float(r["y_pred"][h])), 6),
                        "daily_cosine_sim": r["cosine_sim"],
                    })
            pd.DataFrame(pred_rows).to_csv(pred_path, index=False)
            plot_device(results, device_name, plot_path, threshold=DEFAULT_PLOT_T)

            for T in THRESHOLDS:
                m = metrics_at_threshold(results, T)
                all_threshold_rows.append({"device": device_name, "threshold": T, **m})

            cos_vals = [r["cosine_sim"] for r in results if not np.isnan(r["cosine_sim"])]
            if not cos_vals:
                print(f"  No valid (non-NaN) days for this device.\n")
                continue
            cosine_rows.append({
                "device": device_name, "mean_cosine_sim": round(float(np.mean(cos_vals)), 6),
                "min_cosine_sim": round(float(np.min(cos_vals)), 6),
                "max_cosine_sim": round(float(np.max(cos_vals)), 6),
                "total_days": len(results),
            })
        except Exception as e:
            import traceback
            print(f"  ERROR: {e}"); traceback.print_exc()

    threshold_df = pd.DataFrame(all_threshold_rows)
    threshold_df.to_csv(os.path.join(OUTPUT_FOLDER, "final_summary_all_thresholds.csv"), index=False)

    cosine_df = pd.DataFrame(cosine_rows)
    if not cosine_df.empty:
        overall_cos = round(float(cosine_df["mean_cosine_sim"].mean()), 6)
        cosine_df = pd.concat([cosine_df, pd.DataFrame([{
            "device": "OVERALL_MEAN", "mean_cosine_sim": overall_cos,
            "min_cosine_sim": round(float(cosine_df["min_cosine_sim"].min()), 6),
            "max_cosine_sim": round(float(cosine_df["max_cosine_sim"].max()), 6),
            "total_days": cosine_df["total_days"].sum(),
        }])], ignore_index=True)
    cosine_df.to_csv(os.path.join(OUTPUT_FOLDER, "cosine_similarity_summary.csv"), index=False)
    plot_cosine_summary(cosine_df)

    print("\nOVERALL MEAN ACCURACY PER THRESHOLD")
    for T in THRESHOLDS:
        t_df = threshold_df[threshold_df["threshold"] == T]
        print(f"  T={T}  acc={t_df['mean_accuracy_%'].mean():.2f}%  "
              f"mae={t_df['mean_mae'].mean():.5f}  rmse={t_df['mean_rmse'].mean():.5f}")
    print("\nCOSINE SIMILARITY SUMMARY")
    print(cosine_df.to_string(index=False))
