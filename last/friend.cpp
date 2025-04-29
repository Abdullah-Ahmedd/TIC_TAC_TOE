#include "friend.h"
#include "menu.h"

#include <QGridLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QFont>
#include <QPalette>
#include <QLinearGradient>
#include <QBrush>
#include <QPushButton>
#include <QHBoxLayout>

Friend::Friend(QWidget *parent)
    : QMainWindow(parent), xTurn(true)
{
    resize(420, 500);

    // خلفية متدرجة baby blue
    QPalette palette;
    QLinearGradient gradient(0, 0, 0, height());
    gradient.setColorAt(0.0, QColor(240, 248, 255));
    gradient.setColorAt(1.0, QColor(220, 240, 255));
    palette.setBrush(QPalette::Window, QBrush(gradient));
    this->setAutoFillBackground(true);
    this->setPalette(palette);

    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    QGridLayout *grid = new QGridLayout();
    grid->setSpacing(5);

    QFont font;
    font.setPointSize(28);
    font.setBold(true);

    for (int i = 0; i < 9; ++i) {
        QPushButton *btn = new QPushButton("");
        btn->setFixedSize(100, 100);
        btn->setFont(font);
        btn->setStyleSheet("QPushButton { background-color: white; border: 2px solid #aaa; border-radius: 12px; }");
        connect(btn, &QPushButton::clicked, this, [=]() { handleButtonClicked(i); });
        buttons.append(btn);
        grid->addWidget(btn, i / 3, i % 3);
    }

    // أزرار Undo و Back
    QPushButton* undoButton = new QPushButton("Undo");
    QPushButton* backButton = new QPushButton("Back");

    undoButton->setStyleSheet("font-size:14px; padding:8px; background-color:#FFD700; border-radius:8px;");
    backButton->setStyleSheet("font-size:14px; padding:8px; background-color:#D3D3D3; border-radius:8px;");

    connect(undoButton, &QPushButton::clicked, this, &Friend::undoMove);
    connect(backButton, &QPushButton::clicked, this, &Friend::backToMenu);

    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->addWidget(undoButton);
    bottomLayout->addStretch();
    bottomLayout->addWidget(backButton);

    mainLayout->addLayout(grid);
    mainLayout->addLayout(bottomLayout);

    setCentralWidget(central);
}

Friend::~Friend() {}

void Friend::handleButtonClicked(int index)
{
    QPushButton *btn = buttons[index];
    if (!btn->text().isEmpty()) return;

    QString mark = xTurn ? "X" : "O";
    btn->setText(mark);
    btn->setStyleSheet(
        QString("QPushButton { color: %1; font-weight:bold; background-color: white; border: 2px solid #aaa; border-radius: 12px; }")
            .arg(xTurn ? "red" : "blue")
        );

    moveHistory.append(qMakePair(index, mark)); // ✅ سجل الحركة
    xTurn = !xTurn;
    checkWin();
}

void Friend::checkWin()
{
    QString winner = "";
    const int winCombos[8][3] = {
        {0,1,2}, {3,4,5}, {6,7,8},
        {0,3,6}, {1,4,7}, {2,5,8},
        {0,4,8}, {2,4,6}
    };

    for (const auto &combo : winCombos) {
        QString a = buttons[combo[0]]->text();
        QString b = buttons[combo[1]]->text();
        QString c = buttons[combo[2]]->text();

        if (!a.isEmpty() && a == b && b == c) {
            winner = a;
            break;
        }
    }

    if (!winner.isEmpty()) {
        QString message = (winner == "X") ? "🎉 الأحمر كسب (X)!" : "🎉 الأزرق كسب (O)!";
        QMessageBox::information(this, "الفائز", message);
        resetBoard();
    } else {
        bool draw = true;
        for (QPushButton *btn : buttons) {
            if (btn->text().isEmpty()) {
                draw = false;
                break;
            }
        }
        if (draw) {
            QMessageBox::information(this, "تعادل", "انتهت المباراة بالتعادل!");
            resetBoard();
        }
    }
}

void Friend::resetBoard()
{
    for (QPushButton *btn : buttons) {
        btn->setText("");
        btn->setStyleSheet("QPushButton { background-color: white; border: 2px solid #aaa; border-radius: 12px; }");
    }
    xTurn = true;
    moveHistory.clear(); // ✅ إعادة تعيين التاريخ
}

void Friend::undoMove()
{
    if (moveHistory.isEmpty()) return;

    QPair<int, QString> last = moveHistory.takeLast();
    int index = last.first;

    buttons[index]->setText("");
    buttons[index]->setStyleSheet("QPushButton { background-color: white; border: 2px solid #aaa; border-radius: 12px; }");

    xTurn = (last.second == "X"); // رجع الدور السابق
}

void Friend::backToMenu()
{
    Menu* menu = new Menu("Player");
    menu->show();
    this->close();
}


