// ===== wordlist.h =====
#ifndef WORDLIST_H
#define WORDLIST_H

typedef struct {
    char **words;
    int count;
    int capacity;
} WordList;

void init_list(WordList *list);
void add_word(WordList *list, const char *word);
void sort_list(WordList *list);
void clear_list(WordList *list);
void free_list(WordList *list);
void process_input(WordList *list);
void process_line_from_blocks(WordList *list);

#endif
