#include <cstddef>
#include <cstring>
#include <exception>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>

#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

#include <cstdlib>
#include <unistd.h>
#include <vector>

#include <ncurses.h>
using namespace std;

#define AES_BLOCK_SIZE 16

vector<string> split(const string &s, char delimiter) {
  vector<string> tokens;
  stringstream ss(s);
  string token;
  while (getline(ss, token, delimiter)) {
    tokens.push_back(token);
  }
  return tokens;
}

string join(const vector<string> &tokens, char delimiter) {
  string result;
  for (size_t i = 0; i < tokens.size(); ++i) {
    if (i > 0)
      result += delimiter;
    result += tokens[i];
  }
  return result;
}

string extractDirectory(const string &filePath) {
  size_t found =
      filePath.find_last_of("/\\"); // Finding the last directory separator
  if (found != string::npos) {
    return filePath.substr(0, found); // Extracting directory path
  }
  return ""; // If no directory separator is found
}

string checkDatabasePath(const string &pathToDatabase) {
  if (filesystem::exists(pathToDatabase + "/.passwords")) {
    return pathToDatabase + "/.passwords";
  } else {
    return ""; // Return empty string if file does not exist
  }
}

string calculateSHA256(const string &input) {
  unsigned char hash[SHA256_DIGEST_LENGTH];
  string hashStr;

  // Initialize OpenSSL's SHA-256 algorithm
  EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
  EVP_DigestInit_ex(mdctx, EVP_sha256(), nullptr);
  EVP_DigestUpdate(mdctx, input.c_str(), input.length());
  EVP_DigestFinal_ex(mdctx, hash, nullptr);
  EVP_MD_CTX_free(mdctx);

  // Convert the hash to a hex string
  stringstream ss;
  for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
    ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
  }
  hashStr = ss.str();

  return hashStr;
}

int countRecords(const string &content, char delimiter) {
  stringstream ss(content);
  string item;
  int count = 0;

  while (getline(ss, item, delimiter)) {
    count++;
  }
  return count;
}

string padDatabaseKey(const string &password) {
  if (password.length() % 16 == 0) {
    return password;
  } else {
    string padding(16 - (password.length() % 16), '0');
    return password + padding;
  }
}

class PasswordManager {
private:
  // Points to home directory on Windows or UNIX
  string homeDir;
  string pathToDatabase;
  string databaseKeyHash;
  string ciphertext;
  string decryptionKey;
  string passwordHash;
  string content;
  int recordsCount;

  void decryptDatabase() {
    // Decrypting with AES-CBC
    if (!ciphertext.empty()) 
		{
      AES_KEY aes_key;
      AES_set_decrypt_key(
          reinterpret_cast<const unsigned char *>(decryptionKey.c_str()), 128,
          &aes_key);

      string iv = decryptionKey.substr(0, AES_BLOCK_SIZE);
      vector<unsigned char> ivVec(iv.begin(), iv.end());

      vector<unsigned char> decrypted;
      decrypted.resize(ciphertext.size());

      AES_cbc_encrypt(
          reinterpret_cast<const unsigned char *>(ciphertext.c_str()),
          decrypted.data(), ciphertext.size(), &aes_key, ivVec.data(),
          AES_DECRYPT);

      // Remove padding
      int pad_len = decrypted.back();
      decrypted.resize(decrypted.size() - pad_len);

      content = string(decrypted.begin(), decrypted.end());

      recordsCount = countRecords(content, '|');
      cout << "\u2714 " << recordsCount << " records found" << endl;
    } 
		else 
		{
      content = "";
      recordsCount = 0;
      cout << "Database has no records.";
    }

    displayOptions();
  }

