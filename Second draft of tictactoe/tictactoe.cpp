#include <iostream>
#include <vector>
#include "sqlite3.h"
#include <string>
#include <ctime>
#include <cstdlib>
#include <sstream>

using namespace std;

// Global Variables
vector<vector<char>> board(3, vector<char>(3, ' '));
string currentUser;
string secondUser;
sqlite3* db;
bool againstAI = false;
int aiDifficulty = 1; // 1 = Easy, 2 = Medium, 3 = Hard
char userSymbol = 'X'; // Default symbol for user

// Hash function (simple)
string simpleHash(const string& password) {
    int hash = 0;
    for (char c : password) {
        hash += c;
    }
    return to_string(hash);
}

// Escape string for SQL queries to prevent SQL injection
string escapeString(const string& str) {
    string result;
    for (char c : str) {
        if (c == '\'') {
            result += "''"; // Escape single quotes by doubling them
        }
        else {
            result += c;
        }
    }
    return result;
}

// SQL execution
bool executeSQL(const string& sql) {
    char* errMsg = 0;
    int rc = sqlite3_exec(db, sql.c_str(), 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        cerr << "SQL error: " << errMsg << endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

// Setup database
void setupDatabase() {
    string createUsers = "CREATE TABLE IF NOT EXISTS Users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT UNIQUE,"
        "password TEXT);";

    string createHistory = "CREATE TABLE IF NOT EXISTS History ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT,"
        "result TEXT,"
        "game_details TEXT,"
        "date TIMESTAMP DEFAULT CURRENT_TIMESTAMP);";

    executeSQL(createUsers);
    executeSQL(createHistory);

    // Add game_details column if it doesn't exist (for backward compatibility)
    executeSQL("ALTER TABLE History ADD COLUMN game_details TEXT DEFAULT '';");
}

// Register
bool registerUser(string& username) {
    string password;
    cout << "Enter new username: ";
    cin >> username;
    cout << "Enter new password: ";
    cin >> password;

    string escapedUsername = escapeString(username);
    string hashedPassword = simpleHash(password);

    stringstream ss;
    ss << "INSERT INTO Users (username, password) VALUES ('"
        << escapedUsername << "', '" << hashedPassword << "');";

    if (executeSQL(ss.str())) {
        cout << "User registered successfully!\n";
        return true;
    }
    else {
        cout << "Registration failed. Username might already exist.\n";
        return false;
    }
}

// Login
bool loginUser(string& username) {
    string password;
    cout << "Enter username: ";
    cin >> username;
    cout << "Enter password: ";
    cin >> password;

    string escapedUsername = escapeString(username);
    string hashedPassword = simpleHash(password);

    stringstream ss;
    ss << "SELECT * FROM Users WHERE username = '"
        << escapedUsername << "' AND password = '" << hashedPassword << "';";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, ss.str().c_str(), -1, &stmt, 0);

    if (rc != SQLITE_OK) {
        cerr << "Failed to fetch data: " << sqlite3_errmsg(db) << endl;
        return false;
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        cout << "Login successful! Welcome " << username << "!\n";
        sqlite3_finalize(stmt);
        return true;
    }
    else {
        cout << "Invalid username or password.\n";
        sqlite3_finalize(stmt);
        return false;
    }
}

// Board functions
void displayBoard() {
    cout << "\n";
    cout << "  0   1   2\n";
    for (int i = 0; i < 3; i++) {
        cout << i << " ";
        for (int j = 0; j < 3; j++) {
            cout << board[i][j];
            if (j < 2) cout << " | ";
        }
        cout << "\n";
        if (i < 2) cout << "  ---------\n";
    }
    cout << "\n";
}

// Function to convert board to string for storage
string boardToString() {
    string boardStr = "";
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            boardStr += board[i][j];
        }
    }
    return boardStr;
}

// Function to display board from string
void displayBoardFromString(const string& boardStr) {
    if (boardStr.length() != 9) {
        cout << "Invalid board data.\n";
        return;
    }

    cout << "\n";
    cout << "  0   1   2\n";
    for (int i = 0; i < 3; i++) {
        cout << i << " ";
        for (int j = 0; j < 3; j++) {
            char cell = boardStr[i * 3 + j];
            cout << cell;
            if (j < 2) cout << " | ";
        }
        cout << "\n";
        if (i < 2) cout << "  ---------\n";
    }
    cout << "\n";
}

