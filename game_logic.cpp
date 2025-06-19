#include "game_logic.h"
#include <cstdlib>
#include <limits>
#include <string>
#include <vector>
#include <algorithm>

std::vector<std::vector<char>> board(3, std::vector<char>(3, ' '));

std::string simpleHash(const std::string& password) {
    int hash = 0;
    for (char c : password) {
        hash += c;
    }
    return std::to_string(hash);
}

std::string escapeString(const std::string& str) {
    std::string result;
    for (char c : str) {
        if (c == '\'') {
            result += "''";
        } else {
            result += c;
        }
    }
    return result;
}

char checkWinner() {
    for (int i = 0; i < 3; i++) {
        if (board[i][0] != ' ' &&
            board[i][0] == board[i][1] &&
            board[i][1] == board[i][2])
            return board[i][0];
    }
    for (int i = 0; i < 3; i++) {
        if (board[0][i] != ' ' &&
            board[0][i] == board[1][i] &&
            board[1][i] == board[2][i])
            return board[0][i];
    }
    if (board[0][0] != ' ' &&
        board[0][0] == board[1][1] &&
        board[1][1] == board[2][2])
        return board[0][0];

    if (board[0][2] != ' ' &&
        board[0][2] == board[1][1] &&
        board[1][1] == board[2][0])
        return board[0][2];

    return ' ';
}

bool isDraw() {
    for (auto& row : board) {
        for (char cell : row) {
            if (cell == ' ')
                return false;
        }
    }
    return true;
}

std::pair<int, int> easyAIMove() {
    std::vector<std::pair<int, int>> moves;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (board[i][j] == ' ')
                moves.push_back({i, j});

    if (moves.empty()) return {-1, -1};
    return moves[rand() % moves.size()];
}

std::pair<int, int> mediumAIMove(char aiChar, char playerChar) {
    for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) {
        if (board[i][j] == ' ') {
            board[i][j] = aiChar;
            if (checkWinner() == aiChar) {
                board[i][j] = ' ';
                return {i, j};
            }
            board[i][j] = ' ';
        }
    }

    for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) {
        if (board[i][j] == ' ') {
            board[i][j] = playerChar;
            if (checkWinner() == playerChar) {
                board[i][j] = ' ';
                return {i, j};
            }
            board[i][j] = ' ';
        }
    }

    return easyAIMove();
}

int minimax(bool isAI, char aiChar, char playerChar) {
    char winner = checkWinner();
    if (winner == aiChar) return 10;
    if (winner == playerChar) return -10;
    if (isDraw()) return 0;

    int best = isAI ? -1000 : 1000;
    for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) {
        if (board[i][j] == ' ') {
            board[i][j] = isAI ? aiChar : playerChar;
            int score = minimax(!isAI, aiChar, playerChar);
            board[i][j] = ' ';
            if (isAI)
                best = std::max(best, score);
            else
                best = std::min(best, score);
        }
    }
    return best;
}

std::pair<int, int> hardAIMove(char aiChar, char playerChar) {
    int bestScore = -1000;
    std::pair<int, int> bestMove = {-1, -1};

    for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) {
        if (board[i][j] == ' ') {
            board[i][j] = aiChar;
            int score = minimax(false, aiChar, playerChar);
            board[i][j] = ' ';
            if (score > bestScore) {
                bestScore = score;
                bestMove = {i, j};
            }
        }
    }

    return bestMove;
}
