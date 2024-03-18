CC=g++
CFLAGS=-std=gnu++17 -lssl -lcrypto -lncurses
TARGET=keychain

$(TARGET): keychain.cpp
	$(CC) $(CFLAGS) -o $(TARGET) keychain.cpp

install:
	@echo "Installing..."
	@chmod +x keychain
	@cp keychain /usr/local/bin/
	@echo "Done!"
