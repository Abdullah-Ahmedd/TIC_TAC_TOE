#include <iostream>
#include <vector>
#include <sqlite3.h>
#include <string>
#include <cstdlib> // for rand()
#include <ctime>   // for seeding rand()

std::string currentUsername = ""; // To track logged-in user
bool isNewUser = false;           // To know if user registered or logged in
int aiDifficulty = 3;             // 1: Easy, 2: Medium, 3: Hard

// -------- SQLite Functions --------
sqlite3* db;
int openDatabase() {
    int rc = sqlite3_open("game.db", &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    }
    return 0;
}

int closeDatabase() {
    sqlite3_close(db);
    return 0;
}

int createTables() {
    const char* createUserTableSQL = 
        "CREATE TABLE IF NOT EXISTS users ("
        "username TEXT PRIMARY KEY, "
        "passwordHash TEXT NOT NULL);";
    
    const char* createHistoryTableSQL = 
        "CREATE TABLE IF NOT EXISTS game_history ("
        "username TEXT, "
        "result TEXT, "
        "FOREIGN KEY(username) REFERENCES users(username));";
    
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, createUserTableSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return 1;
    }

    rc = sqlite3_exec(db, createHistoryTableSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return 1;
    }

    return 0;
}

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
            hash += std::to_string((int)c + 5);
        }
        return hash;
    }
};

// Check if username already exists in the database
bool userExists(const std::string& username) {
    std::string sql = "SELECT COUNT(*) FROM users WHERE username = '" + username + "';";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to fetch user: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        int count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        return count > 0;
    }
    sqlite3_finalize(stmt);
    return false;
}

// Register new user in SQLite
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

    std::string sql = "INSERT INTO users (username, passwordHash) VALUES ('" + uname + "', '" + newUser.getPasswordHash() + "');";
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    } else {
        currentUsername = uname;
        isNewUser = true;
        std::cout << "Registration successful!\n";
    }
}

// Login existing user
bool loginUser() {
    std::string uname, pword;
    std::cout << "Enter username: ";
    std::cin >> uname;
    std::cout << "Enter password: ";
    std::cin >> pword;

    std::string sql = "SELECT passwordHash FROM users WHERE username = '" + uname + "';";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to fetch user: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        std::string storedHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (storedHash == User::hashPassword(pword)) {
            std::cout << "Login successful!\n";
            currentUsername = uname;
            sqlite3_finalize(stmt);
            return true;
        }
    }
    std::cout << "Invalid username or password!\n";
    sqlite3_finalize(stmt);
    return false;
}

// Save game result to SQLite
void saveGameResult(const std::string& username, const std::string& result) {
    std::string sql = "INSERT INTO game_history (username, result) VALUES ('" + username + "', '" + result + "');";
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}

// Show game history from SQLite
void showGameHistory(const std::string& username) {
    std::string sql = "SELECT result FROM game_history WHERE username = '" + username + "';";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to fetch game history: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    bool found = false;
    std::cout << "\nGame History for " << username << ":\n";
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string result = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::cout << "- " << result << "\n";
        found = true;
    }

    if (!found) {
        std::cout << "No games played yet.\n";
    }
    sqlite3_finalize(stmt);
}

// Show score summary from SQLite
void showScoreSummary(const std::string& username) {
    std::string sql = "SELECT result FROM game_history WHERE username = '" + username + "';";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to fetch game history: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    int wins = 0, losses = 0, draws = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string result = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (result == "WIN")
            wins++;
        else if (result == "LOSS")
            losses++;
        else if (result == "DRAW")
            draws++;
    }

    std::cout << "\nScore Summary for " << username << ":\n";
    std::cout << "Wins: " << wins << "\n";
    std::cout << "Losses: " << losses << "\n";
    std::cout << "Draws: " << draws << "\n";
    sqlite3_finalize(stmt);
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

void playTicTacToe() {
    srand(time(0));
    Game game;
    int row, col;
    bool playerTurn = true;
    
    std::cout << "Welcome to Tic-Tac-Toe!" << std::endl;
    game.printBoard();

    while (true) {
        if (playerTurn) {
            std::cout << "Your move (row and column): ";
            std::cin >> row >> col;
            if (game.makeMove(row, col)) {
                game.printBoard();
                if (game.checkWin('X')) {
                    std::cout << "You win!" << std::endl;
                    saveGameResult(currentUsername, "WIN");
                    break;
                }
                game.switchPlayer();
            } else {
                std::cout << "Invalid move. Try again!" << std::endl;
            }
        } else {
            std::cout << "AI's move...\n";
            std::pair<int, int> bestMove = findBestMove(game);
            game.setCell(bestMove.first, bestMove.second, 'O');
            game.printBoard();
            if (game.checkWin('O')) {
                std::cout << "AI wins!" << std::endl;
                saveGameResult(currentUsername, "LOSS");
                break;
            }
            game.switchPlayer();
        }
        
        if (game.checkDraw()) {
            std::cout << "It's a draw!" << std::endl;
            saveGameResult(currentUsername, "DRAW");
            break;
        }
    }
}

int main() {
    if (openDatabase()) {
        return 1;
    }
    if (createTables()) {
        closeDatabase();
        return 1;
    }

    int option;

    std::cout << "Welcome to Tic-Tac-Toe\n";
    std::cout << "1. Register\n2. Login\nEnter your choice: ";
    std::cin >> option;

    if (option == 1) {
        registerUser();
    } else if (option == 2) {
        if (!loginUser()) {
            closeDatabase();
            return 0;
        }
    }

    int gameOption;
    while (true) {
        std::cout << "\n1. Play Tic-Tac-Toe\n2. View Game History\n3. View Score Summary\n4. Logout\nEnter your choice: ";
        std::cin >> gameOption;

        if (gameOption == 1) {
            playTicTacToe();
        } else if (gameOption == 2) {
            showGameHistory(currentUsername);
        } else if (gameOption == 3) {
            showScoreSummary(currentUsername);
        } else if (gameOption == 4) {
            std::cout << "Logging out...\n";
            currentUsername = "";
            break;
        }
    }

    closeDatabase();
    return 0;
}
