#include <filesystem>
#include <iostream>
#include <ostream>

#include "sqlitewrap.h"

SqliteWrap::SqliteWrap() {}

bool SqliteWrap::connect(const std::string &db_name)
{
    // Check if the database file already exists
    if (!std::filesystem::exists(db_name))
    {
        std::cerr << "Error: Database file does not exist: " << db_name << std::endl;
        return false;  // Return an error without attempting to connect
    }

    // Implement SQLite database connection logic using C++17 features
    int rc = sqlite3_open_v2(db_name.c_str(), &_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);

    if (rc != SQLITE_OK)
    {
        // Handle connection error
        std::cerr << "Error opening database: " << sqlite3_errmsg(_db) << std::endl;
        return false;
    }

    // Connection successful
    std::cout << "Connected to database: " << db_name << std::endl;
    return true;
}

void SqliteWrap::connect_(const std::string &db_name)
{
    try
    {
        // Check if the database file already exists
        if (!std::filesystem::exists(db_name))
        {
            throw std::invalid_argument("Error: Database file does not exist: " + db_name);
        }

        // Implement SQLite database connection logic using C++17 features
        int rc = sqlite3_open_v2(db_name.c_str(), &_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);

        if (rc != SQLITE_OK)
        {
            // Handle connection error
            throw std::runtime_error("Error opening database: " + std::string(sqlite3_errmsg(_db)));
        }
        else
        {
            // Connection successful
            std::cout << "Connected to database: " << db_name << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        throw std::runtime_error("Error connecting database: " + std::string(e.what()));
    }
}

bool SqliteWrap::exists(const std::string &db_name)
{
    return std::filesystem::exists(db_name);
}

bool SqliteWrap::create_db(const std::string &db_name)
{
    // Already exists
    if (std::filesystem::exists(db_name)) return false;

    // Implement SQLite database connection logic using C++17 features
    int rc = sqlite3_open_v2(db_name.c_str(), &_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);

    if (rc != SQLITE_OK)
    {
        std::cerr << "Error opening database: " << sqlite3_errmsg(_db) << std::endl;
        return false;
    }

    // Connection successful
    std::cout << "Db Created and connected to : " << db_name << std::endl;

    return true;
}

bool SqliteWrap::delete_db(const std::string &db_name)
{
    // Check if the database file already exists
    if (!std::filesystem::exists(db_name))
    {
        std::cerr << "Error: Database file does not exist: " << db_name << std::endl;
        return false;  // Return an error without attempting to connect
    }

    // Delete the database file
    std::filesystem::remove(db_name);

    return true;
}

bool SqliteWrap::execute_sql(const std::string &sql)
{
    if (!_db)
    {
        std::cerr << "Error: Database not connected." << std::endl;
        return false;
    }

    char* errorMessage = nullptr;
    
    int rc = sqlite3_exec(_db, sql.c_str(), nullptr, nullptr, &errorMessage);

    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL error: " << errorMessage << std::endl;
        _last_error = errorMessage;
        sqlite3_free(errorMessage);
        return false;
    }

    return true;
}

bool SqliteWrap::select_count_async (const std::string &table, const std::string &condition, int &count)
{
    if (!_db)
    {
        std::cerr << "Error: Database not connected." << std::endl;
        return false;
    }

    char *errorMessage = nullptr;

    std::string sql = "SELECT COUNT(*) FROM " + table;
    if (!condition.empty())  sql += " WHERE " + condition;
    sql += ";";

    sqlite3_stmt *statement = nullptr;
    sqlite3_prepare_v2(_db, sql.c_str(), -1, &statement, 0);        // prepare our query

    count = sqlite3_column_int(statement, 0);
    int rc = sqlite3_step(statement);   // retrieve the first row (only row, in this case) of the results

    if (rc != SQLITE_ROW)
    {
        count = -1;
        sqlite3_finalize(statement);                 // free our statement
        return false;
    }

    count = sqlite3_column_int(statement, 0);    // retrieve the value of the first column (0-based)
    sqlite3_finalize(statement);                 // free our statement

    return true;
}

bool SqliteWrap::select_count (const std::string &table, const std::string &condition, int &count)
{

    return true;
}


bool SqliteWrap::select(const std::string &table, const std::string &condition, void* user_param, int (*callback)(void*,int,char**,char**), std::string &result)
{
    if (!_db)
    {
        std::cerr << "Error: Database not connected." << std::endl;
        return false;
    }

    char *errorMessage = nullptr;

    std::string sql = "SELECT * FROM " + table;

    if (!condition.empty())
    {
        sql += " WHERE " + condition;
    }

    sql += ";";

    int rc = sqlite3_exec(_db, sql.c_str(), callback, user_param, &errorMessage);

    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL error: " << errorMessage << std::endl;
        _last_error = errorMessage;
        sqlite3_free(errorMessage);
        return false;
    }

    return true;
}
























