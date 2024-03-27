# QuicKeys

is a terminal app for secure and personal password management. Built with C++.

<p align="center">
  <img src="logo.png" alt="logo" width="200"/>
</p>

## Installation

Below I've provided build instructions as I've yet to work with package management installers. Looking into NixOS currently.

# Build
```bash
git clone https://github.com/yung-turabian/QuicKeys.git  &&
sudo make install -C QuicKeys &&
rm -rf QuicKeys
```

## Usage

The way QuicKeys works is that on UNIX machines a .passwords file will be created in the home directory where the encrypted passwords will be stored. On Windows the same will occur but instead in the current user's directory. 

```bash
$ quickeys

Please Enter Decryption key:

[1] Show credentials
[2] Add credentials
[3] Edit credentials
[4] Delete credentials
[5] Change database password
[6] Backup database
[7] Erase database
[8] Exit

QuicKeys~$ 1

id | username/email | password | platform
1	dummy@gmail.com	password123 Google	
```

## Contributing

Pull requests are welcome. For major changes, please open an issue first
to discuss what you would like to change.

Please make sure to update tests as appropriate.

## License

[GNU](https://choosealicense.com/licenses/gpl-3.0/)

## Attribution

[House key icons created by Freepik - Flaticon](https://www.flaticon.com/free-icons/house-key "house key icons")
