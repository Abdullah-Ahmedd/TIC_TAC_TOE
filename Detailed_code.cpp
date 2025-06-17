// Include necessary libraries for input/output, vectors, database, strings, time, and random numbers
#include <iostream>  // For console input/output (cout, cin)
#include <vector>    // For dynamic arrays (vector)
#include "sqlite3.h" // For SQLite database operations
#include <string>    // For string operations
#include <ctime>     // For time-related functions (seeding random numbers)
#include <cstdlib>   // For random number generation (rand)
#include <sstream>   // For string stream operations (building SQL queries)

// This allows us to use cout, cin, string, etc. without typing std:: every time
using namespace std;

// ====================
// GLOBAL VARIABLES
// ====================

// Create a 3x3 game board using a 2D vector, filled with empty spaces ' '
// vector<vector<char>> means a vector that contains vectors of characters
vector<vector<char>> board(3, vector<char>(3, ' '));

// Store the username of the current player
string currentUser;

// Store the username of the second player (for 2-player games)
string secondUser;

// Pointer to the SQLite database connection
sqlite3* db;

// Boolean flag to track if we're playing against AI (true) or another player (false)
bool againstAI = false;

// AI difficulty level: 1 = Easy, 2 = Medium, 3 = Hard
int aiDifficulty = 1;

// ====================
// UTILITY FUNCTIONS
// ====================

// Simple hash function to "encrypt" passwords
// This adds up all the ASCII values of characters in the password
string simpleHash(const string& password) {
    int hash = 0; // Start with 0
    
    // Loop through each character in the password
    for (char c : password) {
        hash += c; // Add the ASCII value of each character
    }
    
    // Convert the integer hash back to a string and return it
    return to_string(hash);
}

// Escape single quotes in strings to prevent SQL injection attacks
// SQL injection is when malicious code is inserted into SQL queries
string escapeString(const string& str) {
    string result; // Create an empty result string
    
    // Go through each character in the input string
    for (char c : str) {
        if (c == '\'') { // If we find a single quote
            result += "''"; // Replace it with two single quotes (SQL escape)
        }
        else {
            result += c; // Otherwise, just add the character normally
        }
    }
    return result; // Return the escaped string
}

// Execute SQL commands and handle any errors
bool executeSQL(const string& sql) {
    char* errMsg = 0; // Pointer to hold error messages
    
    // Execute the SQL command
    // sqlite3_exec runs the SQL and returns a result code
    int rc = sqlite3_exec(db, sql.c_str(), 0, 0, &errMsg);
    
    // Check if the execution was successful
    if (rc != SQLITE_OK) {
        // If there was an error, print it to the console
        cerr << "SQL error: " << errMsg << endl;
        sqlite3_free(errMsg); // Free the error message memory
        return false; // Return false to indicate failure
    }
    return true; // Return true to indicate success
}

// ====================
// DATABASE FUNCTIONS
// ====================

// Create the database tables if they don't already exist
void setupDatabase() {
    // SQL command to create the Users table
    // INTEGER PRIMARY KEY AUTOINCREMENT means the id will automatically increment
    // TEXT UNIQUE means the username must be unique (no duplicates)
    string createUsers = "CREATE TABLE IF NOT EXISTS Users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT UNIQUE,"
        "password TEXT);";

    // SQL command to create the History table to track game results
    // TIMESTAMP DEFAULT CURRENT_TIMESTAMP automatically sets the current date/time
    string createHistory = "CREATE TABLE IF NOT EXISTS History ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT,"
        "result TEXT,"
        "date TIMESTAMP DEFAULT CURRENT_TIMESTAMP);";

    // Execute both SQL commands to create the tables
    executeSQL(createUsers);
    executeSQL(createHistory);
}

