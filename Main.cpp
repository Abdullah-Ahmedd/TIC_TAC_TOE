#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <ctime>
#include <chrono>
#include <sqlite3.h>


#include <iomanip>
#include <limits>
#include <algorithm>
#include <cstring>

using namespace std;

// SQLite database handler
class Database {
private:
    sqlite3* db;
    string dbName = "tictactoe.db";

public:
    Database() {
        int rc = sqlite3_open(dbName.c_str(), &db);
        if (rc) {
            cerr << "Error opening database: " << sqlite3_errmsg(db) << endl;
            return;
        }
        
        // Create users table if not exists
        const char* createUsersTable = 
            "CREATE TABLE IF NOT EXISTS users ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "username TEXT UNIQUE NOT NULL,"
            "password TEXT NOT NULL,"
            "wins INTEGER DEFAULT 0,"
            "losses INTEGER DEFAULT 0,"
            "draws INTEGER DEFAULT 0);";
        
        rc = sqlite3_exec(db, createUsersTable, nullptr, nullptr, nullptr);
        if (rc != SQLITE_OK) {
            cerr << "Error creating users table: " << sqlite3_errmsg(db) << endl;
        }
        
        // Create games history table if not exists
        const char* createGamesTable = 
            "CREATE TABLE IF NOT EXISTS games ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "player1 TEXT NOT NULL,"
            "player2 TEXT NOT NULL,"
            "winner TEXT,"
            "date TEXT NOT NULL,"
            "mode TEXT NOT NULL,"
            "difficulty TEXT,"
            "moves TEXT NOT NULL);";
        
        rc = sqlite3_exec(db, createGamesTable, nullptr, nullptr, nullptr);
        if (rc != SQLITE_OK) {
            cerr << "Error creating games table: " << sqlite3_errmsg(db) << endl;
        }
    }
    
    ~Database() {
        sqlite3_close(db);
    }
    
