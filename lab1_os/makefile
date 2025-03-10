# Определение компилятора и флагов
CC = gcc
CFLAGS = -Wall -Wextra -std=c11

# Имя исполняемого файла
TARGET = myshell

# Исходные файлы
SRCS = myshell.c utility.c  # Добавьте сюда все ваши файлы .c
OBJS = $(SRCS:.c=.o)  # Преобразование .c файлов в .o

# Основное правило для сборки
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Правило для компиляции .c файлов в .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Очистка скомпилированных файлов
.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET)

# Упрощение для запуска
run: $(TARGET)
	./$(TARGET)
