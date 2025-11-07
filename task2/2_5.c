#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Node {
    char *word;
    struct Node *next;
};

char* read_input_string() {
    char input[1000];
    fgets(input, 1000, stdin);
    char *result = malloc(strlen(input) + 1);
    strcpy(result, input);
    return result;
}

struct Node* create_list(char *s) {
    struct Node *cur;
    struct Node *res;
    
    char *token = strtok(s, " \t\n");
    if (token == NULL) return NULL;
    
    res = cur = malloc(sizeof(struct Node));
    cur->word = malloc(strlen(token) + 1);
    strcpy(cur->word, token);
    
    token = strtok(NULL, " \t\n");
    while (token != NULL) {
        cur = cur->next = malloc(sizeof(struct Node));
        cur->word = malloc(strlen(token) + 1);
        strcpy(cur->word, token);
        token = strtok(NULL, " \t\n");
    }
    cur->next = NULL;
    return res;
}

void print_list(struct Node *head) {
    struct Node *current = head;
    while (current != NULL) {
        printf("%s", current->word);
        if (current->next != NULL) {
            printf(" ");
        }
        current = current->next;
    }
    printf("\n");
}

struct Node* modify_list(struct Node *head) {
    if (head == NULL) {
        return NULL;
    }
    
    struct Node *tail = head;
    while (tail->next != NULL) {
        tail = tail->next;
    }
    char *last_word = tail->word;
    
    struct Node *current = head;
    struct Node *prev = NULL;
    
    while (current != NULL) {
        if (current != tail && strcmp(current->word, last_word) == 0) {
            if (prev == NULL) {
                head = current->next;
                free(current->word);
                free(current);
                current = head;
            } else {
                prev->next = current->next;
                free(current->word);
                free(current);
                current = prev->next;
            }
        } else {
            prev = current;
            current = current->next;
        }
    }
    
    return head;
}

int main() {
    char *input_string = read_input_string();
    struct Node *head = create_list(input_string);
    free(input_string);
    head = modify_list(head);
    print_list(head);
    
    return 0;
}
