/*
Шелл типовой, подвариант А.

ЛОГИЧЕСКИЕ ОТЛИЧИЯ от другого подварианта:
"Должна быть реализована хотя бы одна из следующих операций": реализована ";"
"Желательно реализовать хотя бы одну из следующих возможностей": возможность
  выполнения команды cmd путем передачи ее в качестве параметра при вызове my_shell:

КАК СДЕЛАТЬ КОД БОЛЕЕ ОТЛИЧАЮЩИМСЯ?
Разделить дальнейшие пункты между вариантами и выполнить:
1.  переименовать переменные
2.  разбить программу на модули, как в методичке
3.  подставить в код тело функций str_equal, is_splitter_1, is_splitter_2, init_empty ...
4.  разбить syntax_analisys на подфункции
5.  выводить другие сообщения об ошибках, возможно убрать некоторые assert'ы
6.  использовать другой форматтер кода (в VS Code: install extension Clang-formatter, init ~/.clang-format,
    CTRL+SHIFT+I)
7.  убрать комментарии
8.  поменять приглашение к вводу в функции main: вместо printf("$ ") сделать "=> "
9.  изменить стиль дебага.

ЧТО ПОТЕНЦИАЛЬНО ОПАСНО:
- при ошибочном синтаксисе программа завершает работу, а не обрабатывает ошибку.
  Это связано с тем, что сложно не допустить утечку памяти.
- экранирующий \\ работает не совсем корректно, см "echo hello \; echo world"

БНФ:
<слово>       ::= {буква | цифра | ниж_подчеркивание | ...}
<команда>     ::= <слово> {<слово>} [<перенаправление_ввода>]  {<слово>} [<перенаправление_вЫвода>] {<слово>} |
                  <слово> {<слово>} [<перенаправление_вЫвода>] {<слово>} [<перенаправление_ввода>]  {<слово>}
<конвейер>    ::= <команда> {'|' <команда>}
<строка>      ::= <конвейер> { [один из ';' '&'] <конвейер> } [один из ';' '&'] [<комментарий>]
<комментарий> ::= #{<любой_символ>}
*/

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/* МАКРОСЫ */

#define syntax_assert(b, ...)             \
    do {                                  \
        if (!(b)) {                       \
            fprintf(stderr, __VA_ARGS__); \
            exit(1); /* авария */         \
        }                                 \
    } while (0)
#define str_equal(s1, s2) (strcmp(s1, s2) == 0)

/*
ЛЕКСИЧЕСКИЙ АНАЛИЗ
Входные данные: строка
Выходные данные: вектор слов (char**)
Задача этапа: разбить входную строку на "лексемы", то есть на слова.
Зачем: со словами проще работать, чем со строкой символов.
*/

/* символы-разделители */
int is_space(char c) {
    return c == ' ' || c == '\t' || c == '\n';
}

/* операторы, гарантированно состоящие из одного символа */
int is_splitter_1(char c) {
    return c == ';' || c == '<' || c == '|' || c == '&';
}

/* операторы, состоящие из одного или двух символов */
int is_splitter_2(char c) {
    return c == '>';
}

void free_vector_word(char** vw, int word_count) {
    for (int i = 0; i < word_count; i++) {
        free(vw[i]);
    }
    free(vw);
}

