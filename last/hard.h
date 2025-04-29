#ifndef HARD_H
#define HARD_H

#include <QMainWindow>
#include <QPushButton>
#include <QVector>

class hard : public QMainWindow
{
    Q_OBJECT

public:
    explicit hard(QWidget *parent = nullptr);
    ~hard();

private slots:
    void makeMove(int index);
    void checkWin();
    void resetBoard();
    void aiMove();

private:
    QVector<QPushButton*> buttons;  // All buttons on the Tic-Tac-Toe board
    bool xTurn;  // true = X (Red), false = O (Blue)
    int minimax(int depth, bool isMaximizing);
    int evaluateBoard();
    bool isBoardFull();
    bool isWinning(char player);
    int getBestMove();
};

#endif // HARD_H


