// ===== wordlist.c =====
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "wordlist.h"

#define BLOCK_SIZE 100
#define INITIAL_BUF_SIZE 10

// Специальные слова
char *special_words[] = {"||", ">>", "&&", "|", ">", "<", ";", "(", ")", NULL};

// Глобальные переменные для чтения блоков
static char block[BLOCK_SIZE + 1];
static int block_pos = 0;
static int block_len = 0;
static int eof_reached = 0;

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
        list->capacity *= 2;
        list->words = realloc(list->words, list->capacity * sizeof(char*));
        if (list->words == NULL) {
            printf("Ошибка памяти\n");
            exit(1);
        }
    }
    
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

// получение следующего символа 
char getsym(FILE *stream) {
    if (block_pos >= block_len) {
        // Пытаемся прочитать блок
        if (fscanf(stream, "%100c", block) == 1) {
            block_len = BLOCK_SIZE;
            block_pos = 0;
        } else {
            return EOF;
        }
    }
    return block[block_pos++];
}
// Проверка, является ли символ частью простого слова
int is_simple_char(char c) {
    return isalnum(c) || c == '$' || c == '\'' || c == '/' || c == ',';
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

// Обработка ввода с использованием блоков
void process_input(WordList *list) {
    char current_char;
    char *word_buf = NULL;
    int buf_size = 0;
    int buf_pos = 0;
    
    // Инициализация буфера для слова
    buf_size = INITIAL_BUF_SIZE;
    word_buf = malloc(buf_size);
    if (word_buf == NULL) {
        printf("Ошибка памяти\n");
        return;
    }
    
    // Сбрасываем состояние чтения блоков
    block_pos = 0;
    block_len = 0;
    eof_reached = 0;
    
    while (!eof_reached) {
        // Получаем следующий символ
        current_char = getsym(stdin);
        
        if (current_char == EOF) {
            // Конец файла - обрабатываем последнее слово если есть
            if (buf_pos > 0) {
                word_buf[buf_pos] = '\0';
                add_word(list, word_buf);
            }
            break;
        }
        
        // Обработка конца строки
        if (current_char == '\n') {
            if (buf_pos > 0) {
                word_buf[buf_pos] = '\0';
                add_word(list, word_buf);
                buf_pos = 0;
            }
            
            // Выводим результаты для текущей строки
            if (list->count > 0) {
                printf("%d\n", list->count);
                
                // Вывод в исходном порядке
                int i;
                for (i = 0; i < list->count; i++) {
                    printf("%s\n", list->words[i]);
                }
                
                // Сортировка и вывод в лексикографическом порядке
                sort_list(list);
                for (i = 0; i < list->count; i++) {
                    printf("%s\n", list->words[i]);
                }
                
                clear_list(list);
            }
            continue;
        }
        
        // Пропускаем пробельные символы (кроме перевода строки)
        if (isspace(current_char)) {
            if (buf_pos > 0) {
                word_buf[buf_pos] = '\0';
                add_word(list, word_buf);
                buf_pos = 0;
            }
            continue;
        }
        
        // Проверяем специальные слова
        // Создаем временную строку для проверки специальных слов
        char temp_str[4] = {current_char, '\0', '\0', '\0'};
        int special_len = get_max_special_len(temp_str);
        
        // Если это начало специального слова
        if (special_len > 0) {
            // Если было обычное слово, добавляем его
            if (buf_pos > 0) {
                word_buf[buf_pos] = '\0';
                add_word(list, word_buf);
                buf_pos = 0;
            }
            
            // Собираем полное специальное слово
            char special_word[4] = {current_char, '\0', '\0', '\0'};
            int current_len = 1;
            
            // Пытаемся получить более длинное специальное слово
            for (int i = 1; i < 3; i++) {
                if (block_pos >= block_len) break;
                
                special_word[i] = block[block_pos];
                special_word[i+1] = '\0';
                
                int new_len = get_max_special_len(special_word);
                if (new_len > current_len) {
                    current_len = new_len;
                    getsym(stdin); // Пропускаем символ так как он часть специального слова
                } else {
                    break;
                }
            }
            
            special_word[current_len] = '\0';
            add_word(list, special_word);
            continue;
        }
        
        // Обычный символ - добавляем в буфер
        // Увеличиваем буфер если нужно
        if (buf_pos >= buf_size - 1) {
            buf_size *= 2;
            char *new_buf = realloc(word_buf, buf_size);
            if (new_buf == NULL) {
                printf("Ошибка памяти\n");
                free(word_buf);
                return;
            }
            word_buf = new_buf;
        }
        
        word_buf[buf_pos++] = current_char;
    }
    
    free(word_buf);
}