// прочитать очередное слово из входной строки, выделить на него память и вернуть память
char* get_word_in_str(char** s) {
    char* start = *s;
    while (is_space(*start)) {
        start++;
    }
    if (*start == '\0') {
        return NULL;
    }
    if (is_splitter_1(*start)) {
        *s = start + 1;
        char* tmp = (char*)malloc(2 * sizeof(char));
        tmp[0] = *start;
        tmp[1] = '\0';
        return tmp;
    }
    if (is_splitter_2(*start)) {
        char* tmp = (char*)malloc(3 * sizeof(char));
        tmp[0] = *start;
        if (*start == *(start + 1)) {
            tmp[1] = *start;
            tmp[2] = '\0';
            *s = start + 2;
        } else {
            *s = start + 1;
            tmp[1] = '\0';
        }
        return tmp;
    }
    // написана не команда: обычное слово или строка в кавычках
    char* end = start;
    if (*start == '"' || *start == '\'') {
        // кавычки
        end++;
        while (*end != *start) {
            syntax_assert(*end != '\0', "LEXIC ERROR: end of line, but quotation mark is not closed!\n");
            end++;
        }
        start++;
        *end = ' ';

        int len = end - start;
        char* tmp = malloc((len + 1) * sizeof(char));
        tmp[len] = '\0';
        memcpy(tmp, start, len);
        *s = end;
        return tmp;
    } else {
        int buf_size = 8, buf_index = 0;
        char* tmp = malloc(buf_size * sizeof(char));
        // слово: команда или её параметр или её флаг
        while (!is_space(*end) && *end != '\0' && !is_splitter_1(*end) && !is_splitter_2(*end)) {
            if (*end == '\\') {
                end++;
            }
            syntax_assert(*end != '\0', "ERROR: end of line after \\\n");
            tmp[buf_index] = *end;
            buf_index++;
            if (buf_index == buf_size - 1) {
                buf_size *= 2;
                tmp = realloc(tmp, buf_size * sizeof(char));
            }
            end++;
        }
        tmp[buf_index] = '\0';
        *s = end;
        return tmp;
    }
}

char* replace_word_by_env(char* word) {
    if (word == NULL || word[0] != '$') {
        return word;
    }
    char* answer = getenv(word + 1);
    if (answer == NULL) {
        if (str_equal(word, "$EUID")) {
            long euid = geteuid();
            answer = (char*)malloc(32);
            sprintf(answer, "%ld", euid);
        } else {
            answer = strdup("");
        }
    } else {
        answer = strdup(answer);
    }
    free(word);
    return answer;
}

char** make_vector_word(char* s) {
    for (char* c = s; *c != '\0'; c++) {
        if (*c == '#') {
            *c = ' ';
            for (; *c != '\0'; c++) {
                *c = ' ';
            }
            break;
        }
    }
    int vw_size = 8;
    char** vw = (char**)malloc(vw_size * sizeof(char*));
    int index = 0;
    char* c_word;
    while (1) {
        c_word = get_word_in_str(&s);
        c_word = replace_word_by_env(c_word);
        vw[index] = c_word;
        index++;
        if (c_word == NULL) {
            return vw;
        }
        if (index == vw_size) {
            // этот assert можно безопасно убрать
            syntax_assert(vw_size < 1024, "ERROR: shell command is too long (%d words)\n", vw_size);
            vw_size *= 2;
            vw = realloc(vw, vw_size * sizeof(char*));
        }
    }
}

void print_vw(char** vw) {
    printf("[");
    for (int index = 0; vw[index] != NULL; index++) {
        printf("'%s' ", vw[index]);
    }
    printf("]\n");
}

/*
СИНТАКСИЧЕСКИЙ АНАЛИЗ
Входные данные: вектор слов
Выходные данные: структура, которую удобно было бы использовать для запуска команд
Задача: понять, что последовательность лексем соответствует синтаксису шелла,
        и преобразовать в более удобную для исполнения форму.
Зачем: нужно убедиться, что строка корректна, и только после этого исполнять её

Структура данных: список списков.

A | B ; C & D | E | F ; G | H

A->C->D->G
|     |  |
B     E  H
      |
      F

A связана с B по cmd_inf, A связана с C по next
*/

enum type_of_next {
    // виды связей соседних команд в списке команд, используется только NXT
    NXT,
    AND,
    OR
};

struct cmd_inf {
    char** argv;             // список из имени команды и аргументов
    char* infile;            // переназначенный файл стандартного ввода
    char* outfile;           // переназначенный файл стандартного вывода
    int append;              // TRUE для >> FALSE для >
    int backgrnd;            // =1, если команда подлежит выполнению в фоновом режиме
    struct cmd_inf* pipe;    // следующая команда после "|"
    struct cmd_inf* next;    // следующая после ";" (или после "&")
    enum type_of_next type;  // связь со следующей командой через ; или && или ||
};

typedef struct cmd_inf node;