    bool registerUser(const string& username, const string& password) {
        // Check if username already exists
        if (userExists(username)) {
            return false;
        }
        
        string sql = "INSERT INTO users (username, password) VALUES (?, ?);";
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            cerr << "Error preparing statement: " << sqlite3_errmsg(db) << endl;
            return false;
        }
        
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);
        
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        
        return rc == SQLITE_DONE;
    }
    
    bool authenticateUser(const string& username, const string& password) {
        string sql = "SELECT password FROM users WHERE username = ?;";
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            cerr << "Error preparing statement: " << sqlite3_errmsg(db) << endl;
            return false;
        }
        
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        
        rc = sqlite3_step(stmt);
        bool authenticated = false;
        
        if (rc == SQLITE_ROW) {
            string storedPassword = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            authenticated = (password == storedPassword);
        }
        
        sqlite3_finalize(stmt);
        return authenticated;
    }
    
    bool userExists(const string& username) {
        string sql = "SELECT 1 FROM users WHERE username = ?;";
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            cerr << "Error preparing statement: " << sqlite3_errmsg(db) << endl;
            return false;
        }
        
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        
        rc = sqlite3_step(stmt);
        bool exists = (rc == SQLITE_ROW);
        
        sqlite3_finalize(stmt);
        return exists;
    }
    
    void updateStats(const string& username, int result) {
        // result: 1 = win, 0 = draw, -1 = loss
        string field;
        if (result == 1) {
            field = "wins";
        } else if (result == 0) {
            field = "draws";
        } else {
            field = "losses";
        }
        
        string sql = "UPDATE users SET " + field + " = " + field + " + 1 WHERE username = ?;";
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            cerr << "Error preparing statement: " << sqlite3_errmsg(db) << endl;
            return;
        }
        
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
    
    void recordGame(const string& player1, const string& player2, const string& winner, 
                    const string& mode, const string& difficulty, const string& moves) {
        // Get current date and time
        auto now = chrono::system_clock::now();
        time_t now_time = chrono::system_clock::to_time_t(now);
        tm* now_tm = localtime(&now_time);
        
        char dateStr[20];
        strftime(dateStr, sizeof(dateStr), "%Y-%m-%d %H:%M:%S", now_tm);
        
        string sql = "INSERT INTO games (player1, player2, winner, date, mode, difficulty, moves) "
                    "VALUES (?, ?, ?, ?, ?, ?, ?);";
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            cerr << "Error preparing statement: " << sqlite3_errmsg(db) << endl;
            return;
        }
        
        sqlite3_bind_text(stmt, 1, player1.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, player2.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, winner.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, dateStr, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 5, mode.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 6, difficulty.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 7, moves.c_str(), -1, SQLITE_STATIC);
        
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
    
    void displayUserStats(const string& username) {
        string sql = "SELECT wins, losses, draws FROM users WHERE username = ?;";
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            cerr << "Error preparing statement: " << sqlite3_errmsg(db) << endl;
            return;
        }
        
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            int wins = sqlite3_column_int(stmt, 0);
            int losses = sqlite3_column_int(stmt, 1);
            int draws = sqlite3_column_int(stmt, 2);
            
            cout << "\n===== " << username << "'s Stats =====\n";
            cout << "Wins: " << wins << "\n";
            cout << "Losses: " << losses << "\n";
            cout << "Draws: " << draws << "\n";
            cout << "Total Games: " << (wins + losses + draws) << "\n";
            if (wins + losses + draws > 0) {
                double winRate = (static_cast<double>(wins) / (wins + losses + draws)) * 100;
                cout << "Win Rate: " << fixed << setprecision(2) << winRate << "%\n";
            }
            cout << "==========================\n\n";
        }
        
        sqlite3_finalize(stmt);
    }
    
    void displayGameHistory(const string& username) {
        string sql = "SELECT player1, player2, winner, date, mode, difficulty "
                    "FROM games WHERE player1 = ? OR player2 = ? "
                    "ORDER BY date DESC LIMIT 10;";
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            cerr << "Error preparing statement: " << sqlite3_errmsg(db) << endl;
            return;
        }
        
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_STATIC);
        
        cout << "\n===== Recent Game History for " << username << " =====\n";
        cout << left << setw(20) << "Date" 
             << setw(15) << "Player 1" 
             << setw(15) << "Player 2" 
             << setw(15) << "Winner" 
             << setw(15) << "Mode"
             << setw(10) << "Difficulty" << endl;
        cout << string(90, '-') << endl;
        
        bool hasGames = false;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            hasGames = true;
            string player1 = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            string player2 = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            const char* winnerText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            string winner = winnerText ? string(winnerText) : "Draw";
            string date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            string mode = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            const char* diffText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
            string difficulty = diffText ? string(diffText) : "N/A";
            
            cout << left << setw(20) << date.substr(0, 19)
                 << setw(15) << player1
                 << setw(15) << player2
                 << setw(15) << winner
                 << setw(15) << mode
                 << setw(10) << difficulty << endl;
        }
        
        if (!hasGames) {
            cout << "No game history found for " << username << endl;
        }
        
        cout << "==========================================\n\n";
        sqlite3_finalize(stmt);
    }
};

// TicTacToe Game Engine
class TicTacToe {
private:
    char board[3][3];
    Database* db;
    string player1;
    string player2;
    string currentPlayer;
    string mode;
    string difficulty;
    string moves;
    
