#include <cstring>
#include <filesystem>
#include <iostream>
#include <ostream>
#include <fstream>

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

    std::string sql = "SELECT * FROM " +  table;
    if (!condition.empty()) sql += " WHERE " + condition;
    sql += ";";

    sqlite3_stmt *statement = nullptr;

    sqlite3_prepare_v2(_db, sql.c_str(), -1, &statement, 0);        // prepare our query

    int rc;
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

    std::cerr << "Error: " << sqlite3_errmsg(_db) << std::endl;

    if (rc != SQLITE_DONE)
    {
        sqlite3_finalize(statement);                 // free our statement
        return false;
    }

    sqlite3_finalize(statement);                 // free our statement

    return true;
}

bool SqliteWrap::get_table_list(std::string& query, std::vector<std::string>& tablelist)
{
    //const char* query = "SELECT name FROM sqlite_master WHERE type='table';";
    sqlite3_stmt* statement = nullptr;

    sqlite3_prepare_v2(_db, query.c_str(), -1, &statement, 0);

    int rc;
    while ((rc = sqlite3_step(statement)) == SQLITE_ROW) {
        // Retrieve and store the table name
        const char* t = reinterpret_cast<const char*>(sqlite3_column_text(statement, 0));
        std::cout << "Table Name: " << t << std::endl;
        tablelist.push_back(t);
    }

    // Check for errors or no tables found
    if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
        std::cerr << "Error: " << sqlite3_errmsg(_db) << std::endl;
        sqlite3_finalize(statement);
        return false;
    }

    // Don't forget to finalize the statement
    sqlite3_finalize(statement);

    return true;
}

bool SqliteWrap::get_sqlite_version(std::string &version)
{
    const char* pingQuery = "SELECT sqlite_version();";
    sqlite3_stmt *pingStatement = nullptr;

    sqlite3_prepare_v2(_db, pingQuery, -1, &pingStatement, 0);

    int rc = sqlite3_step(pingStatement);

    if (rc != SQLITE_ROW) {
        std::cerr << "Error: " << sqlite3_errmsg(_db) << std::endl;
        sqlite3_finalize(pingStatement); // Don't forget to finalize the statement
        return false;
    }

    // Retrieve and print the SQLite version
    const char* sqliteVersion = reinterpret_cast<const char*>(sqlite3_column_text(pingStatement, 0));
    std::cout << "SQLite Version: " << sqliteVersion << std::endl;

    // Don't forget to finalize the statement
    sqlite3_finalize(pingStatement);

    return true;
}

bool SqliteWrap::get_database_name(std::string &db_name)
{
    sqlite3_stmt* statement;
    const char* sql = "PRAGMA database_list;";
    if (sqlite3_prepare_v2(_db, sql, -1, &statement, nullptr) == SQLITE_OK) {
        while (sqlite3_step(statement) == SQLITE_ROW) {
            const char* n = reinterpret_cast<const char*>(sqlite3_column_text(statement, 1));
            std::cout << "Database Name: " << n << std::endl;
            db_name = n;
        }
        sqlite3_finalize(statement);
    } else {
        std::cerr << "Error executing PRAGMA query." << std::endl;
    }

    //sqlite3_close(_db);

    return true;
}

bool SqliteWrap::get_database_schema(const std::string& pathfile)
{
    const char* query = "SELECT name, sql FROM sqlite_master WHERE type IN ('table', 'view');";
    sqlite3_stmt* statement = nullptr;

    // Open the file for writing
    std::ofstream outputFile(pathfile, std::ios::out);
    if (!outputFile.is_open()) {
        std::cerr << "Error: Unable to open the file for writing." << std::endl;
        return false;
    }

    // Execute the query
    sqlite3_prepare_v2(_db, query, -1, &statement, 0);

    int rc;
    while ((rc = sqlite3_step(statement)) == SQLITE_ROW) {
        // Retrieve and store the table name and SQL definition
        const char* tableName = reinterpret_cast<const char*>(sqlite3_column_text(statement, 0));
        const char* sqlDefinition = reinterpret_cast<const char*>(sqlite3_column_text(statement, 1));

        // Save to the file
        //outputFile << "Table Name: " << tableName << "\n";
        outputFile << "@@@sql@@@\n" << sqlDefinition << "\n";
    }

    // Check for errors or no tables found
    if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
        std::cerr << "Error: " << sqlite3_errmsg(_db) << std::endl;
        sqlite3_finalize(statement);
        outputFile.close();
        return false;
    }

    // Don't forget to finalize the statement
    sqlite3_finalize(statement);

    // Close the file
    outputFile.close();

    return true;
}

bool SqliteWrap::execute_sql_schema(const std::string& pathfile)
{
    std::ifstream file(pathfile);
    if (!file.is_open())
    {
        std::cerr << "Error: Unable to open file " << pathfile << std::endl;
        return false;
    }

    std::string sqlSeparator = "@@@sql@@@";
    std::string line;
    std::stringstream sqlBuffer;
    std::vector<std::string> sqlCommands;

    // Read the file line by line
    while (std::getline(file, line))
    {
        if (line == sqlSeparator)
        {
            // Store the accumulated SQL command
            sqlCommands.push_back(sqlBuffer.str());
            sqlBuffer.str("");
        }
        else
        {
            // Accumulate lines to form a complete SQL command
            sqlBuffer << line << '\n';
        }
    }

    // Check if there's a command in the buffer after reading the last line
    if (!sqlBuffer.str().empty())
    {
        // Store the last accumulated SQL command
        sqlCommands.push_back(sqlBuffer.str());
    }

    // Execute each SQL command
    for (const std::string& sql : sqlCommands)
    {
        if (!execute_sql(sql))
        {
            std::cerr << "Error executing SQL command: " << sql << std::endl;
            return false;
        }
    }

    return true;
}


