  void saveDatabase() {
    ofstream db_handle(pathToDatabase, ios::binary);
    vector<unsigned char> ciphertext;
    if (recordsCount != 0) {
      AES_KEY aes_key;
      AES_set_encrypt_key(
          reinterpret_cast<const unsigned char *>(decryptionKey.c_str()), 128,
          &aes_key);

      string iv = decryptionKey.substr(0, 16); // First 16 bytes as IV
      std::vector<unsigned char> ivVec(iv.begin(), iv.end());

      // Pad the content
      int pad_len = AES_BLOCK_SIZE - (content.size() % AES_BLOCK_SIZE);
      content.append(pad_len, static_cast<char>(pad_len));

      ciphertext.resize(content.size());

      AES_cbc_encrypt(reinterpret_cast<const unsigned char *>(content.c_str()),
                      ciphertext.data(), content.size(), &aes_key, ivVec.data(),
                      AES_ENCRYPT);
    }

    db_handle.write(databaseKeyHash.c_str(), databaseKeyHash.size());
    db_handle.write(reinterpret_cast<const char *>(ciphertext.data()),
                    ciphertext.size());
    db_handle.close();
  }

  string checkDatabase() {
    cout << "$ '/.passwords' not found in User files\n";
    for (int _ = 0; _ < 3; ++_) {
      cout << "$ Please enter the absolute path to '/.passwords' or press "
              "enter to create a new database: ";
      getline(cin, pathToDatabase);
      pathToDatabase = extractDirectory(pathToDatabase);
      if (!checkDatabasePath(pathToDatabase).empty()) {
        return pathToDatabase;
      } else if (pathToDatabase == "") {
        pathToDatabase = homeDir + "/.passwords";
        ofstream db_handle(pathToDatabase);
        string default_pass = calculateSHA256(padDatabaseKey("password123"));
        db_handle << default_pass;
        db_handle.close();
        cout << "Default decryption key for database is 'password123'\n";
        return pathToDatabase;
      } else {
        cout << "Database not found\n";
      }
    }
    return "not_found";
  }

  void displayOptions() 
	{
		bool shouldExit = false;
		int inputError = 0;

    while (!shouldExit) {
      cout << "\n[1] Show credentials" << endl;
      cout << "[2] Add credentials" << endl;
      cout << "[3] Edit credentials" << endl;
      cout << "[4] Delete credentials" << endl;
      cout << "[5] Change database password" << endl;
      cout << "[6] Backup database" << endl;
      cout << "[7] Erase database" << endl;
      cout << "[8] Exit" << endl;

      int option;
			char input[100];
		  
      fprintf(stdout, "\n> ");
			if(inputError != 0) {
				switch(inputError) {
						case 1:
							fprintf(stderr, "Invalid input. Please enter an integer.\n");
							inputError = 0;
							break;
						default:
							break;
				}
			}

					
			if(fgets(input, sizeof(input), stdin) != NULL) {
		      fprintf(stdout, "\033[2J");
				  fprintf(stdout, "\033[H");

					if(sscanf(input, "%d", &option) == 1) {
							switch(option)
							{
									case 1:
										showCredentials();
										break;
									case 2:
										addCredentials();
										break;
									case 3:
										editCredentials();
										break;
									case 4:
										deleteCredentials();
										break;
									case 5:
										changeDatabasePassword();
										break;
									case 6:
										backupDatabase();
										break;
									case 7:
										eraseDatabase();
										break;
									case 8:
										fprintf(stdout, "Goodbye!\n");
										shouldExit = true;
										break;
									default:
										break;
							}
					} else {
							inputError = 1;
					}
			} else {
						fprintf(stderr, "Error reading input.\n");
						shouldExit = true;
			}

    }
  }

  void showCredentials() {
    if (recordsCount == 0) {
      cout << "Database has no records";
    }

    vector<vector<string> > table;
    istringstream iss(content);
    string line;

    while (getline(iss, line, '|')) {
      vector<string> row;
      istringstream row_ss(line);
      string item;

      while (std::getline(row_ss, item, '-')) {
        row.push_back(item);
      }

      table.push_back(row);
    }

    fprintf(stdout, "id | username/email | password | platform");
    for (const auto &row : table) {
      for (const auto &item : row) {
        cout << item << "\t";
      }
      cout << endl;
    }
  }

