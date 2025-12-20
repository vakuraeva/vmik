// functions.c

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "functions.h"

int is_space(char c) {
    return c == ' ' || c == '\t' || c == '\n';
}

int is_splitter_1(char c) {
    return c == ';' || c == '<' || c == '|' || c == '&';
}

int is_splitter_2(char c) {
    return c == '>';
}

void free_vector_word(char** vw, int word_count) {
    for (int i = 0; i < word_count; i++) {
        free(vw[i]);
    }
    free(vw);
}

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
        char* tmp = malloc(2 * sizeof(char));
        tmp[0] = *start;
        tmp[1] = '\0';
        return tmp;
    }
    if (is_splitter_2(*start)) {
        char* tmp = malloc(3 * sizeof(char));
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
    if (*start == '"' || *start == '\'') {
        char* end = start + 1;
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
        char* end = start;
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
        if (strcmp(word, "$EUID") == 0) {
            long euid = geteuid();
            answer = malloc(32);
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
    char** vw = malloc(vw_size * sizeof(char*));
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
    return word == NULL || strcmp(word, "&") == 0 || strcmp(word, ";") == 0;
}

int word_is_IO_redirect(char* word) {
    return strcmp(word, "<") == 0 || strcmp(word, ">") == 0 || strcmp(word, ">>") == 0;
}

int word_is_command(char* word) {
    return word_is_delimiter(word) || word_is_IO_redirect(word) || strcmp(word, "|") == 0;
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
    node* answer_head = NULL;
    while (vw[i] != NULL) {
        syntax_assert(!word_is_command(vw[i]), "ERROR 1: command starts with '%s'\n", vw[i]);
        node* sub_head = NULL;
        while (!word_is_delimiter(vw[i])) {
            node* cur = malloc(sizeof(node));
            init_empty(cur);
            int argv_size = 8;
            int argv_index = 0;
            cur->argv = malloc(argv_size * sizeof(char*));
            syntax_assert(!word_is_command(vw[i]), "ERROR 2: command starts with '%s'\n", vw[i]);
            while (!word_is_delimiter(vw[i]) && strcmp(vw[i], "|") != 0) {
                if (word_is_IO_redirect(vw[i])) {
                    int filename_index = i + 1;
                    syntax_assert(!word_is_command(vw[filename_index]), "ERROR: expected filename after IO redirect\n");
                    if (strcmp(vw[i], "<") == 0) {
                        syntax_assert(cur->infile == NULL, "ERROR: repeatable INput redirect\n");
                        cur->infile = vw[filename_index];
                    } else {
                        syntax_assert(cur->outfile == NULL, "ERROR: repeatable OUTput redirect\n");
                        cur->outfile = vw[filename_index];
                        cur->append = strcmp(vw[i], ">>") == 0;
                    }
                    vw[filename_index] = NULL;
                    i += 2;
                } else {
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
            i += vw[i] != NULL && strcmp(vw[i], "|") == 0;
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

        if (strcmp(vw[i], "&") == 0) {
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

struct ZombieNode {
    int pid;
    struct ZombieNode* next;
};

struct ZombieNode* back_processes = NULL;

void zombie_push(int pid) {
    struct ZombieNode* p = malloc(sizeof(struct ZombieNode));
    p->pid = pid;
    p->next = back_processes;
    back_processes = p;
}

void check_zombies() {
    struct ZombieNode *new_head = NULL, *q;
    while (back_processes != NULL) {
        q = back_processes;
        back_processes = back_processes->next;
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

    int pipes[sons_cnt - 1][2];
    for (int i = 0; i < sons_cnt - 1; i++) {
        pipe(pipes[i]);
    }

    for (int i = 0; i < sons_cnt; i++) {
        node* info = sub_list;
        for (int j = 0; j < i; j++) {
            info = info->pipe;
        }

        if (strcmp(info->argv[0], "cd") == 0) {
            if (info->argv[1] == NULL) {
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

        if (strcmp(info->argv[0], "pwd") == 0) {
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

        if (strcmp(info->argv[0], "exit") == 0) {
            exit(0);
        }

        sons_pids[i] = fork();
        if (sons_pids[i] == 0) {
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
