#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int key;
    struct Node *left;
    struct Node *right;
} Node;

Node* create_node(int key) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->key = key;
    node->left = NULL;
    node->right = NULL;
    return node;
}

Node* insert(Node* root, int key) {
    if (root == NULL) return create_node(key);
    
    if (key < root->key) 
        root->left = insert(root->left, key);
    else if (key > root->key) 
        root->right = insert(root->right, key);
    
    return root;
}

Node* min_value_node(Node* node) {
    Node* current = node;
    while (current && current->left != NULL)
        current = current->left;
    return current;
}

Node* delete(Node* root, int key) {
    if (root == NULL) return root;
    
    if (key < root->key) 
        root->left = delete(root->left, key);
    else if (key > root->key) 
        root->right = delete(root->right, key);
    else {
        if (root->left == NULL) {
            Node* temp = root->right;
            free(root);
            return temp;
        } else if (root->right == NULL) {
            Node* temp = root->left;
            free(root);
            return temp;
        }
        
        Node* temp = min_value_node(root->right);
        root->key = temp->key;
        root->right = delete(root->right, temp->key);
    }
    return root;
}

int search(Node* root, int key) {
    if (root == NULL) return 0;
    
    if (key == root->key) return 1;
    else if (key < root->key) return search(root->left, key);
    else return search(root->right, key);
}

void free_tree(Node* root) {
    if (root == NULL) return;
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

int main() {
    Node* root = NULL;
    char op;
    int key;
    
    while (scanf(" %c", &op) != EOF) {
        if (op == 'E') {
            free_tree(root);
            return 0;
        }
        else if (op == '+' || op == '-' || op == '?') {
            scanf("%d", &key);
            if (op == '+') {
                root = insert(root, key);
            } else if (op == '-') {
                root = delete(root, key);
            } else if (op == '?') {
                printf("%d %s\n", key, search(root, key) ? "yes" : "no");
            }
        }
    }
    
    free_tree(root);
    return 0;
}
