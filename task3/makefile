# ===== Makefile =====
CC = gcc
CFLAGS = -Wall -std=c99
OBJS = main.o wordlist.o
TARGET = program

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

main.o: main.c wordlist.h
	$(CC) $(CFLAGS) -c main.c

wordlist.o: wordlist.c wordlist.h
	$(CC) $(CFLAGS) -c wordlist.c

clean:
	rm -f $(OBJS) $(TARGET)
