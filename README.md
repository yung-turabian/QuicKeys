# Keychain

Keychain is a terminal app for secure and personal password management. Built with C++.

<img src="logo.png" alt="logo" width="200"/>

## Installation

Below I've provided build instructions as I've yet to work with package management installers. Looking into NixOS currently.

# Build
```bash
$ git clone https://github.com/yung-turabian/keychain.git
$ sudo make install -C keychain
$ rm -rf keychain
```

## Usage

```bash
$ keychain

```

```bash
$ Please Enter Decryption key:  
```

```bash
[1] Show credentials
[2] Add credentials
[3] Edit credentials
[4] Delete credentials
[5] Change database password
[6] Backup database
[7] Erase database
[8] Exit

keychain~$ 1
```

## Contributing

Pull requests are welcome. For major changes, please open an issue first
to discuss what you would like to change.

Please make sure to update tests as appropriate.

## License

[MIT](https://choosealicense.com/licenses/mit/)

## Attribution

[House key icons created by Freepik - Flaticon](https://www.flaticon.com/free-icons/house-key "house key icons")