void init_empty(node* proc) {
    proc->infile = NULL;
    proc->outfile = NULL;
    proc->pipe = NULL;
    proc->next = NULL;
    proc->append = 0;
    proc->backgrnd = 0;
    proc->type = NXT;
}

int word_is_delimiter(char* word) {
    return word == NULL || str_equal(word, "&") || str_equal(word, ";");
}

int word_is_IO_redirect(char* word) {
    return str_equal(word, "<") || str_equal(word, ">") || str_equal(word, ">>");
}

int word_is_command(char* word) {
    return word_is_delimiter(word) || word_is_IO_redirect(word) || str_equal(word, "|");
}

node* list_reverse_1(node* sub_head) {
    node *sub_head_2 = NULL, *helper;
    while (sub_head != NULL) {
        helper = sub_head;
        sub_head = sub_head->pipe;
        helper->pipe = sub_head_2;
        sub_head_2 = helper;
    }
    return sub_head_2;
}

node* list_reverse_2(node* head) {
    node *head_2 = NULL, *helper;
    while (head != NULL) {
        helper = head;
        head = head->next;
        helper->next = head_2;
        head_2 = helper;
    }
    return head_2;
}

node* syntax_analisys(char** vw) {
    int i = 0;
    // списки строятся ПЕРЕВЁРНУТЫМИ! потому что так проще.
    // Потом переверну ещё раз, и станет норм
    node* answer_head = NULL;
    // внешний цикл обрабатывает целые конвейеры, разделенные ; и &
    while (vw[i] != NULL) {
        syntax_assert(!word_is_command(vw[i]), "ERROR 1: command starts with '%s'\n", vw[i]);
        node* sub_head = NULL;
        // одна итерация внутреннего цикла - один конвейер
        while (!word_is_delimiter(vw[i])) {
            // формируем argv и другую информацию
            node* cur = (node*)malloc(sizeof(node));
            init_empty(cur);
            int argv_size = 8;
            int argv_index = 0;
            cur->argv = (char**)malloc(argv_size * sizeof(char*));
            syntax_assert(!word_is_command(vw[i]), "ERROR 2: command starts with '%s'\n", vw[i]);
            while (!word_is_delimiter(vw[i]) && !str_equal(vw[i], "|")) {
                // прочитать все слова до ЛЮБОГО разделителя между командами шелла
                if (word_is_IO_redirect(vw[i])) {
                    int filename_index = i + 1;
                    syntax_assert(!word_is_command(vw[filename_index]), "ERROR: expected filename after IO redirect\n");
                    if (str_equal(vw[i], "<")) {
                        syntax_assert(cur->infile == NULL, "ERROR: repeatable INput redirect\n");
                        cur->infile = vw[filename_index];
                    } else {
                        syntax_assert(cur->outfile == NULL, "ERROR: repeatable OUTput redirect\n");
                        cur->outfile = vw[filename_index];
                        cur->append = str_equal(vw[i], ">>");
                    }
                    vw[filename_index] = NULL;
                    i += 2;
                } else {
                    // обычное слово
                    cur->argv[argv_index] = vw[i];
                    vw[i] = NULL;
                    i++;
                    argv_index++;
                    if (argv_index == argv_size - 1) {
                        argv_size *= 2;
                        cur->argv = realloc(cur->argv, argv_size * sizeof(char*));
                    }
                }
            }
            i += vw[i] != NULL && str_equal(vw[i], "|");
            cur->argv[argv_index] = NULL;
            cur->pipe = sub_head;
            sub_head = cur;
        }
        sub_head = list_reverse_1(sub_head);
        sub_head->next = answer_head;
        answer_head = sub_head;

        if (vw[i] == NULL) {
            break;
        }

        if (str_equal(vw[i], "&")) {
            sub_head->backgrnd = 1;
        }
        i++;
    }
    return list_reverse_2(answer_head);
}

