#ifndef FRIEND_H
#define FRIEND_H

#include <QMainWindow>
#include <QPushButton>
#include <QVector>
#include <QPair>

class Friend : public QMainWindow
{
    Q_OBJECT

public:
    explicit Friend(QWidget *parent = nullptr);
    ~Friend();

private slots:
    void handleButtonClicked(int index);
    void checkWin();
    void resetBoard();
    void undoMove();     // ✅ زر Undo
    void backToMenu();   // ✅ زر Back

private:
    QVector<QPushButton*> buttons;
    bool xTurn;
    QVector<QPair<int, QString>> moveHistory; // ✅ تاريخ الحركات
};

#endif // FRIEND_H

