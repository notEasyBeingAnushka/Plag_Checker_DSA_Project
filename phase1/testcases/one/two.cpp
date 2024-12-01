#include <bits/stdc++.h>
using namespace std;

class Solution {
public:
    void solveSudoku(vector<vector<char>>& board) {
        if (!board.empty()) {
            solve(board);
        }
    }

    bool solve(vector<vector<char>>& board) {
        int row, col;
        if (!findEmptyCell(board, row, col)) {
            return true;
        }

        for (char num = '1'; num <= '9'; num++) {
            if (isValid(board, row, col, num)) {
                board[row][col] = num;
                if (solve(board)) {
                    return true;
                }
                board[row][col] = '.';
            }
        }
        return false;
    }

    bool findEmptyCell(const vector<vector<char>>& board, int& row, int& col) {
        for (row = 0; row < 9; row++) {
            for (col = 0; col < 9; col++) {
                if (board[row][col] == '.') {
                    return true;
                }
            }
        }
        return false;
    }

    bool isValid(vector<vector<char>>& board, int row, int col, char num) {
        return !isInRow(board, row, num) && !isInCol(board, col, num) && !isInSubgrid(board, row, col, num);
    }

    bool isInRow(const vector<vector<char>>& board, int row, char num) {
        for (int col = 0; col < 9; col++) {
            if (board[row][col] == num) {
                return true;
            }
        }
        return false;
    }

    bool isInCol(const vector<vector<char>>& board, int col, char num) {
        for (int row = 0; row < 9; row++) {
            if (board[row][col] == num) {
                return true;
            }
        }
        return false;
    }

    bool isInSubgrid(const vector<vector<char>>& board, int row, int col, char num) {
        int startRow = 3 * (row / 3), startCol = 3 * (col / 3);
        for (int i = startRow; i < startRow + 3; i++) {
            for (int j = startCol; j < startCol + 3; j++) {
                if (board[i][j] == num) {
                    return true;
                }
            }
        }
        return false;
    }
};

