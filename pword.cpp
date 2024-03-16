#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>
#include <cryptopp/files.h>
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/osrng.h>
#include <cryptopp/filters.h>
#include <cryptopp/cryptlib.h>
#include <cryptopp/rijndael.h>
#include <unistd.h>
#include <vector>
using namespace std;
using namespace CryptoPP;

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

string calculateSHA256(const string& input) {
	SHA256 hash;
	string digest;

	StringSource(input, true, new HashFilter(hash, new HexEncoder(new StringSink(digest))));

	return digest;
}

string aesDecrypt(const string& ciphertext, const string& key) {
	SecByteBlock iv(AES::BLOCKSIZE);
	AutoSeededRandomPool prng;
	prng.GenerateBlock(iv, iv.size());
	
	string decryptedtext;

	CBC_Mode< AES >::Decryption d(
			(CryptoPP::byte*)key.c_str(), key.length(), iv);
	StringSource(ciphertext, true, 
		new StreamTransformationFilter(d, new StringSink(decryptedtext)));
	return decryptedtext;
}

string unpad(const string& padded_text, size_t block_size) {
	size_t pad_length = static_cast<size_t>(padded_text[padded_text.length() - 1]);
	if (pad_length >= 1 && pad_length <= block_size) {
		return padded_text.substr(0, padded_text.length() - pad_length);
	}
	return padded_text; // Return original text if padding is invalid
}

int countRecords(const string& content, char delimiter) {
	stringstream ss(content);
	string item;
	int count = 0;

	while (getline(ss, item, delimiter)) {
		count++;
	}
	return count;
}

string padDatabaseKey(const string& password) {
	if (password.length() % 16 == 0) {
		return password;
	} else {
		string padding(16 - (password.length() % 16), '0');
		return password + padding;
	}
}

class PasswordManager {
    public:
        string path_to_database;
	string db_key_hash;
        string ciphertext;
	string decryption_key;
	string password_hash;
	string content;
	int records_count;
	PasswordManager() {
            try {
                ifstream db_handle("passwords.db");
                path_to_database = "passwords.db";
                if (db_handle.is_open()) {
		    db_handle.close();
                } else {
                    throw (path_to_database);
                }
            } 
            catch (string badPath) {
                path_to_database = check_database();
                ifstream db_handle(path_to_database);
		db_handle.close();
            }
	    char key_buffer[65];
	    ifstream db_handle(path_to_database, ios::binary);
	    db_handle.read(key_buffer, 64);
	    key_buffer[64] = '\0';
	    db_key_hash = key_buffer;
	    stringstream file_buffer;
	    file_buffer << db_handle.rdbuf();
	    ciphertext = file_buffer.str();

	    for (int _ = 0; _ < 3; ++_) {
	    	decryption_key = getpass("Decryption key: ");
		decryption_key = padDatabaseKey(decryption_key);
		// Calculate SAH-256 sum for given passwd
		password_hash = calculateSHA256(decryption_key);

		if (db_key_hash == password_hash) {
			db_handle.close();
			decrypt_db();
			break;
		}
		else {
			cout << "Invalid password.\n";
	    	}
	    }
            
        }

	void decrypt_db() {
		// Decrypting with AES-CBC
		if (ciphertext.length() != 0) {
			string aes_instance = aesDecrypt(ciphertext, password_hash); 
			string unpadded_text = unpad(aes_instance, AES::BLOCKSIZE);
			content = string(unpadded_text.begin(), unpadded_text.end());
			records_count = countRecords(content, '|');
			cout << "\u2714 " << records_count << " records found" << endl;
		} else {
			content = "";
			records_count = 0;
			cout << "Database has no records.";
		}
		display_options();
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
                    ofstream db_handle(path_to_database);
                    string default_pass = calculateSHA256(padDatabaseKey("password123"));
                    db_handle << default_pass;
                    db_handle.close();
                    cout << "Default decryption key for database is 'password123'";
                    return path_to_database;
                } else {
                    cout << "Database not found\n";
                }
            }
	    return "not_found";
        }

	void display_options() {
		while(1) {
			cout << "\n[1] Show credentials\n";
			cout << "[2] Add credentials\n";
			cout << "[3] Edit credentials\n";
			cout << "[4] Delete credentials\n";
			cout << "[5] Change database password\n";
			cout << "[6] Generate password\n";
			cout << "[7] Backup database\n";
			cout << "[8] Erase database\n";
			cout << "[9] Exit\n";
			int option;
			cout << "> ";
			cin >> option;

			if (option==1) { show_credentials(); }
			else if (option==2) { add_credentials(); }
			//else if (option==3) { edit_credentials(); }
			//else if (option==4) { delete_credentials(); }
			//else if (option==5) { change_db_password(); }
			//else if (option==6) { generate_password(); }
			//else if (option==7) { backup_database(); }
			//else if (option==8) { erase_database(); }
			else if (option==9) { cout << "Goodby"; break;}
			else { cout << "Invalid option!"; }
		}
	}

	void show_credentials() {
		if (records_count == 0) {
			cout << "Database has no records";
		}

		vector<vector<string>> table;
		istringstream iss(content);
		string line;

		while(getline(iss, line, '|')) {
			vector<string> row;
			istringstream row_ss(line);
			string item;

			while (std::getline(row_ss, item, '-')) {
				row.push_back(item);
			}

			table.push_back(row);
		}

		cout << "id\tusername/email\tpassword\tplatform" << endl;
		for (const auto& row : table) {
			for (const auto& item : row) {
				cout << item << "\t";
			}
			cout << endl;
		}
	}

	void add_credentials() {
		while (true) {
			vector<string> new_creds;
			string username_or_email;
			string password1;
			string password2;
			string platform;

			cout << "username/email: ";
			cin >> username_or_email;
			cout << "password: ";
			cin >> password1;
			cout << "retype password: ";
			cin >> password2;
			if (password1 != password2) {
				cout << "passwords do not match";
				continue;
			}
			cout << "platform: ";
			cin >> platform;
			if (records_count == 0) {
				new_creds.push_back("1");
				new_creds.push_back(username_or_email);
				new_creds.push_back(password1);
				new_creds.push_back(platform);

				for (size_t i = 0; i < new_creds.size(); ++i) {
					if (i > 0) {
						content += "-";
					}
					content += new_creds[i];
				}
			} else {
			
			}

			records_count += 1;
			
			cout << "Records added";
			break;
		}
	}
};

int main() {
    PasswordManager password_manager;
    return 0;
}
