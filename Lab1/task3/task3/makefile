CC = gcc
CFLAGS = -m32 -g -Wall
TARGET = menu_map_main

all: clean srcs linker

srcs:
	$(CC) $(CFLAGS) -c -o menu_map.o menu_map.c

linker:
	$(CC) $(CFLAGS) -o menu_map_main menu_map.o

debug: $(TARGET)
	gdb ./$(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f menu_map_main menu_map.o
