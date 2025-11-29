// ===== wordlist.c =====
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "wordlist.h"

// Специальные слова
char *special_words[] = {"||", ">>", "&&", "|", ">", "<", ";", "(", ")", NULL};

// Инициализация списка
void init_list(WordList *list) {
    list->count = 0;
    list->capacity = 10;
    list->words = malloc(list->capacity * sizeof(char*));
    if (list->words == NULL) {
        printf("Ошибка памяти\n");
        exit(1);
    }
}

// Добавление слова в список
void add_word(WordList *list, const char *word) {
    if (strlen(word) == 0) return;  
	
    if (list->count >= list->capacity) {
        // Увеличиваем емкость
        list->capacity *= 2;
        list->words = realloc(list->words, list->capacity * sizeof(char*));
        if (list->words == NULL) {
            printf("Ошибка памяти\n");
            exit(1);
        }
    }
    
    // Копируем слово
    list->words[list->count] = malloc(strlen(word) + 1);
    if (list->words[list->count] == NULL) {
        printf("Ошибка памяти\n");
        exit(1);
    }
    strcpy(list->words[list->count], word);
    list->count++;
}

// Простая сортировка пузырьком
void sort_list(WordList *list) {
    int i, j;
    char *temp;
    
    for (i = 0; i < list->count - 1; i++) {
        for (j = 0; j < list->count - i - 1; j++) {
            if (strcmp(list->words[j], list->words[j + 1]) > 0) {
                // Меняем местами
                temp = list->words[j];
                list->words[j] = list->words[j + 1];
                list->words[j + 1] = temp;
            }
        }
    }
}

// Очистка списка
void clear_list(WordList *list) {
    int i;
    for (i = 0; i < list->count; i++) {
        free(list->words[i]);
    }
    list->count = 0;
}

// Полное освобождение списка
void free_list(WordList *list) {
    clear_list(list);
    free(list->words);
}

// Проверка, является ли символ частью простого слова
int is_simple_char(char c) {
    return isalnum(c) || c == '$' || c == ' ' || c == '/' || c == ',';
}

// Проверка, является ли строка специальным словом
int is_special_word(const char *str, int len) {
    int i;
    for (i = 0; special_words[i] != NULL; i++) {
        if (strncmp(str, special_words[i], len) == 0 && strlen(special_words[i]) == len) {
            return 1;
        }
    }
    return 0;
}

// Получение максимальной длины специального слова
int get_max_special_len(const char *str) {
    int i, max_len = 0;
    for (i = 0; special_words[i] != NULL; i++) {
        int len = strlen(special_words[i]);
        if (strncmp(str, special_words[i], len) == 0 && len > max_len) {
            max_len = len;
        }
    }
    return max_len;
}

// Обработка строки и разбиение на слова
void process_line(const char *line, WordList *list) {
    int i = 0;
    int len = strlen(line);
    char buffer[1000];
    int buf_index = 0;
  
    // Пропускаем символ новой строки в конце
    if (len > 0 && line[len-1] == '\n') {
        len--;
    }

    while (i < len) {
        // Пропускаем пробельные символы
        if (isspace(line[i])) {
            // Если в буфере есть слово, добавляем его
            if (buf_index > 0) {
                buffer[buf_index] = '\0';
                add_word(list, buffer);
                buf_index = 0;
            }
            i++;
            continue;
        }
        
        // Проверяем специальные слова
        int special_len = get_max_special_len(&line[i]);
        if (special_len > 0) {
            // Если в буфере есть накопленное слово, добавляем его
            if (buf_index > 0) {
                buffer[buf_index] = '\0';
                add_word(list, buffer);
                buf_index = 0;
            }
            
            // Добавляем специальное слово
            strncpy(buffer, &line[i], special_len);
            buffer[special_len] = '\0';
            add_word(list, buffer);
            i += special_len;
            continue;
        }
        
        // Обычный символ для простого слова
        if (is_simple_char(line[i])) {
            buffer[buf_index++] = line[i];
            i++;
        } else {
            // Если встретили не-специальный и не-простой символ
            if (buf_index > 0) {
                buffer[buf_index] = '\0';
                add_word(list, buffer);
                buf_index = 0;
            }
            
            // Добавляем одиночный символ как слово
            buffer[0] = line[i];
            buffer[1] = '\0';
            add_word(list, buffer);
            i++;
        }
    }
    
    // Добавляем последнее слово, если есть
    if (buf_index > 0) {
        buffer[buf_index] = '\0';
        add_word(list, buffer);
    }
}
