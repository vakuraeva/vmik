//cat.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
	int line_count = 0; 
	char **filenames = NULL; 
	int file_count = 0;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-n") == 0) { 
				line_count = 1;
		}
		else { 
			filenames = realloc(filenames, (file_count + 1) * sizeof(char *));
			filenames[file_count++] = argv[i];
		}
	}

	
	for (int i = 0; i < file_count; i++) {
		FILE *file;

		
		file = fopen(filenames[i], "r");
		if (!file) {
	
			perror("cat");
			continue;
		}
		
		char line[1000];

         	while (fgets(line, sizeof(line), file)) {
               		if (line_count) {
                		printf("%4d  %s", line_count++, line);
                	} else {
                 		 printf("%s", line);
                	}
            	}

	
		fclose(file);
				
	}
	
	if (filenames) {
		free(filenames);
	}
	return 0;
}


