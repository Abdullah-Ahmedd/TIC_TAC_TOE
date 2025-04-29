#ifndef LEVEL_H
#define LEVEL_H

#include <QMainWindow>
#include <QPushButton>

class level : public QMainWindow
{
    Q_OBJECT

public:
    explicit level(QWidget *parent = nullptr);
    ~level();

private:
    QPushButton* easyButton;
    QPushButton* mediumButton;
    QPushButton* hardButton;
    QPushButton* backButton;
};

#endif // LEVEL_H