// Register a new user in the database
bool registerUser(string& username) {
    string password; // Variable to store the entered password
    
    // Ask user for username and password
    cout << "Enter new username: ";
    cin >> username; // Read username from console input
    cout << "Enter new password: ";
    cin >> password; // Read password from console input

    // Escape the username to prevent SQL injection
    string escapedUsername = escapeString(username);
    
    // Hash the password for basic security
    string hashedPassword = simpleHash(password);

    // Use stringstream to build the SQL INSERT query
    stringstream ss;
    ss << "INSERT INTO Users (username, password) VALUES ('"
        << escapedUsername << "', '" << hashedPassword << "');";

    // Try to execute the SQL command
    if (executeSQL(ss.str())) {
        cout << "User registered successfully!\n";
        return true; // Registration successful
    }
    else {
        cout << "Registration failed. Username might already exist.\n";
        return false; // Registration failed
    }
}

// Login an existing user
bool loginUser(string& username) {
    string password; // Variable to store the entered password
    
    // Ask user for login credentials
    cout << "Enter username: ";
    cin >> username;
    cout << "Enter password: ";
    cin >> password;

    // Escape username and hash password (same as registration)
    string escapedUsername = escapeString(username);
    string hashedPassword = simpleHash(password);

    // Build SQL SELECT query to find matching user
    stringstream ss;
    ss << "SELECT * FROM Users WHERE username = '"
        << escapedUsername << "' AND password = '" << hashedPassword << "';";

    // Prepare the SQL statement for execution
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, ss.str().c_str(), -1, &stmt, 0);

    // Check if the SQL preparation was successful
    if (rc != SQLITE_OK) {
        cerr << "Failed to fetch data: " << sqlite3_errmsg(db) << endl;
        return false;
    }

    // Execute the prepared statement
    rc = sqlite3_step(stmt);
    
    // Check if we found a matching row (user exists with correct password)
    if (rc == SQLITE_ROW) {
        cout << "Login successful! Welcome " << username << "!\n";
        sqlite3_finalize(stmt); // Clean up the prepared statement
        return true; // Login successful
    }
    else {
        cout << "Invalid username or password.\n";
        sqlite3_finalize(stmt); // Clean up the prepared statement
        return false; // Login failed
    }
}

// ====================
// GAME BOARD FUNCTIONS
// ====================

// Display the current state of the game board
void displayBoard() {
    cout << "\n"; // Print a blank line for spacing
    
    // Print column numbers (0, 1, 2) as headers
    cout << "  0   1   2\n";
    
    // Loop through each row (i = row index)
    for (int i = 0; i < 3; i++) {
        cout << i << " "; // Print the row number
        
        // Loop through each column in the current row
        for (int j = 0; j < 3; j++) {
            cout << board[i][j]; // Print the character at this position
            if (j < 2) cout << " | "; // Print separator between columns (not after last column)
        }
        cout << "\n"; // End the line
        
        // Print horizontal separator line between rows (not after last row)
        if (i < 2) cout << "  ---------\n";
    }
    cout << "\n"; // Print another blank line for spacing
}

// Check if there's a winner on the board
char checkWinner() {
    // Check rows - look for three in a row horizontally
    for (int i = 0; i < 3; i++) {
        // Check if all three positions in row i have the same non-empty character
        if (board[i][0] != ' ' &&           // First position is not empty
            board[i][0] == board[i][1] &&   // First equals second
            board[i][1] == board[i][2])     // Second equals third
            return board[i][0]; // Return the winning character (X or O)
    }

    // Check columns - look for three in a row vertically
    for (int i = 0; i < 3; i++) {
        // Check if all three positions in column i have the same non-empty character
        if (board[0][i] != ' ' &&           // Top position is not empty
            board[0][i] == board[1][i] &&   // Top equals middle
            board[1][i] == board[2][i])     // Middle equals bottom
            return board[0][i]; // Return the winning character
    }

    // Check diagonal from top-left to bottom-right
    if (board[0][0] != ' ' &&               // Top-left is not empty
        board[0][0] == board[1][1] &&       // Top-left equals center
        board[1][1] == board[2][2])         // Center equals bottom-right
        return board[0][0]; // Return the winning character

    // Check diagonal from top-right to bottom-left
    if (board[0][2] != ' ' &&               // Top-right is not empty
        board[0][2] == board[1][1] &&       // Top-right equals center
        board[1][1] == board[2][0])         // Center equals bottom-left
        return board[0][2]; // Return the winning character

    return ' '; // No winner found, return empty space
}

