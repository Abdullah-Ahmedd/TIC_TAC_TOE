#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <limits>

std::string currentUsername = ""; // To track logged-in user

// -------- User Class --------
class User {
private:
    std::string username;
    std::string passwordHash;
public:
    User() {}
    User(std::string uname, std::string pword) {
        username = uname;
        passwordHash = hashPassword(pword);
    }

    std::string getUsername() {
        return username;
    }

    std::string getPasswordHash() {
        return passwordHash;
    }

    static std::string hashPassword(const std::string& password) {
        std::string hash = "";
        for (char c : password) {
            hash += std::to_string((int)c + 5); // Simple shifting hash
        }
        return hash;
    }
};

// Check if username already exists
bool userExists(const std::string& username) {
    std::ifstream file("users.txt");
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string storedUsername;
        iss >> storedUsername;
        if (storedUsername == username) {
            return true;
        }
    }
    return false;
}

// Register new user
void registerUser() {
    std::string uname, pword;
    std::cout << "Enter new username: ";
    std::cin >> uname;
    if (userExists(uname)) {
        std::cout << "Username already exists!\n";
        return;
    }
    std::cout << "Enter new password: ";
    std::cin >> pword;

    User newUser(uname, pword);

    std::ofstream file("users.txt", std::ios::app);
    file << newUser.getUsername() << " " << newUser.getPasswordHash() << "\n";
    file.close();
    currentUsername = uname;
    std::cout << "Registration successful!\n";
}

// Login existing user
bool loginUser() {
    std::string uname, pword;
    std::cout << "Enter username: ";
    std::cin >> uname;
    std::cout << "Enter password: ";
    std::cin >> pword;

    std::ifstream file("users.txt");
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string storedUsername, storedHash;
        iss >> storedUsername >> storedHash;
        if (storedUsername == uname && storedHash == User::hashPassword(pword)) {
            std::cout << "Login successful!\n";
            currentUsername = uname;
            return true;
        }
    }
    std::cout << "Invalid username or password!\n";
    return false;
}

// Save game result
void saveGameResult(const std::string& username, const std::string& result) {
    std::ofstream file("history.txt", std::ios::app);
    file << username << " " << result << "\n";
    file.close();
}

// Show game history
void showGameHistory(const std::string& username) {
    std::ifstream file("history.txt");
    if (!file) {
        std::cout << "No game history found!\n";
        return;
    }

    std::string line;
    bool found = false;
    std::cout << "\nGame History for " << username << ":\n";
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string storedUsername, result;
        iss >> storedUsername >> result;
        if (storedUsername == username) {
            std::cout << "- " << result << "\n";
            found = true;
        }
    }

    if (!found) {
        std::cout << "No games played yet.\n";
    }
    std::cout << "-------------------------\n";
}

// Show score summary
void showScoreSummary(const std::string& username) {
    std::ifstream file("history.txt");
    if (!file) {
        std::cout << "No game history found!\n";
        return;
    }

    int wins = 0, losses = 0, draws = 0;
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string storedUsername, result;
        iss >> storedUsername >> result;
        if (storedUsername == username) {
            if (result == "WIN")
                wins++;
            else if (result == "LOSS")
                losses++;
            else if (result == "DRAW")
                draws++;
        }
    }

    std::cout << "\nScore Summary for " << username << ":\n";
    std::cout << "Wins: " << wins << "\n";
    std::cout << "Losses: " << losses << "\n";
    std::cout << "Draws: " << draws << "\n";
    std::cout << "-------------------------\n";
}

// -------- Game Class --------
class Game {
private:
    std::vector<std::vector<char>> board;
    char currentPlayer;
public:
    Game() {
        board = std::vector<std::vector<char>>(3, std::vector<char>(3, ' '));
        currentPlayer = 'X';
    }

