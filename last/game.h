#ifndef GAME_H
#define GAME_H

#include <QWidget>
#include "board.h"  // استيراد كلاس Board لتمثيل اللوحة
#include "ai.h"     // استيراد كلاس AI لتحديد الذكاء الاصطناعي

class game : public QWidget
{
    Q_OBJECT

public:
    explicit game(AI* ai, board* board, QWidget *parent = nullptr);
    ~game();

private:
    AI* ai;  // الذكاء الاصطناعي الذي سيواجه اللاعب
    board* gameBoard;  // لوحة اللعبة
    void nextMove();  // التعامل مع الحركة التالية (اللاعب أو الكمبيوتر)
    bool checkWinner();  // التحقق من الفائز
    bool isBoardFull();  // التحقق إذا كانت اللوحة ممتلئة
};

#endif // GAME_H
