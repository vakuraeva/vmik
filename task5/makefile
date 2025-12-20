# Компилятор
CC = gcc
CFLAGS = -Wall

# Имена файлов
TARGET = shell
SRCS = main.c functions.c
OBJS = $(SRCS:.c=.o)

# Правило по умолчанию
all: $(TARGET)

# Сборка исполняемого файла
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Компиляция .c файлов в .o
%.o: %.c functions.h
	$(CC) $(CFLAGS) -c $< -o $@

# Очистка
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
