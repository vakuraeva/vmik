#include <stdio.h>
#include <setjmp.h>
#include <string.h>

jmp_buf begin;
char curlex;

void getlex(void);
int expr(void);
int term(void);
int factor(void);
int power(void);
void error(void);

int main() {
    while (1) {
        setjmp(begin);
        printf("==>");
        getlex();
        int result = expr();
        if (curlex != '\n') error();
        printf("\n%d\n", result);
        
        // Очистка буфера после вычисления
        while (getchar() != '\n');
    }
    return 0;
}

void getlex() {
    while ((curlex = getchar()) == ' ');
}

void error(void) {
    printf("\nERROR\n");
    while (getchar() != '\n');
    longjmp(begin, 1);
}

int expr() {
    int e = term();
    while (curlex == '+' || curlex == '-') {
        char op = curlex;
        getlex();
        if (op == '+') e += term();
        else e -= term();
    }
    return e;
}

int term() {
    int t = factor();
    while (curlex == '*' || curlex == '/') {
        char op = curlex;
        getlex();
        if (op == '*') t *= factor();
        else t /= factor();
    }
    return t;
}

int factor() {
    int f = power();
    while (curlex == '\'') {
        getlex();
        int p = power();
        int result = 1;
        for (int i = 0; i < p; i++) result *= f;
        f = result;
    }
    return f;
}

int power() {
    int p;
    switch (curlex) {
        case '0': case '1': case '2': case '3': case '4': case '5':
        case '6': case '7': case '8': case '9': 
            p = curlex - '0'; 
            break;
        case '(': 
            getlex(); 
            p = expr();
            if (curlex != ')') error();
            break;
        default: 
            error();
    }
    getlex();
    return p;
}
