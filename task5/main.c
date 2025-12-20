// main.c
/*БНФ:
<слово>       ::= {буква | цифра | ниж_подчеркивание | ...}
<команда>     ::= <слово> {<слово>} [<перенаправление_ввода>]  {<слово>} [<перенаправление_вЫвода>] {<слово>} |
                  <слово> {<слово>} [<перенаправление_вЫвода>] {<слово>} [<перенаправление_ввода>]  {<слово>}
<конвейер>    ::= <команда> {'|' <команда>}
<строка>      ::= <конвейер> { [';'] <конвейер> } [';'] [<комментарий>]
<комментарий> ::= #{<любой_символ>}
*/

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "functions.h"


void sigint_dummy_handler(int x) {
    printf("\n");
}

int main(int argc, char** argv, char** env) {
    signal(SIGINT, sigint_dummy_handler);

    if (argc > 1) {
        if (strcmp(argv[1], "-s") == 0) {
            for (int i = 2; i < argc; i++) {
                char** vw = make_vector_word(argv[i]);
                int word_count;
                for (word_count = 0; vw[word_count] != NULL; word_count++);
                if (word_count == 0) {
                    free_vector_word(vw, word_count);
                    continue;
                }
                node* t = syntax_analisys(vw);
                execute(t);
                free_vector_word(vw, word_count);
                clear_tree(t);
            }
        } else if (strcmp(argv[1], "-v") == 0) {
            int word_count = argc - 2;
            char** vector_word = malloc(sizeof(char*) * (word_count + 1));
            for (int i = 2; i < argc; i++) {
                vector_word[i - 2] = strdup(argv[i]);
            }
            vector_word[word_count] = NULL;
            node* t = syntax_analisys(vector_word);
            execute(t);
            free_vector_word(vector_word, word_count);
            clear_tree(t);
        } else {
            fprintf(stderr, "ERROR: to use ARGV specify flag -s/-v as argv[1]");
        }
    }

    while (1) {
        printf("$ ");
        char* line = NULL;
        size_t str_size, buf_size;
        str_size = getline(&line, &buf_size, stdin);
        if (str_size == -1) {
            free(line);
            check_zombies();
            break;
        }
        char** vw = make_vector_word(line);
        int word_count;
        for (word_count = 0; vw[word_count] != NULL; word_count++);
        if (word_count == 0) {
            free_vector_word(vw, word_count);
            free(line);
            check_zombies();
            continue;
        }
        node* t = syntax_analisys(vw);
        execute(t);
        free_vector_word(vw, word_count);
        clear_tree(t);
        free(line);
        check_zombies();

    }
    return 0;
}
