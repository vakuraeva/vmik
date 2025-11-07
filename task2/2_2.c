#include <stdio.h>

int main() {
    double x;
    scanf("%lf", &x);
    
    double result = 0;
    double coefficient;
    double derivatus;

    while (scanf("%lf", &coefficient) == 1) {
	derivatus = derivatus*x + result;
        result = result * x + coefficient;
    }
    
    printf("\n");
    printf("%.10g\n", result);
    printf("%.10g\n", derivatus);
    
    return 0;
}
