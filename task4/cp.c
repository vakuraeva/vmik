// cp.c
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
    char buffer[1024];
    ssize_t bytes_read;
    struct stat file_info;
    
    if (argc > 3) {
        fprintf(stderr, "Ошибка: Слишком много аргументов\n");
        return -1;
    }
    
    /*не совпадают*/
    if (strcmp(argv[1], argv[2]) == 0) {
        fprintf(stderr, "Ошибка: '%s' и '%s' это один и тот же файл\n", 
                argv[1], argv[2]);
        return -1;
    }
    
    int read_file;
    int write_file;
    
    if ((read_file = open(argv[1], O_RDONLY)) == -1) {
        fprintf(stderr, "Ошибка: невозможно открыть файл %s\n", argv[1]);
        return -1;
    }
    
    stat(argv[1], &file_info);
    
    if ((write_file = creat(argv[2], file_info.st_mode)) == -1) {
        fprintf(stderr, "Ошибка: невозможно создать файл %s\n", argv[2]);
        close(read_file);
        return -1;
    }
    
    while ((bytes_read = read(read_file, buffer, sizeof(buffer))) > 0) {
        write(write_file, buffer, bytes_read);
    }
    
    close(read_file);
    close(write_file);
    
    return 0;
}
