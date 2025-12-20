// pwd.c

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
	char *cwd = NULL;

	/*динамический буфер*/
	cwd = malloc(4096);

	if (cwd == NULL) {
		perror("malloc");
		return 1;
	}

	/*получение текущей директории*/
	if (getcwd(cwd, 4096) != NULL) {
		printf("%s\n", cwd);
	}
	else {
		perror("pwd");
		free(cwd);
		return 1;
	}

	free(cwd);
	return 0;
}

