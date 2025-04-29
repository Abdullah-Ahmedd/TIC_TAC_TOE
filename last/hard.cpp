#include "hard.h"
#include <QPushButton>
#include <QGridLayout>
#include <QMessageBox>
#include <QFont>
#include <QPalette>
#include <QLinearGradient>
#include <QBrush>

hard::hard(QWidget *parent)
    : QMainWindow(parent), xTurn(true)  // X starts the game
{
    resize(400, 400);

    // Set background gradient (baby blue)
    QPalette palette;
    QLinearGradient gradient(0, 0, 0, height());
    gradient.setColorAt(0.0, QColor(240, 248, 255));  // Light Blue at top
    gradient.setColorAt(1.0, QColor(220, 240, 255));  // Lighter Blue at bottom
    palette.setBrush(QPalette::Window, QBrush(gradient));
    this->setAutoFillBackground(true);
    this->setPalette(palette);

    QWidget *central = new QWidget(this);
    QGridLayout *grid = new QGridLayout(central);
    grid->setSpacing(5);

    QFont font;
    font.setPointSize(28);
    font.setBold(true);

    // Create buttons for the Tic-Tac-Toe grid
    for (int i = 0; i < 9; ++i) {
        QPushButton *btn = new QPushButton("");
        btn->setFixedSize(100, 100);
        btn->setFont(font);
        btn->setStyleSheet(
            "QPushButton { background-color: white; border: 2px solid #aaa; border-radius: 12px; }"
            );
        connect(btn, &QPushButton::clicked, this, [=]() { makeMove(i); });
        grid->addWidget(btn, i / 3, i % 3);
        buttons.append(btn);
    }

    central->setLayout(grid);
    setCentralWidget(central);
}

hard::~hard() {}

void hard::makeMove(int index)
{
    QPushButton *btn = buttons[index];
    if (!btn->text().isEmpty()) return;  // Ignore if the cell is already filled

    // Player's move (X)
    if (xTurn) {
        btn->setText("X");
        btn->setStyleSheet("QPushButton { color: red; font-weight: bold; background-color: white; border: 2px solid #aaa; border-radius: 12px; }");
    } else {  // AI's move (O)
        btn->setText("O");
        btn->setStyleSheet("QPushButton { color: blue; font-weight: bold; background-color: white; border: 2px solid #aaa; border-radius: 12px; }");
    }

    xTurn = !xTurn;
    checkWin();

    // If it's the AI's turn, make the AI move automatically
    if (!xTurn) {
        aiMove();  // AI calculates its move
    }
}

void hard::checkWin()
{
    // Check for winner
    if (isWinning('X')) {
        QMessageBox::information(this, "الفائز", "اللاعب X فاز!");
        resetBoard();
        return;
    }

    if (isWinning('O')) {
        QMessageBox::information(this, "الفائز", "اللاعب O فاز!");
        resetBoard();
        return;
    }

    // Check for draw
    if (isBoardFull()) {
        QMessageBox::information(this, "تعادل", "انتهت المباراة بالتعادل!");
        resetBoard();
    }
}

void hard::resetBoard()
{
    for (QPushButton *btn : buttons) {
        btn->setText("");
        btn->setStyleSheet("QPushButton { background-color: white; border: 2px solid #aaa; border-radius: 12px; }");
    }
    xTurn = true;  // Reset the game for the next match, starting with Player X
}

// Minimax algorithm to calculate the best move
int hard::minimax(int depth, bool isMaximizing)
{
    int score = evaluateBoard();

    // If AI wins or player wins, return the score
    if (score == 10) return score;
    if (score == -10) return score;

    // If board is full, it's a draw
    if (isBoardFull()) return 0;

    // Maximizing for AI (O)
    if (isMaximizing) {
        int best = -1000;

        // Try all possible moves
        for (int i = 0; i < 9; i++) {
            if (buttons[i]->text().isEmpty()) {
                buttons[i]->setText("O");
                best = std::max(best, minimax(depth + 1, !isMaximizing));
                buttons[i]->setText("");
            }
        }

        return best;
    } else {  // Minimizing for player (X)
        int best = 1000;

        // Try all possible moves
        for (int i = 0; i < 9; i++) {
            if (buttons[i]->text().isEmpty()) {
                buttons[i]->setText("X");
                best = std::min(best, minimax(depth + 1, !isMaximizing));
                buttons[i]->setText("");
            }
        }

        return best;
    }
}

// Evaluate the board to check if there is a winner
int hard::evaluateBoard()
{
    // Check rows, columns and diagonals
    if (isWinning('O')) return 10;
    if (isWinning('X')) return -10;
    return 0;
}

// Check if the board is full
bool hard::isBoardFull()
{
    for (QPushButton *btn : buttons) {
        if (btn->text().isEmpty()) return false;
    }
    return true;
}

// Check if a player has won
bool hard::isWinning(char player)
{
    const int winCombos[8][3] = {
        {0, 1, 2}, {3, 4, 5}, {6, 7, 8}, // Rows
        {0, 3, 6}, {1, 4, 7}, {2, 5, 8}, // Columns
        {0, 4, 8}, {2, 4, 6}            // Diagonals
    };

    for (const auto &combo : winCombos) {
        if (buttons[combo[0]]->text() == QString(1, player) &&
            buttons[combo[1]]->text() == QString(1, player) &&
            buttons[combo[2]]->text() == QString(1, player)) {
            return true;
        }
    }

    return false;
}

// Get the best move for the AI using minimax
int hard::getBestMove()
{
    int bestVal = -1000;
    int bestMove = -1;

    for (int i = 0; i < 9; i++) {
        if (buttons[i]->text().isEmpty()) {
            buttons[i]->setText("O");
            int moveVal = minimax(0, false);
            buttons[i]->setText("");
            if (moveVal > bestVal) {
                bestMove = i;
                bestVal = moveVal;
            }
        }
    }

    return bestMove;
}

void hard::aiMove()
{
    int bestMoveIndex = getBestMove();
    makeMove(bestMoveIndex);  // Make the AI's best move
}
