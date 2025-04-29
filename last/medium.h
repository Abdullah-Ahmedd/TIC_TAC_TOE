#ifndef MEDIUM_H
#define MEDIUM_H

#include <QMainWindow>
#include <QPushButton>
#include <QVector>
#include <QPair>

class medium : public QMainWindow
{
    Q_OBJECT

public:
    explicit medium(QWidget *parent = nullptr);
    ~medium();

private slots:
    void handleCellClicked(int row, int col);
    void undoMove();
    void backToLevel();

private:
    QPushButton* cells[3][3];
    QVector<QPair<int, int>> moveHistory;
    bool gameOver;

    void aiMove();         // AI Medium
    void applyMove(int row, int col, const QString& symbol);
    void clearCell(int row, int col);
    bool checkWinner(const QString& symbol);
    bool isDraw();
};

#endif // MEDIUM_H