    void initializeBoard() {
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                board[i][j] = ' ';
            }
        }
    }
    
    void displayBoard() {
        cout << "\n";
        cout << "     1   2   3 \n";
        cout << "   ┌───┬───┬───┐\n";
        for (int i = 0; i < 3; i++) {
            cout << " " << (i + 1) << " │ " << board[i][0] << " │ " << board[i][1] << " │ " << board[i][2] << " │\n";
            if (i < 2) {
                cout << "   ├───┼───┼───┤\n";
            }
        }
        cout << "   └───┴───┴───┘\n\n";
    }
    
    bool isMoveValid(int row, int col) {
        return (row >= 0 && row < 3 && col >= 0 && col < 3 && board[row][col] == ' ');
    }
    
    bool isGameOver(char& winner) {
        // Check rows
        for (int i = 0; i < 3; i++) {
            if (board[i][0] != ' ' && board[i][0] == board[i][1] && board[i][1] == board[i][2]) {
                winner = board[i][0];
                return true;
            }
        }
        
        // Check columns
        for (int i = 0; i < 3; i++) {
            if (board[0][i] != ' ' && board[0][i] == board[1][i] && board[1][i] == board[2][i]) {
                winner = board[0][i];
                return true;
            }
        }
        
        // Check diagonals
        if (board[0][0] != ' ' && board[0][0] == board[1][1] && board[1][1] == board[2][2]) {
            winner = board[0][0];
            return true;
        }
        
        if (board[0][2] != ' ' && board[0][2] == board[1][1] && board[1][1] == board[2][0]) {
            winner = board[0][2];
            return true;
        }
        
        // Check if board is full (draw)
        bool isFull = true;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (board[i][j] == ' ') {
                    isFull = false;
                    break;
                }
            }
            if (!isFull) break;
        }
        
        if (isFull) {
            winner = 'D'; // Draw
            return true;
        }
        
        return false;
    }
    
    void computerMove(char computerMark) {
        if (difficulty == "easy") {
            // Easy mode: Random valid move
            vector<pair<int, int>> validMoves;
            
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    if (board[i][j] == ' ') {
                        validMoves.push_back(make_pair(i, j));
                    }
                }
            }
            
            if (!validMoves.empty()) {
                random_device rd;
                mt19937 gen(rd());
                uniform_int_distribution<> distrib(0, validMoves.size() - 1);
                
                int index = distrib(gen);
                int row = validMoves[index].first;
                int col = validMoves[index].second;
                
                board[row][col] = computerMark;
                cout << "Computer plays at position (" << (row + 1) << ", " << (col + 1) << ")\n";
                
                // Record move
                moves += to_string(row) + "," + to_string(col) + "," + computerMark + ";";
            }
        }
        else if (difficulty == "medium") {
            // Medium mode: Win if possible, block opponent if needed, otherwise random
            char playerMark = (computerMark == 'X') ? 'O' : 'X';
            
            // Try to win
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    if (board[i][j] == ' ') {
                        board[i][j] = computerMark;
                        char winner = ' ';
                        if (isGameOver(winner) && winner == computerMark) {
                            cout << "Computer plays at position (" << (i + 1) << ", " << (j + 1) << ")\n";
                            moves += to_string(i) + "," + to_string(j) + "," + computerMark + ";";
                            return;
                        }
                        board[i][j] = ' '; // Undo move
                    }
                }
            }
            
            // Block opponent
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    if (board[i][j] == ' ') {
                        board[i][j] = playerMark;
                        char winner = ' ';
                        if (isGameOver(winner) && winner == playerMark) {
                            board[i][j] = computerMark; // Block
                            cout << "Computer plays at position (" << (i + 1) << ", " << (j + 1) << ")\n";
                            moves += to_string(i) + "," + to_string(j) + "," + computerMark + ";";
                            return;
                        }
                        board[i][j] = ' '; // Undo move
                    }
                }
            }
            
            // If no winning or blocking move, try center
            if (board[1][1] == ' ') {
                board[1][1] = computerMark;
                cout << "Computer plays at position (2, 2)\n";
                moves += "1,1," + string(1, computerMark) + ";";
                return;
            }
            
            // Otherwise, random move (like easy mode)
            vector<pair<int, int>> validMoves;
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    if (board[i][j] == ' ') {
                        validMoves.push_back(make_pair(i, j));
                    }
                }
            }
            
            if (!validMoves.empty()) {
                random_device rd;
                mt19937 gen(rd());
                uniform_int_distribution<> distrib(0, validMoves.size() - 1);
                
                int index = distrib(gen);
                int row = validMoves[index].first;
                int col = validMoves[index].second;
                
                board[row][col] = computerMark;
                cout << "Computer plays at position (" << (row + 1) << ", " << (col + 1) << ")\n";
                moves += to_string(row) + "," + to_string(col) + "," + computerMark + ";";
            }
        }
        else if (difficulty == "hard") {
            // Hard mode: Uses minimax algorithm for optimal play
            int bestScore = -1000;
            int bestRow = -1;
            int bestCol = -1;
            
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    if (board[i][j] == ' ') {
                        board[i][j] = computerMark;
                        int score = minimax(board, 0, false, computerMark);
                        board[i][j] = ' ';
                        
                        if (score > bestScore) {
                            bestScore = score;
                            bestRow = i;
                            bestCol = j;
                        }
                    }
                }
            }
            
            board[bestRow][bestCol] = computerMark;
            cout << "Computer plays at position (" << (bestRow + 1) << ", " << (bestCol + 1) << ")\n";
            moves += to_string(bestRow) + "," + to_string(bestCol) + "," + computerMark + ";";
        }
    }
    
    int minimax(char currentBoard[3][3], int depth, bool isMaximizing, char computerMark) {
        char playerMark = (computerMark == 'X') ? 'O' : 'X';
        char winner = ' ';
        
        if (checkWinner(currentBoard, computerMark)) {
            return 10 - depth;
        }
        if (checkWinner(currentBoard, playerMark)) {
            return depth - 10;
        }
        if (isBoardFull(currentBoard)) {
            return 0;
        }
        
        if (isMaximizing) {
            int bestScore = -1000;
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    if (currentBoard[i][j] == ' ') {
                        currentBoard[i][j] = computerMark;
                        int score = minimax(currentBoard, depth + 1, false, computerMark);
                        currentBoard[i][j] = ' ';
                        bestScore = max(score, bestScore);
                    }
                }
            }
            return bestScore;
        } 
        else {
            int bestScore = 1000;
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    if (currentBoard[i][j] == ' ') {
                        currentBoard[i][j] = playerMark;
                        int score = minimax(currentBoard, depth + 1, true, computerMark);
                        currentBoard[i][j] = ' ';
                        bestScore = min(score, bestScore);
                    }
                }
            }
            return bestScore;
        }
    }
    
    bool checkWinner(char board[3][3], char mark) {
        // Check rows
        for (int i = 0; i < 3; i++) {
            if (board[i][0] == mark && board[i][1] == mark && board[i][2] == mark) {
                return true;
            }
        }
        
        // Check columns
        for (int i = 0; i < 3; i++) {
            if (board[0][i] == mark && board[1][i] == mark && board[2][i] == mark) {
                return true;
            }
        }
        
        // Check diagonals
        if (board[0][0] == mark && board[1][1] == mark && board[2][2] == mark) {
            return true;
        }
        
        if (board[0][2] == mark && board[1][1] == mark && board[2][0] == mark) {
            return true;
        }
        
        return false;
    }
    
    bool isBoardFull(char board[3][3]) {
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (board[i][j] == ' ') {
                    return false;
                }
            }
        }
        return true;
    }
    
