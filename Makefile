CC=g++
CFLAGS=-std=gnu++17 -lssl -lcrypto -lncurses
TARGET=keymgr

$(TARGET): keymgr.cpp
	$(CC) $(CFLAGS) -o $(TARGET) keymgr.cpp

install:
	@echo "Installing..."
	@chmod +x keymgr
	@cp keymgr /usr/local/bin/
	@echo "Done!"

uninstall:
	@echo "Uninstalling..."
	@rm /usr/local/bin/keymgr
	@echo "Done!"
