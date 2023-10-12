#include <iostream>
#include <db_cxx.h> // Berkeley DB C++ API

int main() {
    // Define a Berkeley DB environment and database
    DbEnv env(0);
    env.set_cachesize(0, 64 * 1024, 0); // Set cache size (optional)
    env.open("./db", DB_CREATE | DB_INIT_MPOOL, 0); // Open the environment in the current directory

    Db db(&env, 0);
    db.open(nullptr, "mydb.db", nullptr, DB_BTREE, DB_CREATE, 0); // Open the database

    // Define an array of integers
    int array[] = {1, 2, 3, 4, 5};

    // Store each array element in the database with a unique key
    for (int i = 0; i < sizeof(array) / sizeof(array[0]); ++i) {
        // Convert the array index to a string key
        std::string key = "element_" + std::to_string(i);

        // Convert the array element to a string value (for simplicity)
        std::string value = std::to_string(array[i]);

        // Create Dbt objects for key and data
        Dbt key_dbt(const_cast<char*>(key.c_str()), key.length() + 1);
        Dbt data_dbt(const_cast<char*>(value.c_str()), value.length() + 1);

        // Store the key-value pair in the database
        if (db.put(nullptr, &key_dbt, &data_dbt, 0) != 0) {
            std::cerr << "Error adding data for key: " << key << std::endl;
        }
    }

    // Calculate the summation by retrieving and adding all values from the database
    int summation = 0;

    for (int i = 0; i < sizeof(array) / sizeof(array[0]); ++i) {
        std::string key = "element_" + std::to_string(i);
        Dbt key_dbt(const_cast<char*>(key.c_str()), key.length() + 1);
        Dbt data_dbt;

        if (db.get(nullptr, &key_dbt, &data_dbt, 0) == 0) {
            int element_value = std::stoi(static_cast<char*>(data_dbt.get_data()));
            summation += element_value;
        }
    }

    std::cout << "Summation of array elements: " << summation << std::endl;

    // Close the database and environment
    db.close(0);
    env.close(0);

    return 0;
}
