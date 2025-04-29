#ifndef MENU_H
#define MENU_H

#include <QMainWindow>
#include <QPushButton>
#include <QString>

class Menu : public QMainWindow
{
    Q_OBJECT

public:
    explicit Menu(const QString& username, QWidget *parent = nullptr);
    ~Menu();

private:
    QString currentUsername;
    QPushButton *playFriendButton;
    QPushButton *playComputerButton;
    QPushButton *viewHistoryButton;
    QPushButton *backButton;
};

#endif // MENU_H
