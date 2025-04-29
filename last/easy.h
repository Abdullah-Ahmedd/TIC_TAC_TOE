#ifndef EASY_H
#define EASY_H

#include <QMainWindow>
#include <QPushButton>
#include <QVector>
#include <QPair>

class easy : public QMainWindow
{
    Q_OBJECT

public:
    explicit easy(QWidget *parent = nullptr);
    ~easy();

private slots:
    void handleCellClicked(int row, int col);
    void undoMove();
    void backToLevel();

private:
    QPushButton* cells[3][3];
    QVector<QPair<int, int>> moveHistory;
    bool gameOver;

    void aiMove();         // AI Easy
    void applyMove(int row, int col, const QString& symbol);
    void clearCell(int row, int col);
    bool checkWinner(const QString& symbol);
    bool isDraw();
};

#endif // EASY_H
