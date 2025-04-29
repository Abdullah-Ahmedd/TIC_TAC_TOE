#include "medium.h"
#include "level.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QPalette>
#include <QLinearGradient>
#include <QFont>
#include <cstdlib>
#include <ctime>

medium::medium(QWidget *parent)
    : QMainWindow(parent), gameOver(false)
{
    std::srand(unsigned(std::time(nullptr)));
    resize(450, 550);

    QPalette palette;
    QLinearGradient grad(0, 0, 0, height());
    grad.setColorAt(0.0, QColor(240, 248, 255));
    grad.setColorAt(1.0, QColor(220, 240, 255));
    palette.setBrush(QPalette::Window, QBrush(grad));
    setPalette(palette);

    QFont font;
    font.setPointSize(32);
    QGridLayout* grid = new QGridLayout();

    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            cells[r][c] = new QPushButton("");
            cells[r][c]->setFixedSize(120, 120);
            cells[r][c]->setFont(font);
            cells[r][c]->setStyleSheet("background-color: white; border: 2px solid #333; border-radius: 10px;");
            connect(cells[r][c], &QPushButton::clicked, this, [=]() { handleCellClicked(r, c); });
            grid->addWidget(cells[r][c], r, c);
        }
    }

    QPushButton* undoButton = new QPushButton("Undo");
    QPushButton* backButton = new QPushButton("Back");
    undoButton->setStyleSheet("font-size:16px; padding:8px; background-color:#FFD700; border-radius:8px;");
    backButton->setStyleSheet("font-size:16px; padding:8px; background-color:#D3D3D3; border-radius:8px;");

    connect(undoButton, &QPushButton::clicked, this, &medium::undoMove);
    connect(backButton, &QPushButton::clicked, this, &medium::backToLevel);

    QHBoxLayout* controls = new QHBoxLayout();
    controls->addWidget(undoButton);
    controls->addStretch();
    controls->addWidget(backButton);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addLayout(grid);
    layout->addLayout(controls);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(20);

    QWidget* central = new QWidget(this);
    central->setLayout(layout);
    setCentralWidget(central);
}

medium::~medium() {}

void medium::handleCellClicked(int row, int col)
{
    if (gameOver || !cells[row][col]->text().isEmpty()) return;

    applyMove(row, col, "X");
    moveHistory.append({row, col});

    if (checkWinner("X")) {
        gameOver = true;
        QMessageBox::information(this, "Game Over", "🎉 You win!");
        return;
    }

    if (isDraw()) {
        gameOver = true;
        QMessageBox::information(this, "Game Over", "It's a draw!");
        return;
    }

    aiMove();

    if (checkWinner("O")) {
        gameOver = true;
        QMessageBox::information(this, "Game Over", "Computer wins!");
    } else if (isDraw()) {
        gameOver = true;
        QMessageBox::information(this, "Game Over", "It's a draw!");
    }
}

void medium::aiMove()
{
    QVector<QPair<int, int>> emptyCells;
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c)
            if (cells[r][c]->text().isEmpty())
                emptyCells.append({r, c});

    if (emptyCells.isEmpty()) return;

    // Logic for Medium AI: Try to block or win
    QPair<int, int> move = emptyCells[std::rand() % emptyCells.size()];
    applyMove(move.first, move.second, "O");
    moveHistory.append(move);
}

void medium::applyMove(int row, int col, const QString& symbol)
{
    cells[row][col]->setText(symbol);
    cells[row][col]->setEnabled(false);
    cells[row][col]->setStyleSheet(
        QString("background-color: white; border: 2px solid #333; border-radius: 10px; color: %1; font-weight:bold;")
            .arg(symbol == "X" ? "red" : "blue")
        );
}

void medium::clearCell(int row, int col)
{
    cells[row][col]->setText("");
    cells[row][col]->setEnabled(true);
    cells[row][col]->setStyleSheet("background-color: white; border: 2px solid #333; border-radius: 10px;");
}

bool medium::checkWinner(const QString& symbol)
{
    for (int i = 0; i < 3; ++i)
        if ((cells[i][0]->text() == symbol && cells[i][1]->text() == symbol && cells[i][2]->text() == symbol) ||
            (cells[0][i]->text() == symbol && cells[1][i]->text() == symbol && cells[2][i]->text() == symbol))
            return true;

    return (cells[0][0]->text() == symbol && cells[1][1]->text() == symbol && cells[2][2]->text() == symbol) ||
           (cells[0][2]->text() == symbol && cells[1][1]->text() == symbol && cells[2][0]->text() == symbol);
}

bool medium::isDraw()
{
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c)
            if (cells[r][c]->text().isEmpty())
                return false;
    return true;
}

void medium::undoMove()
{
    if (moveHistory.size() < 2) return;
    QPair<int, int> ai = moveHistory.takeLast();
    QPair<int, int> user = moveHistory.takeLast();
    clearCell(ai.first, ai.second);
    clearCell(user.first, user.second);
    gameOver = false;
}

void medium::backToLevel()
{
    level* l = new level();
    l->show();
    this->close();
}

