CC=g++
CFLAGS=-std=gnu++17 -lssl -lcrypto -lncurses
TARGET=quickeys

$(TARGET): quickeys.cpp
	$(CC) $(CFLAGS) -o $(TARGET) quickeys.cpp

install:
	@echo "Installing QuicKeys..."
	@chmod +x quickeys
	@cp quickeys /usr/local/bin/
	@echo "Done!"

uninstall:
	@echo "Uninstalling..."
	@rm /usr/local/bin/quickeys
	@echo "Done!"
