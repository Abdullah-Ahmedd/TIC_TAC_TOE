#include <iostream>
#include <sqlite3.h>  // Include SQLite3 header
#include "SQLiteCpp.h"

// Callback function for retrieving data
static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    for (int i = 0; i < argc; i++) {
        std::cout << azColName[i] << " = " << (argv[i] ? argv[i] : "NULL") << std::endl;
    }
    std::cout << std::endl;
    return 0;
}

int main() {
    sqlite3 *db;  // Database connection object
    char *errMsg = 0;

    // Open or create the database (test.db) in the current directory
    int rc = sqlite3_open("test.db", &db);  // 'test.db' is the database file
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return (0);
    } else {
        std::cout << "Opened database successfully" << std::endl;
    }

    // Create a table if it doesn't already exist
    const char *createTableSQL = "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY, name TEXT);";
    rc = sqlite3_exec(db, createTableSQL, 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    } else {
        std::cout << "Table created successfully (if not already exists)" << std::endl;
    }

    // Insert some data into the table
    const char *insertSQL = "INSERT INTO users (name) VALUES ('John Doe');";
    rc = sqlite3_exec(db, insertSQL, 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    } else {
        std::cout << "Data inserted successfully" << std::endl;
    }

    // Query the database to retrieve the data
    const char *selectSQL = "SELECT * FROM users;";
    rc = sqlite3_exec(db, selectSQL, callback, 0, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    } else {
        std::cout << "Query executed successfully" << std::endl;
    }

    // Close the database connection
    sqlite3_close(db);
    return 0;
}
