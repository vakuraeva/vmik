// echo.c

#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
	int no_n = 0; /*для флага -n*/
	int escapes = 0; /*для флага -e*/

	/*обрабатываем флаги*/
	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (strcmp(argv[i], "-n") == 0) {
				no_n = 1;
			}
			else if (strcmp(argv[i], "-e") == 0) {
				escapes = 1;
			}
		}
	}

	/*выводим аргументы*/
	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-') continue;

		if (escapes) {
			char *arg = argv[i];
			while (*arg) {
				if (*arg == '\\' && *(arg + 1)) {
					switch (*(arg + 1)) {
						case 'n': putchar('\n'); break; /*перевод строки*/
						case 't': putchar('\t'); break; /*табуляция*/
						case '\\': putchar('\\'); break; /*один слэш*/
						default: putchar(*arg); putchar(*(arg + 1)); break;
					}
					arg += 2;
				} else {
					putchar(*arg);
					arg++;
				}
			}
		} 
		else {
			printf("%s", argv[i]);
		}

		if (i < argc - 1) {
			printf(" ");
		}
	}

	if (!no_n) {
		printf("\n");
	}

	return 0;
}