public:
    TicTacToe(Database* database) : db(database) {
        initializeBoard();
        moves = "";
    }
    
    void setPlayer1(const string& username) {
        player1 = username;
    }
    
    void setPlayer2(const string& username) {
        player2 = username;
    }
    
    void setMode(const string& gameMode) {
        mode = gameMode;
    }
    
    void setDifficulty(const string& diffLevel) {
        difficulty = diffLevel;
    }
    
    void startGame() {
        initializeBoard();
        moves = "";
        currentPlayer = player1;
        char winner = ' ';
        bool gameEnded = false;
        
        cout << "\n==== Game Started ====\n";
        cout << "Player 1 (" << player1 << "): X\n";
        if (mode == "pvp") {
            cout << "Player 2 (" << player2 << "): O\n";
        } else {
            cout << "Computer (AI): O\n";
        }
        
        // Main game loop
        while (!gameEnded) {
            displayBoard();
            
            char playerMark = (currentPlayer == player1) ? 'X' : 'O';
            
            if (currentPlayer == player2 && mode == "pvc") {
                // Computer's turn
                cout << "\nComputer (" << difficulty << " difficulty) is thinking...\n";
                computerMove(playerMark);
            } else {
                // Human player's turn
                cout << "\n" << currentPlayer << "'s turn (" << playerMark << ").\n";
                int row, col;
                bool validMove = false;
                
                while (!validMove) {
                    cout << "Enter row (1-3): ";
                    while (!(cin >> row) || row < 1 || row > 3) {
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        cout << "Invalid input! Please enter a number between 1 and 3: ";
                    }
                    
                    cout << "Enter column (1-3): ";
                    while (!(cin >> col) || col < 1 || col > 3) {
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        cout << "Invalid input! Please enter a number between 1 and 3: ";
                    }
                    
                    row--; // Convert to 0-based indexing
                    col--; // Convert to 0-based indexing
                    
                    if (isMoveValid(row, col)) {
                        validMove = true;
                        board[row][col] = playerMark;
                        // Record move
                        moves += to_string(row) + "," + to_string(col) + "," + playerMark + ";";
                    } else {
                        cout << "That position is already taken. Try again.\n";
                    }
                }
            }
            
            // Check if game is over
            if (isGameOver(winner)) {
                displayBoard();
                string winnerName = "";
                
                if (winner == 'X') {
                    winnerName = player1;
                    cout << "Player 1 (" << player1 << ") wins!\n";
                    db->updateStats(player1, 1); // Win
                    if (mode == "pvp") {
                        db->updateStats(player2, -1); // Loss
                    }
                } 
                else if (winner == 'O') {
                    if (mode == "pvp") {
                        winnerName = player2;
                        cout << "Player 2 (" << player2 << ") wins!\n";
                        db->updateStats(player2, 1); // Win
                    } else {
                        winnerName = "Computer";
                        cout << "Computer wins!\n";
                    }
                    db->updateStats(player1, -1); // Loss
                } 
                else {
                    cout << "It's a draw!\n";
                    db->updateStats(player1, 0); // Draw
                    if (mode == "pvp") {
                        db->updateStats(player2, 0); // Draw
                    }
                }
                
                // Record game in database
                db->recordGame(player1, (mode == "pvp" ? player2 : "Computer"), 
                              winnerName, mode, difficulty, moves);
                
                gameEnded = true;
            } 
            else {
                // Switch players
                currentPlayer = (currentPlayer == player1) ? player2 : player1;
            }
        }
        
        cout << "\n==== Game Ended ====\n\n";
    }
};