void print_tree(node* list_of_lists) {
    node* head_of_heads = list_of_lists;
    while (head_of_heads != NULL) {
        printf("-- conveyer --\n");
        node* sub_head = head_of_heads;
        while (sub_head != NULL) {
            printf("argv: [");
            for (int index = 0; sub_head->argv[index] != NULL; index++) {
                printf("'%s'", sub_head->argv[index]);
            }
            printf("]\n< '%s' >%c '%s' %c\n", sub_head->infile, sub_head->append ? '>' : '_', sub_head->outfile,
                   sub_head->backgrnd ? '&' : '_');
            sub_head = sub_head->pipe;
        }
        head_of_heads = head_of_heads->next;
    }
}

void clear_tree(node* list_of_lists) {
    node* head_of_heads = list_of_lists;
    while (head_of_heads != NULL) {
        node *sub_head = head_of_heads, *helper;
        head_of_heads = head_of_heads->next;
        while (sub_head != NULL) {
            helper = sub_head;
            sub_head = sub_head->pipe;
            for (int i = 0; helper->argv[i] != NULL; i++) {
                free(helper->argv[i]);
            }
            free(helper->argv);
            free(helper->infile);
            free(helper->outfile);
            free(helper);
        }
    }
}

/*
ИСПОЛНЕНИЕ

Входные данные: список списков команд
Выходные данные: процессы и пайпы
Задача: запустить конвейеры
*/

struct ZombieNode {
    int pid;
    struct ZombieNode* next;
};
struct ZombieNode* back_processes = NULL;

void zombie_push(int pid) {
    struct ZombieNode* p = (struct ZombieNode*)malloc(sizeof(struct ZombieNode));
    p->pid = pid;
    p->next = back_processes;
    back_processes = p;
}

void check_zombies()  // работа с фоновыми демонами
{
    struct ZombieNode *new_head = NULL, *q;
    while (back_processes != NULL) {
        q = back_processes;
        back_processes = back_processes->next;
        // проверить жив ли процесс
        int status;
        int ret_code = waitpid(q->pid, &status, WNOHANG);
        if (ret_code == q->pid) {
            printf("Process with pid=%d stopped with ret_code=%d\n", q->pid, WEXITSTATUS(status));
            waitpid(q->pid, NULL, 0);
            free(q);
        } else {
            q->next = new_head;
            new_head = q;
        }
    }
    back_processes = new_head;
}

