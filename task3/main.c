// ===== main.c =====
#include <stdio.h>
#include <stdlib.h>
#include "wordlist.h"

#define BLOCK_SIZE 100

int main() {
    WordList list;
     
    
    // Инициализация списка
    init_list(&list);
    
     
    // Обработка ввода
    process_input(&list);
    
    // Освобождение памяти
    free_list(&list);
    return 0;
}
