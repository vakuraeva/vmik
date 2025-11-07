#include <stdio.h>

int use_counter=0;
int counter = 0;
double str2double(char str[]) {
    double result = 0.0;
    double fraction = 0.1;
    int i = 0;
    int sign = 1;
    int has_point = 0;
    int exponent = 0;
    int exp_sign = 1;
    use_counter = 0; 
    
    // Проверяем знак
    if (str[i] == '-') {
        sign = -1;
        i++;
    } else if (str[i] == '+') {
        i++;
    }
    
    // Целая часть
    while (str[i] >= '0' && str[i] <= '9') {
        result = result * 10 + (str[i] - '0');
        i++;
    }
    
    // Дробная часть
    if (str[i] == '.') {
        has_point = 1;
        i++;
        while (str[i] >= '0' && str[i] <= '9'){ 

            result = result + (str[i] - '0') * fraction;
            fraction = fraction * 0.1;
            i++;
        }
    }
    
    // Экспонента
    if (str[i] == 'e' || str[i] == 'E' || str[i] == 'm') {
	if (str[i] == 'm') {
		use_counter = 1;
	}
        i++;
        
        // Знак экспоненты
        if (str[i] == '-') {
            exp_sign = -1;
            i++;
        } else if (str[i] == '+') {
            i++;
        }
        
        // Значение экспоненты
        while (str[i] >= '0' && str[i] <= '9') {
            exponent = exponent * 10 + (str[i] - '0');
            i++;
        }
    }
    
    // Применяем экспоненту
    if (exponent > 0) {
        if (exp_sign == 1) {
            for (int j = 0; j < exponent; j++) {
                result = result * 10;
            }
        } else {
            for (int j = 0; j < exponent; j++) {
                result = result / 10;
            }
        }
    }
    
    return sign * result;
}

int main() {
    char str[100];
    use_counter=0;
    int  int_number = 0; 
    while (scanf("%s", str) == 1) {
        double number = str2double(str);
        printf("%.10g\n", number);
	if (use_counter = 1) {
		counter = 0;
		
		int_number = (int)number;
		
		while (int_number >  0){
			int_number = int_number/10;
			counter++;
			
		}	
	printf("%d\n", counter);
	}

    }
    
    return 0;
}