// Check if the game is a draw (board is full with no winner)
bool isDraw() {
    // Loop through every position on the board
    for (auto& row : board) {           // For each row
        for (char cell : row) {         // For each cell in the row
            if (cell == ' ')            // If we find an empty space
                return false;           // Game is not a draw yet
        }
    }
    return true; // All spaces are filled, it's a draw
}

// ====================
// AI FUNCTIONS
// ====================

// Easy AI - makes random moves
/*
================================================================================
                           EASY AI ALGORITHM EXPLANATION
================================================================================

The Easy AI uses the simplest possible strategy: RANDOM MOVES.

BASIC CONCEPT:
- Look at all empty spaces on the board
- Pick one randomly
- Make the move there

HOW IT WORKS STEP BY STEP:

1. SCAN THE BOARD: Go through every position (0,0) to (2,2)
2. FIND EMPTY SPACES: If a position contains ' ' (space), it's available
3. COLLECT MOVES: Add all available positions to a list
4. RANDOM SELECTION: Use random number generator to pick one move
5. RETURN MOVE: Give back the randomly selected position

EXAMPLE:
Current Board:        Available Moves:      Random Selection:
X | O |              (0,2), (1,0),         Let's say random number
---------            (1,2), (2,0),         picks index 2, so we
  | X |              (2,1)                 choose (1,2)
---------            
O |   |              

ADVANTAGES:
- Very fast (no complex calculations)
- Unpredictable (keeps games interesting)
- Easy to implement and understand
- Never gets stuck in analysis paralysis

DISADVANTAGES:
- Doesn't try to win
- Doesn't try to block opponent from winning
- Makes obviously bad moves sometimes
- Easy for humans to beat

WHEN TO USE:
- Beginner players who want a relaxed game
- When you want unpredictable, fun gameplay
- For testing purposes
- As a fallback when other AI strategies fail

This AI difficulty is perfect for new players learning the game!
================================================================================
*/

pair<int, int> easyAIMove() {
    // Create a list to store all available moves
    vector<pair<int, int>> moves;
    
    // Loop through the entire board to find empty spaces
    for (int i = 0; i < 3; i++)         // For each row
        for (int j = 0; j < 3; j++)     // For each column
            if (board[i][j] == ' ')     // If this position is empty
                moves.push_back({ i, j }); // Add this position to available moves

    // Safety check - if no moves available, return invalid position
    if (moves.empty()) return { -1, -1 };
    
    // Return a random move from the available moves
    // rand() % moves.size() gives a random index from 0 to moves.size()-1
    return moves[rand() % moves.size()];
}

// Medium AI - tries to win, then block, then random


