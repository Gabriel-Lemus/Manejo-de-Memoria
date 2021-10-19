CC = g++
CPPFLAGS = -Wall -Wextra -pthread
DEBUGFLAGS = -ggdb3
TARGET = main

all: ./src/$(TARGET).cpp
	$(CC) $(CPPFLAGS) ./src/*.cpp -o ./build/$(TARGET)

run:
	./build/$(TARGET)

clean:
	rm -f ./build/$(TARGET)

dbg:
	$(CC) $(CPPFLAGS) $(DEBUGFLAGS) ./src/*.cpp -o ./debug/$(TARGET)_debug

debugFile:
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=./debug/$(TARGET)-out.txt ./debug/$(TARGET)_debug

cleanDebug:
	rm -f ./debug/$(TARGET)-out.txt ./debug/$(TARGET)_debug
