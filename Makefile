CC=g++
CFLAGS=-std=gnu++17 -lssl -lcrypto -lncurses
TARGET=skey

$(TARGET): skey.cpp
	$(CC) $(CFLAGS) -o $(TARGET) skey.cpp

install:
	@echo "Installing..."
	@chmod +x skey
	@cp skey /usr/local/bin/
	@echo "Done!"

uninstall:
	@echo "Uninstalling..."
	@rm /usr/local/bin/skey
	@echo "Done!"
