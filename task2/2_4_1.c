#include <stdio.h>

// Функция для нахождения максимальной цифры в целом числе
int find_max_digit(long long num) {
    if (num < 0) num = -num; // Работаем с положительным числом
    
    int max_digit = 0;
    
    // Обрабатываем случай, когда число 0
    if (num == 0) {
        return 0;
    }
    
    // Перебираем все цифры числа
    while (num > 0) {
        int digit = num % 10; // Получаем последнюю цифру
        if (digit > max_digit) {
            max_digit = digit;
        }
        num = num / 10; // Убираем последнюю цифру
    }
    
    return max_digit;
}

double str2double(char str[]) {
    double result = 0.0;
    double fraction = 0.1;
    int i = 0;
    int sign = 1;
    int has_point = 0;
    int exponent = 0;
    int exp_sign = 1;
    int use_m = 0; // Флаг для буквы 'm'
    
    // Пропускаем пробелы в начале
    while (str[i] == ' ') {
        i++;
    }
    
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
        while (str[i] >= '0' && str[i] <= '9') {
            result = result + (str[i] - '0') * fraction;
            fraction = fraction * 0.1;
            i++;
        }
    }
    
    // Экспонента (обрабатываем и 'e' и 'm')
    if (str[i] == 'e' || str[i] == 'E' || str[i] == 'm' || str[i] == 'M') {
        // Запоминаем, если это 'm'
        if (str[i] == 'm' || str[i] == 'M') {
            use_m = 1;
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
    
    // Если использовали 'm', находим максимальную цифру целой части
    if (use_m) {
        // Берем целую часть числа
        long long int_part = (long long)result;
        if (int_part < 0) int_part = -int_part; // Работаем с положительным
        
        int max_digit = find_max_digit(int_part);
        
        // Возвращаем число и максимальную цифру (используем отрицательное значение как флаг)
        // Для простоты будем выводить в основной программе
        return sign * result; // Само число возвращаем как обычно
    }
    
    return sign * result;
}

int main() {
    char str[100];
    
    while (scanf("%s", str) == 1) {
        double number = str2double(str);
        
        // Проверяем, была ли в строке буква 'm'
        int has_m = 0;
        for (int i = 0; str[i] != '\0'; i++) {
            if (str[i] == 'm' || str[i] == 'M') {
                has_m = 1;
                break;
            }
        }
        
        if (has_m) {
            // Берем целую часть числа для поиска максимальной цифры
            long long int_part = (long long)number;
            if (int_part < 0) int_part = -int_part; // Работаем с положительным
            
            int max_digit = find_max_digit(int_part);
            printf("%.10g (max digit: %d)\n", number, max_digit);
        } else {
            printf("%.10g\n", number);
        }
    }
    
    return 0;
}
