#include <stdio.h>

int main() {
    double epsilon;
    scanf("%lf", &epsilon);
    
    double x;
    while (scanf("%lf", &x) == 1) {
        if (x == 0.0) {
            printf("%.10g\n", 0.0);
            continue;
        }
        
        double current = 1.0;
        double next;
        
        while (1) {
            next = 0.5 * (current + x / current);

              double difference;
          if (current > next) {
              difference = current - next;
         } else {
         difference = next - current;
         }
 
         if (difference < epsilon) {
            break;
         }  
         current = next;
        }   
         
        printf("%.10g\n", next);
    }
    
    return 0;
}