/*
================================================================================
                          MEDIUM AI ALGORITHM EXPLANATION
================================================================================

The Medium AI uses a STRATEGIC PRIORITY system with three levels of decision making.
This creates a balanced opponent that's challenging but not unbeatable.

BASIC CONCEPT:
- Use a priority-based decision system
- Always check for winning moves first
- Then check for blocking moves
- Finally, fall back to random moves

THE THREE-TIER STRATEGY:

PRIORITY 1: WIN THE GAME (Offensive Strategy)
- Check every empty position on the board
- Temporarily place AI's symbol (O) there
- See if this creates a winning condition (3 in a row)
- If yes, make that move immediately!

PRIORITY 2: BLOCK THE OPPONENT (Defensive Strategy)  
- If no winning move exists, check for opponent threats
- For each empty position, temporarily place opponent's symbol (X)
- See if this would let the opponent win on their next turn
- If yes, block by placing AI's symbol there!

PRIORITY 3: RANDOM MOVE (Fallback Strategy)
- If no winning or blocking moves exist, make a random move
- This uses the same logic as Easy AI

HOW IT WORKS STEP BY STEP:

1. OFFENSIVE CHECK: Try every empty position
   - Place AI symbol temporarily
   - Check if this wins the game
   - If yes, return this position
   - If no, undo and try next position

2. DEFENSIVE CHECK: Try every empty position again
   - Place OPPONENT symbol temporarily  
   - Check if this would win for opponent
   - If yes, block by returning this position
   - If no, undo and try next position

3. FALLBACK: If no strategic moves found, pick randomly

DETAILED EXAMPLE:

Current Board:
X | X |     <- Human has two X's in top row!
---------
O |   |     
---------
  | O |     

Step 1 - Check for AI winning moves:
- Try (0,2): X | X | O  <- No win for AI
- Try (1,1): No win for AI  
- Try (2,0): No win for AI
- No winning moves found, go to step 2

Step 2 - Check for blocking moves:
- Try (0,2) with X: X | X | X  <- HUMAN WOULD WIN!
- Block by placing O at (0,2): X | X | O
- Return position (0,2)

Final Board after AI move:
X | X | O   <- AI blocked the human from winning!
---------
O |   |     
---------
  | O |     

ANOTHER EXAMPLE - AI WINS:

Current Board:
X |   | X   
---------
O | O |     <- AI has two O's in middle row!
---------
  |   |     

Step 1 - Check for AI winning moves:
- Try (1,2): O | O | O  <- AI WINS!
- Return position (1,2) immediately

AI INTELLIGENCE CHARACTERISTICS:
- Opportunistic: Never misses a chance to win
- Defensive: Always blocks opponent from winning  
- Tactical: Makes strategic moves when possible
- Unpredictable: Uses random moves when no strategy applies

ADVANTAGES:
- Much smarter than Easy AI
- Creates engaging, competitive games
- Good balance of challenge and winnability
- Fast execution (no deep analysis)
- Teaches players to think strategically

DISADVANTAGES:
- Still makes some suboptimal moves
- Doesn't plan multiple moves ahead
- Can be defeated by advanced strategies
- Random fallback can sometimes be poor

WHEN TO USE:
- Players who have mastered Easy difficulty
- Those who want tactical gameplay without perfection
- Competitive but not frustrating games
- Good stepping stone to Hard difficulty

This AI provides the perfect balance between challenge and fun!
================================================================================
*/
pair<int, int> mediumAIMove(char aiChar, char playerChar) {
    // First priority: Try to win
    // Check every empty position to see if placing our character there wins the game
    for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) {
        if (board[i][j] == ' ') {        // If position is empty
            board[i][j] = aiChar;        // Temporarily place AI character
            if (checkWinner() == aiChar) { // Check if this wins the game
                board[i][j] = ' ';       // Remove the temporary placement
                return { i, j };         // Return this winning move
            }
            board[i][j] = ' ';           // Remove the temporary placement
        }
    }

    // Second priority: Try to block the player from winning
    // Check every empty position to see if the player would win by placing there
    for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) {
        if (board[i][j] == ' ') {          // If position is empty
            board[i][j] = playerChar;      // Temporarily place player character
            if (checkWinner() == playerChar) { // Check if this would win for player
                board[i][j] = ' ';         // Remove the temporary placement
                return { i, j };           // Return this blocking move
            }
            board[i][j] = ' ';             // Remove the temporary placement
        }
    }

    // If no winning or blocking move found, make a random move
    return easyAIMove();
}

// Hard AI - Uses minimax algorithm for perfect play
// This is a recursive function that explores all possible future game states