char checkWinner() {
    // Check rows
    for (int i = 0; i < 3; i++) {
        if (board[i][0] != ' ' &&
            board[i][0] == board[i][1] &&
            board[i][1] == board[i][2])
            return board[i][0];
    }

    // Check columns
    for (int i = 0; i < 3; i++) {
        if (board[0][i] != ' ' &&
            board[0][i] == board[1][i] &&
            board[1][i] == board[2][i])
            return board[0][i];
    }

    // Check diagonals
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

// AI Easy
pair<int, int> easyAIMove() {
    vector<pair<int, int>> moves;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (board[i][j] == ' ')
                moves.push_back({ i, j });

    if (moves.empty()) return { -1, -1 }; // Safety check
    return moves[rand() % moves.size()];
}

// AI Medium
pair<int, int> mediumAIMove(char aiChar, char playerChar) {
    // Try to win
    for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) {
        if (board[i][j] == ' ') {
            board[i][j] = aiChar;
            if (checkWinner() == aiChar) {
                board[i][j] = ' ';
                return { i, j };
            }
            board[i][j] = ' ';
        }
    }

    // Try to block
    for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) {
        if (board[i][j] == ' ') {
            board[i][j] = playerChar;
            if (checkWinner() == playerChar) {
                board[i][j] = ' ';
                return { i, j };
            }
            board[i][j] = ' ';
        }
    }

    // If no winning or blocking move, make a random move
    return easyAIMove();
}

// AI Hard - Minimax
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
                best = max(best, score);
            else
                best = min(best, score);
        }
    }
    return best;
}

pair<int, int> hardAIMove(char aiChar, char playerChar) {
    int bestScore = -1000;
    pair<int, int> bestMove = { -1, -1 }; // Default invalid move

    for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) {
        if (board[i][j] == ' ') {
            board[i][j] = aiChar;
            int score = minimax(false, aiChar, playerChar);
            board[i][j] = ' ';
            if (score > bestScore) {
                bestScore = score;
                bestMove = { i, j };
            }
        }
    }

    return bestMove;
}

// Save Game History
void saveGameHistory(const string& result, const string& user) {
    string escapedUser = escapeString(user);
    string escapedResult = escapeString(result);
    string gameDetails = boardToString();

    stringstream ss;
    ss << "INSERT INTO History (username, result, game_details) VALUES ('"
        << escapedUser << "', '" << escapedResult << "', '" << gameDetails << "');";

    executeSQL(ss.str());
}

// View Game History
void viewHistory() {
    cout << "\nGame History for " << currentUser << ":\n";

    string escapedUser = escapeString(currentUser);
    string sql = "SELECT username, result, date, game_details FROM History WHERE username = '" + escapedUser + "' ORDER BY date DESC LIMIT 20;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);

    if (rc != SQLITE_OK) {
        cerr << "Failed to fetch history: " << sqlite3_errmsg(db) << endl;
        return;
    }

    cout << "----------------------------------------\n";
    cout << "| Username       | Result          | Date                  |\n";
    cout << "----------------------------------------\n";

    vector<string> gameDetails;
    int gameNumber = 1;

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        string username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        string result = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        string date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        const char* details = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        string gameDetailsStr = details ? details : "";

        cout << "| " << gameNumber << ". " << username;
        for (size_t i = username.length() + 3; i < 15; i++) cout << " ";
        cout << "| " << result;
        for (size_t i = result.length(); i < 15; i++) cout << " ";
        cout << "| " << date << " |\n";

        gameDetails.push_back(gameDetailsStr);
        gameNumber++;
    }
    cout << "----------------------------------------\n";

    sqlite3_finalize(stmt);

    // Ask if user wants to see game details
    cout << "\nWould you like to see the final board state of any game? (y/n): ";
    char choice;
    cin >> choice;

    if (choice == 'y' || choice == 'Y') {
        cout << "Enter game number (1-" << gameDetails.size() << "): ";
        int gameNum;
        cin >> gameNum;

        if (gameNum >= 1 && gameNum <= (int)gameDetails.size()) {
            string details = gameDetails[gameNum - 1];
            if (!details.empty()) {
                cout << "\nFinal board state for game " << gameNum << ":\n";
                displayBoardFromString(details);
            }
            else {
                cout << "No game details available for this game.\n";
            }
        }
        else {
            cout << "Invalid game number.\n";
        }
    }
}

