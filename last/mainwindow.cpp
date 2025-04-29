#include "mainwindow.h"
#include "menu.h" // مهم
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QLabel>
#include <QPalette>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    resize(400, 300);

    // خلفية Baby Blue
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(173, 216, 230));
    this->setAutoFillBackground(true);
    this->setPalette(palette);

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("game.db");
    if (!db.open()) {
        QMessageBox::critical(this, "Database Error", db.lastError().text());
    } else {
        setupDatabase();
    }

    QLabel *userLabel = new QLabel("Username:", this);
    QLabel *passLabel = new QLabel("Password:", this);

    usernameLineEdit = new QLineEdit(this);
    passwordLineEdit = new QLineEdit(this);
    passwordLineEdit->setEchoMode(QLineEdit::Password);

    registerButton = new QPushButton("Register", this);
    loginButton = new QPushButton("Login", this);

    registerButton->setStyleSheet("background-color:#87CEFA; font-size:16px; padding:6px; border-radius:8px;");
    loginButton->setStyleSheet("background-color:#87CEFA; font-size:16px; padding:6px; border-radius:8px;");

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->addWidget(registerButton);
    buttonsLayout->addWidget(loginButton);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(30,30,30,30);
    mainLayout->setSpacing(10);
    mainLayout->addWidget(userLabel);
    mainLayout->addWidget(usernameLineEdit);
    mainLayout->addWidget(passLabel);
    mainLayout->addWidget(passwordLineEdit);
    mainLayout->addLayout(buttonsLayout);

    QWidget *central = new QWidget(this);
    central->setLayout(mainLayout);
    setCentralWidget(central);

    connect(registerButton, &QPushButton::clicked, this, &MainWindow::registerUser);
    connect(loginButton, &QPushButton::clicked, this, &MainWindow::loginUser);
}

MainWindow::~MainWindow()
{
    db.close();
}

void MainWindow::setupDatabase()
{
    QSqlQuery query;
    QString createTable = "CREATE TABLE IF NOT EXISTS Users ("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                          "username TEXT UNIQUE,"
                          "password TEXT"
                          ");";
    if (!query.exec(createTable)) {
        QMessageBox::critical(this, "DB Error", "Failed to create table: " + query.lastError().text());
    }
}

QString MainWindow::simpleHash(const QString& password)
{
    int hash = 0;
    for (auto c : password)
        hash += c.unicode();
    return QString::number(hash);
}

void MainWindow::registerUser()
{
    QString username = usernameLineEdit->text().trimmed();
    QString password = passwordLineEdit->text().trimmed();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please fill both username and password.");
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO Users (username, password) VALUES (:username, :password)");
    query.bindValue(":username", username);
    query.bindValue(":password", simpleHash(password));

    if (query.exec()) {
        QMessageBox::information(this, "Success", "Registered successfully!");
    } else {
        QMessageBox::warning(this, "Error", "Registration failed: " + query.lastError().text());
    }
}

void MainWindow::loginUser()
{
    QString username = usernameLineEdit->text().trimmed();
    QString password = passwordLineEdit->text().trimmed();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please fill both username and password.");
        return;
    }

    QSqlQuery query;
    query.prepare("SELECT * FROM Users WHERE username = :username AND password = :password");
    query.bindValue(":username", username);
    query.bindValue(":password", simpleHash(password));

    if (query.exec() && query.next()) {
        QMessageBox::information(this, "Login", "Login successful!");

        // فتح نافذة Menu وتمرير اسم اللاعب
        Menu *menuWindow = new Menu(username);
        menuWindow->show();
        this->close();
    } else {
        QMessageBox::warning(this, "Login", "Invalid username or password.");
    }
}
