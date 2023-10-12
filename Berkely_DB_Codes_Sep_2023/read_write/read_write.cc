#include <iostream>
#include <string.h>
#include <db_cxx.h> // Berkeley DB C++ API

int main() {
    // Define a Berkeley DB environment and database
    DbEnv env(0);
    env.set_cachesize(0, 64 * 1024, 0); // Set cache size (optional)
    env.open("./db", DB_CREATE | DB_INIT_MPOOL, 0); // Open the environment in the current directory

    Db db(&env, 0);
    db.open(nullptr, "mydb.db", nullptr, DB_BTREE, DB_CREATE, 0); // Open the database

    // Define key and data pairs
    const char* keys[] = {"City", "State", "Country"};
    const char* values[] = {"New York City", "New York", "USA"};

    // Add multiple key-value pairs to the database
    for (int i = 0; i < 3; ++i) {
        Dbt key_dbt(const_cast<char*>(keys[i]), strlen(keys[i]) + 1); // Key
        Dbt data_dbt(const_cast<char*>(values[i]), strlen(values[i]) + 1); // Data

        if (db.put(nullptr, &key_dbt, &data_dbt, 0) != 0) {
            std::cerr << "Error adding data for key: " << keys[i] << std::endl;
        }
    }

    // Retrieve and display all key-value pairs from the database
    Dbt current_key, current_data;
    Dbc* cursor;
    db.cursor(nullptr, &cursor, 0);

    while (cursor->get(&current_key, &current_data, DB_NEXT) == 0) {
        std::cout << "Key: " << static_cast<char*>(current_key.get_data()) << ", Value: " << static_cast<char*>(current_data.get_data()) << std::endl;
    }

    cursor->close();

    // Close the database and environment
    db.close(0);
    env.close(0);

    return 0;
}
