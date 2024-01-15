#include <iostream>
#include <sqlite3.h>
#include <vector>

// Define a structure to represent a person
struct Person {
    std::string firstname;
    std::string lastname;
    std::string phone_number;
};

// Callback function to process each row in the result set
int selectCallback(void* data, int columns, char** row, char**)
{
    std::vector<Person>* resultVector = reinterpret_cast<std::vector<Person>*>(data);

    // Deserialize the row and add it to the result vector
    Person person;
    person.firstname = row[0];
    person.lastname = row[1];
    person.phone_number = row[2];
    resultVector->push_back(person);

    return 0; // Continue processing rows
}

int main()
{
    // SQLite database connection
    sqlite3* db;
    int rc = sqlite3_open(":memory:", &db);  // Use ":memory:" for an in-memory database

    if (rc != SQLITE_OK)
    {
        std::cerr << "Error opening database: " << sqlite3_errmsg(db) << std::endl;
        return rc;
    }

    // Create the Person table
    const char* createTableSQL = "CREATE TABLE Person (firstname TEXT, lastname TEXT, phone_number TEXT);";
    rc = sqlite3_exec(db, createTableSQL, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK)
    {
        std::cerr << "Error creating table: " << sqlite3_errmsg(db) << std::endl;
        return rc;
    }

    // Insert some sample data
    const char* insertDataSQL = "INSERT INTO Person VALUES ('John', 'Doe', '123-456-7890');"
                                "INSERT INTO Person VALUES ('Jane', 'Smith', '987-654-3210');";

    rc = sqlite3_exec(db, insertDataSQL, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK)
    {
        std::cerr << "Error inserting data: " << sqlite3_errmsg(db) << std::endl;
        return rc;
    }

    // Select data from the Person table
    const char* selectDataSQL = "SELECT * FROM Person;";

    std::vector<Person> resultVector;
    rc = sqlite3_exec(db, selectDataSQL, selectCallback, &resultVector, nullptr);

    if (rc != SQLITE_OK)
    {
        std::cerr << "Error selecting data: " << sqlite3_errmsg(db) << std::endl;
        return rc;
    }

    // Display the result
    for (const auto& person : resultVector)
    {
        std::cout << "Firstname: " << person.firstname << ", Lastname: " << person.lastname
                  << ", Phone Number: " << person.phone_number << std::endl;
    }

    // Close the database connection
    sqlite3_close(db);

    return 0;
}
