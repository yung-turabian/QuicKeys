CC=g++
CFLAGS=-std=gnu++17 -lssl -lcrypto -lncurses
TARGET=keychain

$(TARGET): keychain.cpp
	$(CC) $(CFLAGS) -o $(TARGET) keychain.cpp

