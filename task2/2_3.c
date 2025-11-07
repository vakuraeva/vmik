#include <stdio.h>

long fib_iterative(int n) {
    if (n == 0) return 0;
    if (n == 1) return 1;
    
    long prev = 0;
    long current = 1;
    long next;
    
    for (int i = 2; i <= n; i++) {
        next = prev + current;
        prev = current;
        current = next;
    }
    
    return current;
}

long fib_recursive(int n) {
    if (n == 0) return 0;
    if (n == 1) return 1;
    
    return fib_recursive(n - 2) + fib_recursive(n - 1);
}

int main() {
    int n;
    
    while (scanf("%d", &n) == 1) {
        long result_iter = fib_iterative(n);
        printf("%ld\n", result_iter);
          
        long result_rec = fib_recursive(n);
        printf("%ld\n", result_rec);
    }
    
    return 0;
}