/*
================================================================================
                           MINIMAX ALGORITHM EXPLANATION
================================================================================
https://www.youtube.com/watch?v=5y2a0Zhgq0U // this video explains the concept of minimax
https://www.youtube.com/watch?v=l-hh51ncgDI //this video explains the steps to be done at each play to determine the best play 

The Minimax algorithm is a decision-making algorithm used in game theory and 
artificial intelligence for two-player, zero-sum games (like Tic-Tac-Toe, 
Chess, Checkers, etc.). Here's how it works:

BASIC CONCEPT:
- The algorithm assumes both players play optimally (make the best possible moves)
- It explores ALL possible future game states to find the best move
- It uses a "game tree" where each node represents a game state
- Each branch represents a possible move from that state

THE MINIMAX PRINCIPLE:
- The MAXIMIZING player (AI) tries to get the HIGHEST score possible
- The MINIMIZING player (Human) tries to get the LOWEST score possible
- The algorithm alternates between these two perspectives

SCORING SYSTEM:
- +10: AI wins (good for AI)
- -10: Human wins (bad for AI)  
-  0:  Draw (neutral)

HOW THE ALGORITHM WORKS STEP BY STEP:

1. START: Begin with the current game state
2. TERMINAL CHECK: If the game is over (win/lose/draw), return the score
3. GENERATE MOVES: Find all possible moves from current position
4. RECURSION: For each possible move:
   - Make the move temporarily
   - Call minimax recursively for the opponent's turn
   - Undo the move
   - Keep track of the best score found
5. RETURN: Return the best score for the current player

EXAMPLE WITH A SIMPLE GAME STATE:
Let's say it's the AI's turn and there are 3 possible moves:

Current Board:        Move 1 Result:     Move 2 Result:     Move 3 Result:
X | O |              X | O | O          X | O |            X | O |    
---------            ---------          ---------          ---------
  | X |              O | X |            O | X |            O | X | O  
---------            ---------          ---------          ---------
O |   |              O |   |            O |   |            O |   |    

The algorithm would:
1. Try Move 1: AI places O at position (0,2)
   - Recursively check all human responses
   - Find that human can win next turn
   - Score: -10 (bad for AI)

2. Try Move 2: AI places O at position (1,0)  
   - Recursively check all human responses
   - Find that game continues with various outcomes
   - Score: 0 (neutral)

3. Try Move 3: AI places O at position (2,2)
   - Recursively check all human responses  
   - Find that AI wins immediately!
   - Score: +10 (great for AI)

Result: AI chooses Move 3 because +10 > 0 > -10

THE RECURSIVE NATURE:
The algorithm calls itself recursively, switching between maximizing and 
minimizing at each level:

Level 0 (AI turn):    Maximize score
Level 1 (Human turn): Minimize score  
Level 2 (AI turn):    Maximize score
Level 3 (Human turn): Minimize score
... and so on until the game ends

MINIMAX TREE EXAMPLE:
                    Current State (AI turn, MAX)
                   /         |         \
              Move 1       Move 2       Move 3
             (Score: -10)  (Score: 0)   (Score: +10)
                |             |             |
        Human responses  Human responses  Game Over!
        (MIN level)      (MIN level)      Return +10
            |                |
        AI responses     AI responses
        (MAX level)      (MAX level)
            |                |
           ...              ...

WHY IT WORKS:
- By exploring ALL possible moves, the AI never misses a winning opportunity
- By assuming the opponent plays optimally, the AI prepares for the worst case
- The recursive structure ensures every possible game sequence is considered
- The alternating min/max ensures each player's best interests are represented

COMPUTATIONAL COMPLEXITY:
- Time Complexity: O(b^d) where b = branching factor, d = depth
- For Tic-Tac-Toe: At most 9! = 362,880 positions to check
- This is manageable for modern computers, but for complex games like Chess,
  optimizations like Alpha-Beta pruning are needed

ADVANTAGES:
- Guarantees optimal play (never loses if a winning/drawing move exists)
- Works for any two-player, zero-sum game
- Easy to understand and implement

DISADVANTAGES:
- Can be computationally expensive for complex games
- Requires complete game tree exploration
- Assumes opponent plays optimally (may not be true for human players)

In our Tic-Tac-Toe implementation, this makes the "Hard" AI unbeatable!
================================================================================
*/





