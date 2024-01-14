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
    int rc = sqlite3_open_v2(db_name.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);

    if (rc != SQLITE_OK)
    {
        // Handle connection error
        std::cerr << "Error opening database: " << sqlite3_errmsg(db) << std::endl;
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
        int rc = sqlite3_open_v2(db_name.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);

        if (rc != SQLITE_OK)
        {
            // Handle connection error
            throw std::runtime_error("Error opening database: " + std::string(sqlite3_errmsg(db)));
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
    int rc = sqlite3_open_v2(db_name.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);

    if (rc != SQLITE_OK)
    {
        std::cerr << "Error opening database: " << sqlite3_errmsg(db) << std::endl;
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
    if (!db)
    {
        std::cerr << "Error: Database not connected." << std::endl;
        return false;
    }

    char* errorMessage = nullptr;

    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errorMessage);

    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL error: " << errorMessage << std::endl;
        sqlite3_free(errorMessage);
        return false;
    }

    return true;
}
