#include <iostream>
#include <vector>
using namespace std;

class TicTacToe {
private:
    vector<vector<char>> board;
    int currentPlayer;

public:
    TicTacToe() {
        board = vector<vector<char>>(3, vector<char>(3, ' '));
        currentPlayer = 1;
    }

    void printBoard() {
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                cout << board[i][j];
                if (j < 2) cout << " | ";
            }
            cout << endl;
            if (i < 2) cout << "---------" << endl;
        }
    }

    bool isBoardFull() {
        for (auto &row : board)
            for (char cell : row)
                if (cell == ' ') return false;
        return true;
    }

    bool makeMove(int row, int column) {
        if (row < 0 || row >= 3 || column < 0 || column >= 3 || board[row][column] != ' ')
            return false;
        board[row][column] = (currentPlayer == 1) ? 'X' : 'O';
        currentPlayer = 3 - currentPlayer;
        return true;
    }

    bool checkWinner() {
        for (int i = 0; i < 3; ++i) {
            if (board[i][0] != ' ' && board[i][0] == board[i][1] && board[i][1] == board[i][2])
                return true;
            if (board[0][i] != ' ' && board[0][i] == board[1][i] && board[1][i] == board[2][i])
                return true;
        }
        if (board[0][0] != ' ' && board[0][0] == board[1][1] && board[1][1] == board[2][2])
            return true;
        if (board[0][2] != ' ' && board[0][2] == board[1][1] && board[1][1] == board[2][0])
            return true;
        return false;
    }

    int getCurrentPlayer() { return currentPlayer; }
};

int main() {
    TicTacToe game;

    // Predefined sequence of moves (row, col)
    vector<pair<int,int>> moves = {
        {0,0}, {1,1}, {0,1}, {1,0}, {0,2}  // Player 1 wins
    };

    for (auto move : moves) {
        game.printBoard();
        cout << "Player " << game.getCurrentPlayer()
             << " plays (" << move.first << "," << move.second << ")\n";
        game.makeMove(move.first, move.second);
        if (game.checkWinner()) break;
    }

    game.printBoard();
    if (game.checkWinner()) {
        cout << "Player " << (3 - game.getCurrentPlayer()) << " wins!\n";
    } else {
        cout << "It's a draw!\n";
    }
    return 0;
}