int minimax(bool isAI, char aiChar, char playerChar) {
    // Check if the game is already over
    char winner = checkWinner();
    if (winner == aiChar) return 10;      // AI wins = good for AI
    if (winner == playerChar) return -10; // Player wins = bad for AI
    if (isDraw()) return 0;               // Draw = neutral

    // If AI's turn, try to maximize the score
    // If player's turn, try to minimize the score
    int best = isAI ? -1000 : 1000; // Start with worst possible score for current player
    
    // Try every possible move
    for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) {
        if (board[i][j] == ' ') {                    // If position is empty
            board[i][j] = isAI ? aiChar : playerChar; // Make the move
            int score = minimax(!isAI, aiChar, playerChar); // Recursively get score
            board[i][j] = ' ';                       // Undo the move
            
            if (isAI)
                best = max(best, score); // AI wants to maximize score
            else
                best = min(best, score); // Player wants to minimize score
        }
    }
    return best; // Return the best score found
}

// Hard AI move selection using minimax
pair<int, int> hardAIMove(char aiChar, char playerChar) {
    int bestScore = -1000;              // Start with worst possible score
    pair<int, int> bestMove = { -1, -1 }; // Default invalid move

    // Try every possible move and find the one with the best score
    for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) {
        if (board[i][j] == ' ') {        // If position is empty
            board[i][j] = aiChar;        // Make the move
            int score = minimax(false, aiChar, playerChar); // Get score (player's turn next)
            board[i][j] = ' ';           // Undo the move
            
            if (score > bestScore) {     // If this move is better than previous best
                bestScore = score;       // Update best score
                bestMove = { i, j };     // Update best move
            }
        }
    }

    return bestMove; // Return the best move found
}

// ====================
// GAME HISTORY FUNCTIONS
// ====================

// Save the result of a game to the database
void saveGameHistory(const string& result, const string& user) {
    // Escape the strings to prevent SQL injection
    string escapedUser = escapeString(user);
    string escapedResult = escapeString(result);

    // Build SQL INSERT query to save the game result
    stringstream ss;
    ss << "INSERT INTO History (username, result) VALUES ('"
        << escapedUser << "', '" << escapedResult << "');";

    // Execute the SQL command
    executeSQL(ss.str());
}

// Display the game history from the database
void viewHistory() {
    cout << "\nGame History:\n";
    
    // SQL query to get the most recent 20 game results, newest first
    string sql = "SELECT username, result, date FROM History ORDER BY date DESC LIMIT 20;";
    
    // Prepare the SQL statement
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);

    // Check if preparation was successful
    if (rc != SQLITE_OK) {
        cerr << "Failed to fetch history: " << sqlite3_errmsg(db) << endl;
        return;
    }

    // Print table headers
    cout << "----------------------------------------\n";
    cout << "| Username       | Result          | Date                  |\n";
    cout << "----------------------------------------\n";

    // Loop through each row returned by the query
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        // Extract data from each column
        string username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        string result = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        string date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        
        // Print the row with proper formatting
        cout << "| " << username;
        // Add spaces to make columns align properly
        for (size_t i = username.length(); i < 15; i++) cout << " ";
        cout << "| " << result;
        for (size_t i = result.length(); i < 15; i++) cout << " ";
        cout << "| " << date << " |\n";
    }
    cout << "----------------------------------------\n";

    // Clean up the prepared statement
    sqlite3_finalize(stmt);
}

// ====================
// MAIN GAME FUNCTION
// ====================