  void addCredentials() {
    while (true) {
      vector<string> new_creds;
      string username_or_email;
      string password1;
      string password2;
      string platform;

      cout << "Username/Email: ";
      cin >> username_or_email;
      cout << "Password: ";
      cin >> password1;
      cout << "Retype password: ";
      cin >> password2;
      if (password1 != password2) {
        cout << "Passwords do not match";
        continue;
      }
      cout << "Platform: ";
      cin >> platform;

      std::stringstream ss;
      if (recordsCount == 0) {
        ss << "1-" << username_or_email << "-" << password1 << "-" << platform;
        content = ss.str();
      } else {
        int record_id = std::stoi(
            content.substr(content.rfind("|") + 1, content.find("-")));
        ss << record_id + 1 << "-" << username_or_email << "-" << password1
           << "-" << platform;
        content += "|" + ss.str();
      }
      recordsCount++;
      saveDatabase();
      std::cout << "Record added" << std::endl;
      break;
    }
  }

  int findRecord(int record_id) {
    vector<string> records = split(content, '|');
    for (int i = 0; i < records.size(); ++i) {
      vector<string> fields = split(records[i], '-');
      if (stoi(fields[0]) == record_id) {
        return i;
      }
    }
    return -1;
  }

  void editCredentials() {
    if (recordsCount != 0) {
      showCredentials();
      int recordIdToEdit = -1;
      for (int i = 4; i > 0; i--) {
        try {
          cout << "Record id to edit: ";
          cin >> recordIdToEdit;
          if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            throw runtime_error("Invalid record id");
          }
        } catch (const exception &e) {
          cerr << e.what();
          continue;
        }

        int recordIndex = findRecord(recordIdToEdit);
        if (recordIndex != -1) {
          cout << "[1] Change username/email" << endl;
          cout << "[2] Change password" << endl;
          int option = -1;
          try {
            cout << "$ ";
            cin >> option;
            if (cin.fail()) {
              cin.clear();
              cin.ignore(numeric_limits<streamsize>::max(), '\n');
              throw runtime_error("Invalid option");
            }
          } catch (const exception &e) {
            cerr << e.what() << endl;
            continue;
          }

          vector<string> records = split(content, '|');
          vector<string> recordFields = split(records[recordIndex], '-');

          if (option == 1) {
            string new_username_or_email;
            cout << "New username/email: ";
            cin >> new_username_or_email;
            recordFields[1] = new_username_or_email;
          } else if (option == 2) {
            string new_password;
            cout << "New password: ";
            cin >> new_password;
            recordFields[2] = new_password;
          } else {
            cerr << "Invalid option" << endl;
            continue;
          }
          records[recordIndex] = join(recordFields, '-');
          content = join(records, '|');
          saveDatabase();
          cout << "Record modified" << endl;
          break;
        } else {
          cerr << "Record id not found" << endl;
        }
      }
    } else {
      cerr << "No records to modify" << endl;
    }
  }

  void deleteCredentials() {
    if (recordsCount != 0) {
      showCredentials();
      int recordIdToDelete = -1;
      for (int i = 4; i > 0; i--) {
        try {
          cout << "Record id to delete: ";
          cin >> recordIdToDelete;
          if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            throw runtime_error("Invalid record id");
          }
        } catch (const exception &e) {
          cerr << e.what() << endl;
          continue;
        }

        int recordIndex = findRecord(recordIdToDelete);
        if (recordIndex != -1) {
          vector<string> new_records = split(content, '|');
          new_records.erase(new_records.begin() + recordIndex);
          recordsCount--;
          content = join(new_records, '|');
          saveDatabase();
          cout << "Record deleted" << endl;
          break;
        } else {
          cerr << "Record id not found" << endl;
        }
      }
    } else {
      cerr << "No records to delete" << endl;
    }
  }

  void changeDatabasePassword() {
    while (true) {
      string current_password;
      cout << "Current decryption key: ";
      cin >> current_password;
      current_password = padDatabaseKey(current_password);
      string current_password_hash = calculateSHA256(current_password);
      if (current_password_hash != databaseKeyHash) {
        cerr << "Current password is incorrect" << endl;
        continue;
      }
      string new_password;
      cout << "New decryption key: ";
      cin >> new_password;
      if (new_password.length() < 10) {
        cerr << "Password must be at least 10 characters" << endl;
        continue;
      }
      string confirm_new_password;
      cout << "Confirm new decryption key: ";
      cin >> confirm_new_password;
      if (new_password != confirm_new_password) {
        cerr << "Decryption keys do not match" << endl;
        continue;
      }
      new_password = padDatabaseKey(new_password);
      string new_password_hash = calculateSHA256(new_password);
      decryptionKey = new_password;
      databaseKeyHash = new_password_hash;
      saveDatabase();
      cout << "Decryption key updated successfully" << endl;
      break;
    }
  }

  void backupDatabase() {
    if (recordsCount != 0) {
      for (int i = 0; i < 3; ++i) {
        string _decryptionKey = getpass("Database decryption key: ");

        string _decryptionKeyHash =
            calculateSHA256(padDatabaseKey(_decryptionKey));
        if (databaseKeyHash == _decryptionKeyHash) {
          ifstream source(pathToDatabase, ios::binary);
          ofstream dest("./passwords.db.bak", ios::binary);
          dest << source.rdbuf();
          cout << "Database backup saved in '" << filesystem::current_path()
               << "/passwords.db.bak'\n";
          break;
        } else {
          cout << "Incorrect database decryption key\n";
        }
      }
    } else {
      cout << "No records to backup\n";
    }
  }

  void eraseDatabase() {
    if (recordsCount != 0) {
      for (int i = 0; i < 3; ++i) {
        string _decryptionKey = getpass("Database decryption key: ");

        string _decryptionKeyHash =
            calculateSHA256(padDatabaseKey(_decryptionKey));
        if (databaseKeyHash == _decryptionKeyHash) {
          content = "";
          recordsCount = 0;
          saveDatabase();
          cout << "Database erased\n";
          break;
        } else {
          cout << "Incorrect database decryption key\n";
        }
      }
    } else {
      cout << "No records to erase\n";
    }
  }