// User Interface Class
class GameUI {
private:
    Database db;
    TicTacToe game;
    string currentUser;
    
    string getPassword() {
        string password;
        cout << "Password: ";
        cin >> password;
        return password;
    }
    
    void clearScreen() {
        #ifdef _WIN32
            system("cls");
        #else
            system("clear");
        #endif
    }
    
    void showMainMenu() {
        while (true) {
            clearScreen();
            cout << "\n======= TIC TAC TOE =======\n";
            cout << "Welcome, " << currentUser << "!\n\n";
            cout << "1. Play vs Player\n";
            cout << "2. Play vs Computer\n";
            cout << "3. View Game History\n";
            cout << "4. View Statistics\n";
            cout << "5. Log Out\n";
            cout << "6. Exit\n";
            cout << "==========================\n";
            cout << "Select an option: ";
            
            int choice;
            cin >> choice;
            
            switch (choice) {
                case 1:
                    playVsPlayer();
                    break;
                case 2:
                    playVsComputer();
                    break;
                case 3:
                    db.displayGameHistory(currentUser);
                    cout << "Press Enter to continue...";
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cin.get();
                    break;
                case 4:
                    db.displayUserStats(currentUser);
                    cout << "Press Enter to continue...";
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cin.get();
                    break;
                case 5:
                    cout << "Logging out...\n";
                    return;
                case 6:
                    cout << "Thank you for playing!\n";
                    exit(0);
                default:
                    cout << "Invalid option. Try again.\n";
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
        }
    }
    
    void playVsPlayer() {
        clearScreen();
        cout << "\n===== PLAYER VS PLAYER =====\n";
        cout << "Player 1: " << currentUser << " (X)\n";
        
        string player2;
        string password;
        bool validPlayer2 = false;


        

        while (!validPlayer2) {
            cout << "Enter Player 2 username: ";
            cin >> player2;
            
            if (player2 == currentUser) {
                cout << "You cannot play against yourself. Please enter a different username.\n";
                continue;
            }
            
            if (!db.userExists(player2)) {
                cout << "User does not exist. Would you like to register this user? (y/n): ";
                char choice;
                cin >> choice;
                
                if (tolower(choice) == 'y') {
                    cout << "Enter password for new user: ";
                    cin >> password;
                    
                    if (db.registerUser(player2, password)) {
                        cout << "User registered successfully.\n";
                        validPlayer2 = true;
                    } else {
                        cout << "Error registering user. Please try again.\n";
                    }
                }
            } else {
                cout << "Enter password for " << player2 << ": ";
                cin >> password;
                
                if (db.authenticateUser(player2, password)) {
                    validPlayer2 = true;
                } else {
                    cout << "Incorrect password. Try again.\n";
                }
            }
        }
        
        game.setPlayer1(currentUser);
        game.setPlayer2(player2);
        game.setMode("pvp");
        game.setDifficulty("N/A");
        game.startGame();
        
        cout << "Press Enter to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin.get();
    }
    
    void playVsComputer() {
        clearScreen();
        cout << "\n===== PLAYER VS COMPUTER =====\n";
        cout << "Select difficulty level:\n";
        cout << "1. Easy\n";
        cout << "2. Medium\n";
        cout << "3. Hard\n";
        cout << "4. Back to Main Menu\n";
        cout << "Select an option: ";
        
        int choice;
        cin >> choice;
        
        string diffLevel;
        switch (choice) {
            case 1:
                diffLevel = "easy";
                break;
            case 2:
                diffLevel = "medium";
                break;
            case 3:
                diffLevel = "hard";
                break;
            case 4:
                return;
            default:
                cout << "Invalid option. Returning to main menu.\n";
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                return;
        }
        
        game.setPlayer1(currentUser);
        game.setPlayer2("Computer");
        game.setMode("pvc");
        game.setDifficulty(diffLevel);
        game.startGame();
        
        cout << "Press Enter to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin.get();
    }
    
public:
    GameUI() : game(&db) {}
    
    void start() {
        while (true) {
            clearScreen();
            cout << "\n======= TIC TAC TOE =======\n";
            cout << "1. Login\n";
            cout << "2. Register\n";
            cout << "3. Exit\n";
            cout << "==========================\n";
            cout << "Select an option: ";
            
            int choice;
            if (!(cin >> choice)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid input. Please try again.\n";
                continue;
            }
            
            switch (choice) {
                case 1:
                    login();
                    break;
                case 2:
                    registerUser();
                    break;
                case 3:
                    cout << "Thank you for playing!\n";
                    return;
                default:
                    cout << "Invalid option. Try again.\n";
            }
        }
    }
    
    void login() {
        clearScreen();
        cout << "\n======= LOGIN =======\n";
        string username, password;
        
        cout << "Username: ";
        cin >> username;
        
        cout << "Password: ";
        cin >> password;
        
        if (db.authenticateUser(username, password)) {
            currentUser = username;
            cout << "Login successful!\n";
            showMainMenu();
        } else {
            cout << "Invalid username or password.\n";
            cout << "Press Enter to continue...";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cin.get();
        }
    }
    
    void registerUser() {
        clearScreen();
        cout << "\n======= REGISTER =======\n";
        string username, password;
        
        cout << "Username: ";
        cin >> username;
        
        if (db.userExists(username)) {
            cout << "Username already exists. Please choose another one.\n";
            cout << "Press Enter to continue...";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cin.get();
            return;
        }
        
        cout << "Password: ";
        cin >> password;
        
        if (db.registerUser(username, password)) {
            cout << "Registration successful! You can now login.\n";
        } else {
            cout << "Error during registration. Please try again.\n";
        }
        
        cout << "Press Enter to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin.get();
    }
};

int main() {
    // Seed random number generator
    srand(static_cast<unsigned int>(time(nullptr)));
    
    // Create SQLite database if not exists
    sqlite3* db;
    int rc = sqlite3_open("tictactoe.db", &db);
    if (rc) {
        cerr << "Error opening database: " << sqlite3_errmsg(db) << endl;
        return 1;
    }
    sqlite3_close(db);
    
    // Start the game UI
    GameUI ui;
    ui.start();
    
    return 0;
}