// Main game loop - handles the actual tic tac toe gameplay
void playGame() {
    // Reset the board to all empty spaces for a new game
    board = vector<vector<char>>(3, vector<char>(3, ' '));
    
    char player = 'X'; // X always goes first
    int row, col;      // Variables to store player's move coordinates

    // Main game loop - continues until someone wins or it's a draw
    while (true) {
        displayBoard(); // Show the current board

        // Determine who the current player is
        string currentPlayerName = (player == 'X') ? currentUser : (againstAI ? "AI" : secondUser);

        // Handle AI move (when playing against AI and it's O's turn)
        if (againstAI && player == 'O') {
            pair<int, int> move; // Variable to store AI's chosen move
            
            // Choose AI move based on difficulty level
            if (aiDifficulty == 1)
                move = easyAIMove();    // Random moves
            else if (aiDifficulty == 2)
                move = mediumAIMove('O', 'X'); // Try to win/block
            else
                move = hardAIMove('O', 'X');   // Perfect play

            // Extract row and column from the AI's move
            row = move.first;
            col = move.second;
            cout << "AI chooses: " << row << ", " << col << endl;
        }
        else {
            // Handle human player move
            cout << "Player " << player << " (" << currentPlayerName << "), enter your move (row and column): ";
            cin >> row >> col;

            // Input validation loop - keep asking until we get valid input
            while (cin.fail() ||           // Input is not a number
                   row < 0 || row > 2 ||   // Row is out of bounds
                   col < 0 || col > 2 ||   // Column is out of bounds
                   board[row][col] != ' ') { // Position is already taken
                
                // If input failed (not a number), clear the error and ignore bad input
                if (cin.fail()) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                }
                cout << "Invalid move. Please enter row (0-2) and column (0-2) for an empty cell: ";
                cin >> row >> col;
            }
        }

        // Place the current player's symbol on the board
        board[row][col] = player;
        
        // Check if this move resulted in a win
        char winner = checkWinner();

        if (winner != ' ') { // Someone won
            displayBoard(); // Show the final board state
            
            if (againstAI) {
                // Handle AI game results
                if (winner == 'X') {
                    cout << "Congratulations " << currentUser << "! You win!\n";
                    saveGameHistory("Win vs AI", currentUser);
                }
                else {
                    cout << "AI wins! Better luck next time!\n";
                    saveGameHistory("Lost vs AI", currentUser);
                }
            }
            else {
                // Handle two-player game results
                string winnerName = (winner == 'X') ? currentUser : secondUser;
                string loserName = (winner == 'X') ? secondUser : currentUser;
                cout << "Congratulations " << winnerName << "! You win!\n";
                // Save results for both players
                saveGameHistory("Won against " + loserName, winnerName);
                saveGameHistory("Lost to " + winnerName, loserName);
            }
            break; // Exit the game loop
        }

        // Check if the game is a draw
        if (isDraw()) {
            displayBoard(); // Show the final board state
            cout << "It's a draw!\n";
            // Save draw result for current player
            saveGameHistory("Draw", currentUser);
            // If playing against another player, save draw for them too
            if (!againstAI) saveGameHistory("Draw", secondUser);
            break; // Exit the game loop
        }

        // Switch to the other player for the next turn
        player = (player == 'X') ? 'O' : 'X';
    }
}

// ====================
// PLAYER MANAGEMENT
// ====================

// Handle registration/login for the second player in two-player games
bool handleSecondPlayer() {
    int choice;              // Store user's menu choice
    bool secondLoggedIn = false; // Track if second player is logged in

    cout << "\nSecond Player Login/Register:\n";
    
    // Loop until second player successfully logs in
    while (!secondLoggedIn) {
        cout << "1. Register\n2. Login\nChoose option: ";
        cin >> choice;

        // Check for invalid input (non-number)
        if (cin.fail()) {
            cin.clear(); // Clear the error flag
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Ignore bad input
            cout << "Invalid input. Please enter 1 or 2.\n";
            continue; // Go back to the beginning of the loop
        }

        // Handle menu choices
        if (choice == 1) {
            secondLoggedIn = registerUser(secondUser); // Try to register
        }
        else if (choice == 2) {
            secondLoggedIn = loginUser(secondUser);    // Try to login
        }
        else {
            cout << "Invalid choice. Try again.\n";
        }

        // Make sure second player isn't the same as first player
        if (secondLoggedIn && secondUser == currentUser) {
            cout << "You cannot play against yourself! Please use a different account.\n";
            secondLoggedIn = false; // Reset login status
        }
    }

    return secondLoggedIn; // Return whether second player successfully logged in
}