// Play Game
void playGame() {
    board = vector<vector<char>>(3, vector<char>(3, ' ')); // Reset board
    char player = userSymbol;
    char aiChar = (userSymbol == 'X') ? 'O' : 'X';
    int row, col;

    while (true) {
        displayBoard();

        string currentPlayerName = (player == userSymbol) ? currentUser : (againstAI ? "AI" : secondUser);

        if (againstAI && player == aiChar) {
            pair<int, int> move;
            if (aiDifficulty == 1)
                move = easyAIMove();
            else if (aiDifficulty == 2)
                move = mediumAIMove(aiChar, userSymbol);
            else
                move = hardAIMove(aiChar, userSymbol);

            row = move.first;
            col = move.second;
            cout << "AI chooses: " << row << ", " << col << endl;
        }
        else {
            cout << "Player " << player << " (" << currentPlayerName << "), enter your move (row and column): ";
            cin >> row >> col;

            // Input validation
            while (cin.fail() || row < 0 || row > 2 || col < 0 || col > 2 || board[row][col] != ' ') {
                if (cin.fail()) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                }
                cout << "Invalid move. Please enter row (0-2) and column (0-2) for an empty cell: ";
                cin >> row >> col;
            }
        }

        board[row][col] = player;
        char winner = checkWinner();

        if (winner != ' ') {
            displayBoard();
            if (againstAI) {
                if (winner == userSymbol) {
                    cout << "Congratulations " << currentUser << "! You win!\n";
                    saveGameHistory("Win vs AI", currentUser);
                }
                else {
                    cout << "AI wins! Better luck next time!\n";
                    saveGameHistory("Lost vs AI", currentUser);
                }
            }
            else {
                string winnerName = (winner == userSymbol) ? currentUser : secondUser;
                string loserName = (winner == userSymbol) ? secondUser : currentUser;
                cout << "Congratulations " << winnerName << "! You win!\n";
                saveGameHistory("Won against " + loserName, winnerName);
                saveGameHistory("Lost to " + winnerName, loserName);
            }
            break;
        }

        if (isDraw()) {
            displayBoard();
            cout << "It's a draw!\n";
            saveGameHistory("Draw", currentUser);
            if (!againstAI) saveGameHistory("Draw", secondUser);
            break;
        }

        player = (player == 'X') ? 'O' : 'X';
    }
}

// Handle second player registration/login
bool handleSecondPlayer() {
    int choice;
    bool secondLoggedIn = false;

    cout << "\nSecond Player Login/Register:\n";
    while (!secondLoggedIn) {
        cout << "1. Register\n2. Login\nChoose option: ";
        cin >> choice;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter 1 or 2.\n";
            continue;
        }

        if (choice == 1) {
            secondLoggedIn = registerUser(secondUser);
        }
        else if (choice == 2) {
            secondLoggedIn = loginUser(secondUser);
        }
        else {
            cout << "Invalid choice. Try again.\n";
        }

        // Make sure second player isn't the same as first player
        if (secondLoggedIn && secondUser == currentUser) {
            cout << "You cannot play against yourself! Please use a different account.\n";
            secondLoggedIn = false;
        }
    }

    return secondLoggedIn;
}

// Main Menu
int main() {
    srand(time(0)); // for random AI moves

    cout << "Welcome to Advanced Tic Tac Toe with SQLite + AI!\n";

    // Open Database
    int rc = sqlite3_open("game.db", &db);
    if (rc) {
        cerr << "Can't open database: " << sqlite3_errmsg(db) << endl;
        return 1;
    }
    else {
        cout << "Opened database successfully.\n";
    }

    setupDatabase();

    int choice;
    bool loggedIn = false;
    while (!loggedIn) {
        cout << "1. Register\n2. Login\nChoose option: ";
        cin >> choice;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter 1 or 2.\n";
            continue;
        }

        if (choice == 1) {
            loggedIn = registerUser(currentUser);
        }
        else if (choice == 2) {
            loggedIn = loginUser(currentUser);
        }
        else {
            cout << "Invalid choice. Try again.\n";
        }
    }

    while (true) {
        cout << "\nMain Menu:\n";
        cout << "1. Play against another Player\n";
        cout << "2. Play against AI\n";
        cout << "3. View History\n";
        cout << "4. Exit\n";
        cout << "Choice: ";
        cin >> choice;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a number between 1 and 4.\n";
            continue;
        }

        if (choice == 1) {
            againstAI = false;
            cout << "\nChoose your symbol:\n1. X\n2. O\nChoice: ";
            int symbolChoice;
            cin >> symbolChoice;
            userSymbol = (symbolChoice == 2) ? 'O' : 'X';
            cout << "You chose: " << userSymbol << endl;

            if (handleSecondPlayer()) {
                playGame();
            }
        }
        else if (choice == 2) {
            againstAI = true;
            cout << "\nChoose your symbol:\n1. X\n2. O\nChoice: ";
            int symbolChoice;
            cin >> symbolChoice;
            userSymbol = (symbolChoice == 2) ? 'O' : 'X';
            cout << "You chose: " << userSymbol << endl;

            cout << "\nChoose AI Difficulty:\n";
            cout << "1. Easy\n2. Medium\n3. Hard\nChoice: ";
            cin >> aiDifficulty;

            if (cin.fail() || aiDifficulty < 1 || aiDifficulty > 3) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid difficulty. Setting to Easy (1).\n";
                aiDifficulty = 1;
            }

            playGame();
        }
        else if (choice == 3) {
            viewHistory();
        }
        else if (choice == 4) {
            cout << "Goodbye!\n";
            break;
        }
        else {
            cout << "Invalid choice. Try again.\n";
        }
    }

    sqlite3_close(db);
    return 0;
}