int conveyer(node* sub_list) {
    int sons_cnt = 0;
    for (node* head = sub_list; head != NULL; head = head->pipe) {
        sons_cnt++;
    }

    int sons_pids[sons_cnt];
    int sons_ret_codes[sons_cnt];
    for (int i = 0; i < sons_cnt; i++) {
        sons_pids[i] = -1;
        sons_ret_codes[i] = -1;
    }

    // создать пайпы для взаимодействия сыновей
    int pipes[sons_cnt - 1][2];
    for (int i = 0; i < sons_cnt - 1; i++) {
        pipe(pipes[i]);
    }

    // запустить сыновей
    for (int i = 0; i < sons_cnt; i++) {
        node* info = sub_list;
        for (int j = 0; j < i; j++) {
            info = info->pipe;
        }

        if (str_equal(info->argv[0], "cd")) {
            if (info->argv[1] == NULL) {
                // go to /home/user
                sons_ret_codes[i] = chdir(getenv("HOME"));
                continue;
            }
            if (info->argv[1][0] == '/') {
                sons_ret_codes[i] = chdir(info->argv[1]);
                continue;
            }
            char c_dir[16384];
            memset(c_dir, 0, 16384);
            getcwd(c_dir, 16383);
            strcat(c_dir, "/");
            strcat(c_dir, info->argv[1]);
            sons_ret_codes[i] = chdir(c_dir);
            continue;
        }

        if (str_equal(info->argv[0], "pwd")) {
            char c_dir[16384];
            memset(c_dir, 0, 16384);
            getcwd(c_dir, 16383);
            int len = strlen(c_dir);
            if (info->outfile != NULL) {
                int fd = open(info->outfile, O_WRONLY | O_CREAT | (info->append ? O_APPEND : O_TRUNC), 0640);
                if (fd == -1) {
                    printf("ERROR: can't create out file '%s'\n", info->outfile);
                    sons_ret_codes[i] = 1;
                    continue;
                }
                write(fd, c_dir, len);
                close(fd);
                sons_ret_codes[i] = 0;
                continue;
            }
            if (i != sons_cnt - 1) {
                sons_ret_codes[i] = write(pipes[i][1], c_dir, len) == len;
                continue;
            }
            sons_ret_codes[i] = printf("%s\n", c_dir) == 0;
            continue;
        }

        if (str_equal(info->argv[0], "exit")) {
            exit(0);
        }

        sons_pids[i] = fork();
        if (sons_pids[i] == 0) {
            // КОД СЫНА
            // перенапроавить ВВ-ВВ в пайп
            if (sons_cnt > 1) {
                if (i == 0) {
                    dup2(pipes[0][1], 1);
                } else if (i == sons_cnt - 1) {
                    dup2(pipes[sons_cnt - 2][0], 0);
                } else {
                    dup2(pipes[i - 1][0], 0);
                    dup2(pipes[i][1], 1);
                }
            }

            for (int i = 0; i < sons_cnt - 1; i++) {
                close(pipes[i][0]);
                close(pipes[i][1]);
            }

            if (info->infile != NULL) {
                int fd = open(info->infile, O_RDONLY);
                if (fd == -1) {
                    printf("ERROR: can't create in file '%s'\n", info->infile);
                    exit(1);
                }
                dup2(fd, 0);
                close(fd);
            }

            if (info->outfile != NULL) {
                int fd = open(info->outfile, O_WRONLY | O_CREAT | (info->append ? O_APPEND : O_TRUNC), 0640);
                if (fd == -1) {
                    printf("ERROR: can't create out file '%s'\n", info->outfile);
                    exit(1);
                }
                dup2(fd, 1);
                close(fd);
            }

            execvp(info->argv[0], info->argv);
            printf("SON %d: EXEC ERROR\n", i);
            exit(1);
        }
    }

    for (int i = 0; i < sons_cnt - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    for (int i = 0; i < sons_cnt; ++i) {
        if (sons_pids[i] != -1) {
            waitpid(sons_pids[i], &sons_ret_codes[i], 0);
        }
    }

    return WEXITSTATUS(sons_ret_codes[sons_cnt - 1]);
}

void execute(node* list_of_lists) {
    for (node* conv = list_of_lists; conv != NULL; conv = conv->next) {
        if (conv->backgrnd) {
            int son = fork();
            if (son == 0) {
                // set group_id to himself
                setpgid(0, 0);
                int fd = open("/dev/null", O_RDONLY);
                dup2(fd, 0);
                close(fd);
                int ret_code = conveyer(conv);
                exit(ret_code);
            }
            zombie_push(son);
        } else {
            conveyer(conv);
        }
    }
}

/*
main
считывает данные, запускает другие этапы
*/

void sigint_dummy_handler(int x) {
    printf("\n");
}

int main(int argc, char** argv, char** env) {
    signal(SIGINT, sigint_dummy_handler);

    if (argc > 1) {
        if (str_equal(argv[1], "-s")) {
            // копипаста с изменениями
            for (int i = 2; i < argc; i++) {
                char** vw = make_vector_word(argv[i]);
                int word_count;
                for (word_count = 0; vw[word_count] != NULL; word_count++) {
                    ;
                }
                if (word_count == 0) {
                    free_vector_word(vw, word_count);
                    continue;
                }
                node* t = syntax_analisys(vw);
                execute(t);
                free_vector_word(vw, word_count);
                clear_tree(t);
            }
        } else if (str_equal(argv[1], "-v")) {
            // копипаста с изменениями
            int word_count = argc - 2;
            char** vector_word = (char**)malloc(sizeof(char*) * (word_count + 1));
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
        for (word_count = 0; vw[word_count] != NULL; word_count++) {
            ;
        }
        if (word_count == 0) {
            free_vector_word(vw, word_count);
            free(line);
            check_zombies();
            continue;
        }
#ifdef DEBUG
        print_vw(vw);
#endif
        node* t = syntax_analisys(vw);
#ifdef DEBUG
        print_tree(t);
#endif
        execute(t);
        free_vector_word(vw, word_count);
        clear_tree(t);
        free(line);
        check_zombies();
    }
    return 0;
}