// ====================
// MAIN FUNCTION
// ====================

// Main function - program entry point
int main() {
    // Seed the random number generator with current time
    // This ensures different random numbers each time the program runs
    srand(time(0));

    cout << "Welcome to Advanced Tic Tac Toe with SQLite + AI!\n";

    // Open/create the SQLite database file
    int rc = sqlite3_open("game.db", &db);
    if (rc) {
        // If database opening failed, print error and exit
        cerr << "Can't open database: " << sqlite3_errmsg(db) << endl;
        return 1; // Return 1 to indicate program failure
    }
    else {
        cout << "Opened database successfully.\n";
    }

    // Create database tables if they don't exist
    setupDatabase();

    // First player login/registration loop
    int choice;              // Store user's menu choice
    bool loggedIn = false;   // Track if first player is logged in
    
    while (!loggedIn) {
        cout << "1. Register\n2. Login\nChoose option: ";
        cin >> choice;

        // Check for invalid input (non-number)
        if (cin.fail()) {
            cin.clear(); // Clear the error flag
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Ignore bad input
            cout << "Invalid input. Please enter 1 or 2.\n";
            continue; // Go back to the beginning of the loop
        }

        // Handle menu choices
        if (choice == 1) {
            loggedIn = registerUser(currentUser); // Try to register
        }
        else if (choice == 2) {
            loggedIn = loginUser(currentUser);    // Try to login
        }
        else {
            cout << "Invalid choice. Try again.\n";
        }
    }

    // Main menu loop - continues until user chooses to exit
    while (true) {
        cout << "\nMain Menu:\n";
        cout << "1. Play against another Player\n";
        cout << "2. Play against AI\n";
        cout << "3. View History\n";
        cout << "4. Exit\n";
        cout << "Choice: ";
        cin >> choice;

        // Check for invalid input (non-number)
        if (cin.fail()) {
            cin.clear(); // Clear the error flag
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Ignore bad input
            cout << "Invalid input. Please enter a number between 1 and 4.\n";
            continue; // Go back to the beginning of the loop
        }

        // Handle menu choices
        if (choice == 1) {
            // Play against another player
            againstAI = false; // Set flag to indicate human vs human
            if (handleSecondPlayer()) { // If second player successfully logs in
                playGame(); // Start the game
            }
        }
        else if (choice == 2) {
            // Play against AI
            againstAI = true; // Set flag to indicate human vs AI
            
            // Let player choose AI difficulty
            cout << "\nChoose AI Difficulty:\n";
            cout << "1. Easy\n2. Medium\n3. Hard\nChoice: ";
            cin >> aiDifficulty;

            // Validate AI difficulty input
            if (cin.fail() || aiDifficulty < 1 || aiDifficulty > 3) {
                cin.clear(); // Clear error flag
                cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Ignore bad input
                cout << "Invalid difficulty. Setting to Easy (1).\n";
                aiDifficulty = 1; // Default to easy
            }

            playGame(); // Start the game against AI
        }
        else if (choice == 3) {
            // View game history
            viewHistory();
        }
        else if (choice == 4) {
            // Exit the program
            cout << "Goodbye!\n";
            break; // Exit the main loop
        }
        else {
            cout << "Invalid choice. Try again.\n";
        }
    }

    // Close the database connection before exiting
    sqlite3_close(db);
    return 0; // Return 0 to indicate successful program completion
}