    void printBoard() {
        std::cout << "\n";
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                std::cout << board[i][j];
                if (j < 2) std::cout << " | ";
            }
            std::cout << "\n";
            if (i < 2) std::cout << "---------\n";
        }
        std::cout << "\n";
    }

    bool makeMove(int row, int col) {
        if (row >= 0 && row < 3 && col >= 0 && col < 3 && board[row][col] == ' ') {
            board[row][col] = currentPlayer;
            return true;
        }
        return false;
    }

    bool checkWin(char player) {
        for (int i = 0; i < 3; i++) {
            if (board[i][0] == player &&
                board[i][1] == player &&
                board[i][2] == player)
                return true;
            if (board[0][i] == player &&
                board[1][i] == player &&
                board[2][i] == player)
                return true;
        }
        if (board[0][0] == player &&
            board[1][1] == player &&
            board[2][2] == player)
            return true;
        if (board[0][2] == player &&
            board[1][1] == player &&
            board[2][0] == player)
            return true;

        return false;
    }

    bool checkDraw() {
        for (auto &row : board)
            for (auto &cell : row)
                if (cell == ' ')
                    return false;
        return true;
    }

    void switchPlayer() {
        currentPlayer = (currentPlayer == 'X') ? 'O' : 'X';
    }

    char getCurrentPlayer() {
        return currentPlayer;
    }

    void reset() {
        board = std::vector<std::vector<char>>(3, std::vector<char>(3, ' '));
        currentPlayer = 'X';
    }

    std::vector<std::vector<char>> getBoard() {
        return board;
    }

    void setCell(int row, int col, char player) {
        board[row][col] = player;
    }
};

// -------- AI Functions --------
int evaluate(Game& game) {
    if (game.checkWin('O'))
        return +10;
    else if (game.checkWin('X'))
        return -10;
    return 0;
}

int minimax(Game& game, bool isMax) {
    int score = evaluate(game);

    if (score == 10 || score == -10)
        return score;
    if (game.checkDraw())
        return 0;

    if (isMax) {
        int best = std::numeric_limits<int>::min();
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (game.getBoard()[i][j] == ' ') {
                    game.setCell(i, j, 'O');
                    best = std::max(best, minimax(game, !isMax));
                    game.setCell(i, j, ' ');
                }
            }
        }
        return best;
    } else {
        int best = std::numeric_limits<int>::max();
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (game.getBoard()[i][j] == ' ') {
                    game.setCell(i, j, 'X');
                    best = std::min(best, minimax(game, !isMax));
                    game.setCell(i, j, ' ');
                }
            }
        }
        return best;
    }
}

std::pair<int, int> findBestMove(Game& game) {
    int bestVal = std::numeric_limits<int>::min();
    std::pair<int, int> bestMove = {-1, -1};

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (game.getBoard()[i][j] == ' ') {
                game.setCell(i, j, 'O');
                int moveVal = minimax(game, false);
                game.setCell(i, j, ' ');

                if (moveVal > bestVal) {
                    bestMove = {i, j};
                    bestVal = moveVal;
                }
            }
        }
    }
    return bestMove;
}

// -------- Main --------
int main() {
    int userChoice;
    std::cout << "Welcome!\n1. Register\n2. Login\nChoice: ";
    std::cin >> userChoice;

    if (userChoice == 1) {
        registerUser();
    } else if (userChoice == 2) {
        if (!loginUser()) {
            std::cout << "Exiting...\n";
            return 0;
        }
    } else {
        std::cout << "Invalid choice.\n";
        return 0;
    }

    std::cout << "Do you want to view your previous game history? (1 = Yes, 2 = No): ";
    int viewHistoryChoice;
    std::cin >> viewHistoryChoice;
    if (viewHistoryChoice == 1) {
        showGameHistory(currentUsername);
        showScoreSummary(currentUsername);
    }

    Game game;
    int mode;
    std::cout << "Choose mode:\n1. Player vs Player\n2. Player vs AI\nChoice: ";
    std::cin >> mode;

    int row, col;
    bool playing = true;

    while (playing) {
        game.printBoard();

        if (mode == 2 && game.getCurrentPlayer() == 'O') {
            std::cout << "AI is thinking...\n";
            auto bestMove = findBestMove(game);
            game.makeMove(bestMove.first, bestMove.second);
        } else {
            std::cout << "Player " << game.getCurrentPlayer() << ", enter row and column (0-2): ";
            std::cin >> row >> col;
            if (!game.makeMove(row, col)) {
                std::cout << "Invalid move. Try again.\n";
                continue;
            }
        }

        if (game.checkWin(game.getCurrentPlayer())) {
            game.printBoard();
            std::cout << "Player " << game.getCurrentPlayer() << " wins!\n";
            if (mode == 2) {
                if (game.getCurrentPlayer() == 'X')
                    saveGameResult(currentUsername, "WIN");
                else
                    saveGameResult(currentUsername, "LOSS");
            } else {
                saveGameResult(currentUsername, "WIN");
            }
            playing = false;
        } else if (game.checkDraw()) {
            game.printBoard();
            std::cout << "It's a draw!\n";
            saveGameResult(currentUsername, "DRAW");
            playing = false;
        } else {
            game.switchPlayer();
        }
    }

    return 0;
}
