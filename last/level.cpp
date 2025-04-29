#include "level.h"
#include "menu.h"
#include "easy.h"
#include "medium.h"
#include "hard.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPalette>
#include <QLinearGradient>
#include <QBrush>

level::level(QWidget *parent)
    : QMainWindow(parent)
{
    resize(420, 450);

    QPalette palette;
    QLinearGradient gradient(0, 0, 0, height());
    gradient.setColorAt(0.0, QColor(240, 248, 255));
    gradient.setColorAt(1.0, QColor(220, 240, 255));
    palette.setBrush(QPalette::Window, QBrush(gradient));
    setPalette(palette);
    setAutoFillBackground(true);

    QLabel* title = new QLabel("Choose AI Difficulty", this);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size:18px; font-weight:bold; color:#333;");

    easyButton   = new QPushButton("Easy", this);
    mediumButton = new QPushButton("Medium", this);
    hardButton   = new QPushButton("Hard", this);
    backButton   = new QPushButton("Back", this);

    easyButton->setStyleSheet("font-size:16px; padding:10px; background-color:#FFC0C0; border-radius:8px;");
    mediumButton->setStyleSheet("font-size:16px; padding:10px; background-color:#FF7F7F; border-radius:8px;");
    hardButton->setStyleSheet("font-size:16px; padding:10px; background-color:#FF0000; border-radius:8px;");
    backButton->setStyleSheet("font-size:14px; padding:8px; background-color:#D3D3D3; border-radius:8px;");

    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(50, 30, 50, 30);
    layout->setSpacing(20);

    layout->addWidget(title);
    layout->addWidget(easyButton);
    layout->addWidget(mediumButton);
    layout->addWidget(hardButton);
    layout->addWidget(backButton);

    QWidget* central = new QWidget(this);
    central->setLayout(layout);
    setCentralWidget(central);

    // الربط بكلاسات اللعبة الصغيرة
    connect(easyButton, &QPushButton::clicked, this, [=]() {
        easy* game = new easy(this);
        game->show();
        this->hide();
    });

    connect(mediumButton, &QPushButton::clicked, this, [=]() {
        medium* game = new medium(this);
        game->show();
        this->hide();
    });

    connect(hardButton, &QPushButton::clicked, this, [=]() {
        hard* game = new hard(this);
        game->show();
        this->hide();
    });

    connect(backButton, &QPushButton::clicked, this, [=]() {
        Menu* m = new Menu("Player");
        m->show();
        this->close();
    });
}

level::~level() {}

