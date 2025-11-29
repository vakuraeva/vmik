// ===== main.c =====
#include <stdio.h>
#include <stdlib.h>
#include "wordlist.h"

int main() {
    WordList list;
    char buffer[1000];
    int i;
    
    // Инициализация списка
    init_list(&list);
    
    // Чтение строк из stdin пока не конец файла
    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        // Обработка строки и добавление слов в список
        process_line(buffer, &list);
        
        // Вывод количества слов
        printf("%d\n", list.count);
        
        // Вывод слов в исходном порядке
        for (i = 0; i < list.count; i++) {
            printf("%s\n", list.words[i]);
        }
        
        // Сортировка списка
        sort_list(&list);
        
        // Вывод слов в лексикографическом порядке
        for (i = 0; i < list.count; i++) {
            printf("%s\n", list.words[i]);
        }
        
        // Очистка списка для следующей строки
        clear_list(&list);
    }
    // Освобождение памяти
    free_list(&list);
    return 0;
}