public:
  PasswordManager() {
    try {
#ifdef _WIN32
      homeDir = getenv("USERPROFILE");
#else
      homeDir = getenv("HOME");
#endif
      if (homeDir.empty()) {
        throw(homeDir);
      }
    } catch (string baseDirPath) {
      cerr << "Unable to determine the home directory of the user.\n" << endl;
    }

    try {
      ifstream db_handle(homeDir + "/.passwords");
      pathToDatabase = homeDir + "/.passwords";
      if (db_handle.is_open()) {
        db_handle.close();
      } else {
        throw(pathToDatabase);
      }
    } catch (string badPath) {
      pathToDatabase = checkDatabase();
      ifstream db_handle(pathToDatabase);
      db_handle.close();
    }
    char key_buffer[65];
    ifstream db_handle(pathToDatabase, ios::binary);
    db_handle.read(key_buffer, 64);
    key_buffer[64] = '\0';
    databaseKeyHash = key_buffer;
    stringstream file_buffer;
    file_buffer << db_handle.rdbuf();
    ciphertext = file_buffer.str();

    for (int i = 4; i > 0; i--) {
      decryptionKey = getpass("Please Enter Decryption key: ");
      decryptionKey = padDatabaseKey(decryptionKey);
      // Calculate SAH-256 sum for given passwd
      passwordHash = calculateSHA256(decryptionKey);

      if (databaseKeyHash == passwordHash) {
        db_handle.close();
        decryptDatabase();
        break;
      } 
			else 
			{
        cout << "\nInvalid password [" << i - 1 << " attempts remaining]\n"
             << endl;
      }
    }
  }
};

int main() {
  PasswordManager password_manager;
  return 0;
}
