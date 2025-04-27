#include <iostream>
#include "sqlite3.h"
#include "sqlite3.c"
//#include "sqlite3ext.h"
//#include "shell.c"

using namespace std;

// Callback function to display query results
static int callback(void *data, int argc, char **argv, char **azColName) {
    for (int i = 0; i < argc; i++) {
        cout << azColName[i] << ": " << (argv[i] ? argv[i] : "NULL") << endl;
    }
    return 0;
}

int main() {
    sqlite3 *db;
    char *errMessage = 0;
    
    // Open database (will create one if it doesn't exist)
    int rc = sqlite3_open("test.db", &db);

    if (rc) {
        cout << "Can't open database: " << sqlite3_errmsg(db) << endl;
        return(0);
    } else {
        cout << "Opened database successfully" << endl;
    }

    // Create table
    const char* createTableSQL = "CREATE TABLE IF NOT EXISTS users ("
                                 "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                 "username TEXT NOT NULL,"
                                 "password TEXT NOT NULL);";
    
    rc = sqlite3_exec(db, createTableSQL, callback, 0, &errMessage);

    if (rc != SQLITE_OK) {
        cout << "SQL error: " << errMessage << endl;
        sqlite3_free(errMessage);
    } else {
        cout << "Table created successfully" << endl;
    }

    // Insert some data into the table
    const char* insertSQL = "INSERT INTO users (username, password) VALUES ('user1', 'password123');";
    
    rc = sqlite3_exec(db, insertSQL, callback, 0, &errMessage);

    if (rc != SQLITE_OK) {
        cout << "SQL error: " << errMessage << endl;
        sqlite3_free(errMessage);
    } else {
        cout << "Record inserted successfully" << endl;
    }

    // Select data from the table
    const char* selectSQL = "SELECT * FROM users;";
    
    rc = sqlite3_exec(db, selectSQL, callback, 0, &errMessage);

    if (rc != SQLITE_OK) {
        cout << "SQL error: " << errMessage << endl;
        sqlite3_free(errMessage);
    }

    // Close the database
    sqlite3_close(db);

    return 0;
}
