#include <filesystem>
#include <iostream>
#include <fstream> 
#include <string>
using namespace std;

string extractDirectory(const string& filePath) {
    size_t found = filePath.find_last_of("/\\"); // Finding the last directory separator
    if (found != string::npos) {
        return filePath.substr(0, found); // Extracting directory path
    }
    return ""; // If no directory separator is found
}

string checkDatabasePath(const string& path_to_database) {
    if (std::filesystem::exists(path_to_database + "/passwords.db")) {
        return path_to_database + "/passwords.db";
    } else {
        return ""; // Return empty string if file does not exist
    }
}

class PasswordManager {
    public:
        string path_to_database;
        PasswordManager() {
            try {
                ifstream db_handle("passwords.db");
                path_to_database = "passwords.db";
                if (db_handle.is_open()) {
                    cout << "Access granted";
                } else {
                    throw (path_to_database);
                }
            } 
            catch (string badPath) {
                path_to_database = check_database();
                ifstream db_handle(path_to_database);
            }
            
        }

        string check_database() {
            cout << "> 'passwords.db' not found in current path\n";
            for (int _ = 0; _ < 3; ++_) {
                cout << "> Please enter the absolute path to 'passwords.db' or press enter to create a new database: ";
                getline(cin, path_to_database);
                path_to_database = extractDirectory(path_to_database);
                if (!checkDatabasePath(path_to_database).empty()) {
                    return path_to_database;
                } else if (path_to_database == "") {
                    path_to_database = "passwords.db";
                    ifstream db_handle(path_to_database);
                    // default pass = WRITE sha256
                    //ofstream db_handle()
                    db_handle.close();
                    cout << "Default decryption key for database is 'password123'";
                    return path_to_database;
                } else {
                    cout << "\U0001F5D1 Database not found";
                }
            }
        }
};

int main() {
    PasswordManager password_manager;
    return 0;
}