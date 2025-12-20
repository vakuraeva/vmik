// functions.h

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#define syntax_assert(b, ...)             \
    do {                                  \
        if (!(b)) {                       \
            fprintf(stderr, __VA_ARGS__); \
            exit(1);                      \
        }                                 \
    } while (0)

enum type_of_next {
    NXT,
    AND,
    OR
};

struct cmd_inf {
    char** argv;
    char* infile;
    char* outfile;
    int append;
    int backgrnd;
    struct cmd_inf* pipe;
    struct cmd_inf* next;
    enum type_of_next type;
};

typedef struct cmd_inf node;

int is_space(char c);
int is_splitter_1(char c);
int is_splitter_2(char c);
void free_vector_word(char** vw, int word_count);
char* get_word_in_str(char** s);
char* replace_word_by_env(char* word);
char** make_vector_word(char* s);
void print_vw(char** vw);
void init_empty(node* proc);
int word_is_delimiter(char* word);
int word_is_IO_redirect(char* word);
int word_is_command(char* word);
node* list_reverse_1(node* sub_head);
node* list_reverse_2(node* head);
node* syntax_analisys(char** vw);
void print_tree(node* list_of_lists);
void clear_tree(node* list_of_lists);
void zombie_push(int pid);
void check_zombies(void);
int conveyer(node* sub_list);
void execute(node* list_of_lists);

#endif
