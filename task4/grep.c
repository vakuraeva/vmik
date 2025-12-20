// grep.c


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
	if (argc < 3) {
		fprintf(stderr, "Должно быть больше аргументов\n");
		return 1;
	}

	char *substring = argv[1];
	char *filename = argv[2];
	int v_fl = 0;


	if (argc > 3 && strcmp(argv[1], "-v") == 0) {
		v_fl = 1;
		substring = argv[2];
		filename = argv[3];
	}

	FILE *file = fopen(filename, "r");
	if (!file) {
		perror("grep");
		return 1;
	}

	char line[1000];


	while (fgets(line, sizeof(line), file)) {
	
		line[strcspn(line, "\n")] = 0;

		int cont = (strstr(line, substring) != NULL);

		if ((cont && !v_fl) || (!cont && v_fl)) {
			printf("%s\n", line);
		}


	}

	fclose(file);
	return 0;
}
