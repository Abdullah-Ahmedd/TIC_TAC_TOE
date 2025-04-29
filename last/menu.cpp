#include "menu.h"
#include "friend.h"   // نافذة اللعب مع صديق
#include "level.h"    // نافذة اختيار مستوى الكمبيوتر

#include <QVBoxLayout>
#include <QLabel>
#include <QPalette>
#include <QBrush>
#include <QLinearGradient>

Menu::Menu(const QString &username, QWidget *parent)
    : QMainWindow(parent), currentUsername(username)
{
    resize(420, 500);

    // خلفية متدرجة Baby Blue
    QPalette palette;
    QLinearGradient gradient(0, 0, 0, height());
    gradient.setColorAt(0.0, QColor(240, 248, 255));
    gradient.setColorAt(1.0, QColor(220, 240, 255));
    palette.setBrush(QPalette::Window, QBrush(gradient));
    this->setAutoFillBackground(true);
    this->setPalette(palette);

    QWidget *central = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(central);

    QLabel *welcomeLabel = new QLabel("Welcome, " + currentUsername, this);
    welcomeLabel->setAlignment(Qt::AlignCenter);
    welcomeLabel->setStyleSheet("font-size:18px; font-weight:bold; color:#333;");

    playFriendButton = new QPushButton("Play with Friend", this);
    playComputerButton = new QPushButton("Play with Computer", this);
    viewHistoryButton = new QPushButton("View History", this);
    backButton = new QPushButton("Back", this);

    playFriendButton->setStyleSheet("font-size:16px; padding:10px; background-color:#90EE90; border-radius:8px;");
    playComputerButton->setStyleSheet("font-size:16px; padding:10px; background-color:#ADD8E6; border-radius:8px;");
    viewHistoryButton->setStyleSheet("font-size:16px; padding:10px; background-color:#FFD700; border-radius:8px;");
    backButton->setStyleSheet("font-size:14px; padding:8px; background-color:#D3D3D3; border-radius:8px;");

    layout->addSpacing(10);
    layout->addWidget(welcomeLabel);
    layout->addSpacing(15);
    layout->addWidget(playFriendButton);
    layout->addWidget(playComputerButton);
    layout->addWidget(viewHistoryButton);
    layout->addWidget(backButton);

    setCentralWidget(central);

    // زرار اللعب مع صديق
    connect(playFriendButton, &QPushButton::clicked, this, [=]() {
        Friend *friendWindow = new Friend();
        friendWindow->show();
        this->close();
    });

    // زرار اللعب مع كمبيوتر ➜ يفتح نافذة مستوى الصعوبة
    connect(playComputerButton, &QPushButton::clicked, this, [=]() {
        level *levelWindow = new level();
        levelWindow->show();
        this->close();
    });

    // زر الرجوع
    connect(backButton, &QPushButton::clicked, this, [=]() {
        this->close();  // Close the current window and return to the previous one
    });
}

Menu::~Menu()
{
}


