#include <cstring>
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

bool SqliteWrap::select_count_sync (const std::string &table, const std::string &condition, int &count)
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


bool SqliteWrap::select(const std::string &table, const std::string &condition, void* user_param, int (*callback)(void*,int,char**,char**), int &count)
{
    if (!_db)
    {
        std::cerr << "Error: Database not connected." << std::endl;
        return false;
    }

    // get the number of rows
    if (!select_count_sync(table, condition, count)) return false;

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

bool SqliteWrap::select_sync(const std::string &table, const std::string &condition, void* user_param, DeserializeCallback callback )
{
    if (!_db)
    {
        std::cerr << "Error: Database not connected." << std::endl;
        return false;
    }

    std::string sql = "SELECT * FROM " + table;
    if (!condition.empty()) sql += " WHERE " + condition;
    sql += ";";

    sqlite3_stmt *statement = nullptr;

    sqlite3_prepare_v2(_db, sql.c_str(), -1, &statement, 0);        // prepare our query

    int rc = 0;

    while ((rc = sqlite3_step(statement)) == SQLITE_ROW)            // execute sqlite3_step while there are rows to be fetched
    {
        int column_count = sqlite3_column_count(statement);

        char** col_values = new char*[column_count];
        for (int i = 0; i < column_count; i++)
        {
            const unsigned char* column_text = sqlite3_column_text(statement, i);

            if (column_text)
            {
                std::string col_text_str(reinterpret_cast<const char*>(column_text));
                col_values[i] = new char[col_text_str.size() + 1];
                std::strcpy(col_values[i], col_text_str.c_str());
            }
            else
            {
                col_values[i] = nullptr;
            }
        }

        if (!callback(user_param, col_values, column_count))
        {
            delete[] col_values;
            sqlite3_finalize(statement);
            return false;
        }

        delete[] col_values;
    }

    if (rc != SQLITE_DONE)
    {
        sqlite3_finalize(statement);                 // free our statement
        return false;
    }

    sqlite3_finalize(statement);                 // free our statement

    return true;
}



























