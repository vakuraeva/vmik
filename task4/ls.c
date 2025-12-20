// ls.c

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

/* флаги*/
typedef struct {
    int recursive;    /*-R*/
    int long_format;  /*-l*/
    int show_group;   /*-g*/
} Flags;

/* получение имени владельца*/
char* get_owner_name(uid_t uid) {
    struct passwd *pw = getpwuid(uid);
    return pw ? pw->pw_name : "unknown";
}

/* получение имени группы*/
char* get_group_name(gid_t gid) {
    struct group *gr = getgrgid(gid);
    return gr ? gr->gr_name : "unknown";
}

/* получение строки с правами доступа*/
void get_permissions(mode_t mode, char *str) {
    /*тип файла*/
    if (S_ISDIR(mode)) str[0] = 'd';
    else if (S_ISLNK(mode)) str[0] = 'l';
    else if (S_ISCHR(mode)) str[0] = 'c';
    else if (S_ISBLK(mode)) str[0] = 'b';
    else if (S_ISFIFO(mode)) str[0] = 'p';
    else if (S_ISSOCK(mode)) str[0] = 's';
    else str[0] = '-';

    /* права пользователя*/
    str[1] = (mode & S_IRUSR) ? 'r' : '-';
    str[2] = (mode & S_IWUSR) ? 'w' : '-';
    str[3] = (mode & S_IXUSR) ? 'x' : '-';

    /* права группы*/
    str[4] = (mode & S_IRGRP) ? 'r' : '-';
    str[5] = (mode & S_IWGRP) ? 'w' : '-';
    str[6] = (mode & S_IXGRP) ? 'x' : '-';

    /* права остальных*/
    str[7] = (mode & S_IROTH) ? 'r' : '-';
    str[8] = (mode & S_IWOTH) ? 'w' : '-';
    str[9] = (mode & S_IXOTH) ? 'x' : '-';

    str[10] = '\0';
}

/* вывод информации о файле*/
void print_file_info(const char *path, const char *filename, Flags flags) {
    struct stat file_stat;
    char full_path[PATH_MAX];

    /* строим полный путь*/
    if (strcmp(path, ".") == 0) {
        strncpy(full_path, filename, sizeof(full_path) - 1);
    } else {
        snprintf(full_path, sizeof(full_path), "%s/%s", path, filename);
    }

    if (lstat(full_path, &file_stat) == -1) {
        perror("lstat");
        return;
    }

    if (flags.long_format || flags.show_group) {
        char permissions[11];
        get_permissions(file_stat.st_mode, permissions);

        printf("%s %2ld ", permissions, file_stat.st_nlink);

        /* всегда выводим имя владельца*/
        printf("%-8s ", get_owner_name(file_stat.st_uid));

        /* а группу только если указано -g*/
        if (flags.show_group) {
            printf("%-8s ", get_group_name(file_stat.st_gid));
        }

        /* выводим размер файлов*/
        if (S_ISREG(file_stat.st_mode)) {
            printf("%8ld ", file_stat.st_size);
        } else {
            printf("%8s ", "");
        }

        /* время изменения*/
        char time_buf[20];
        struct tm *timeinfo = localtime(&file_stat.st_mtime);
        strftime(time_buf, sizeof(time_buf), "%b %d %H:%M", timeinfo);
        printf("%s ", time_buf);
    }

    printf("%s", filename);

    /* обработка символических строк*/
    if (S_ISLNK(file_stat.st_mode)) {
        char link_target[PATH_MAX];
        ssize_t len = readlink(full_path, link_target, sizeof(link_target) - 1);
        if (len != -1) {
            link_target[len] = '\0';
            printf(" -> %s", link_target);
        }
    }

    printf("\n");
}

/*сравнение строк для сортировки*/
int compare_strings(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

/* вывод содержимого каталога*/
void list_directory(const char *path, Flags flags, int is_first_level) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("ls");
        return;
    }

    struct dirent *entry;
    char **entries = NULL;
    size_t entry_count = 0;
    size_t capacity = 100;

    /* выделение памяти для массива имен*/
    entries = malloc(capacity * sizeof(char *));
    if (!entries) {
        perror("malloc");
        closedir(dir);
        return;
    }

    /* читаем все записи каталогов*/
    while ((entry = readdir(dir)) != NULL) {
        /* пропускаем . и .. на первом уровне */
        if (is_first_level && (strcmp(entry->d_name, ".") == 0 ||
                               strcmp(entry->d_name, "..") == 0)) {
            continue;
        }

        if (entry->d_name[0] == '.' && strcmp(entry->d_name, ".") != 0 &&
            strcmp(entry->d_name, "..") != 0) {
            continue;
        }

        /* увеличиваем массив если нужно */
        if (entry_count >= capacity) {
            capacity *= 2;
            char **new_entries = realloc(entries, capacity * sizeof(char *));
            if (!new_entries) {
                perror("realloc");
                break;
            }
            entries = new_entries;
        }

        /* копируем имя файла*/
        entries[entry_count] = malloc(strlen(entry->d_name) + 1);
        if (entries[entry_count]) {
            strcpy(entries[entry_count], entry->d_name);
            entry_count++;
        }
    }

    closedir(dir);


    /*выводим список*/
    for (size_t i = 0; i < entry_count; i++) {
        print_file_info(path, entries[i], flags);

        /* рекурсивный обход для подкаталогов */
        if (flags.recursive) {
            char full_path[PATH_MAX];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, entries[i]);

            struct stat st;
            if (stat(full_path, &st) == 0 && S_ISDIR(st.st_mode) &&
                strcmp(entries[i], ".") != 0 && strcmp(entries[i], "..") != 0) {
                printf("\n%s:\n", full_path);
                list_directory(full_path, flags, 0);
            }
        }

        free(entries[i]);
    }

    free(entries);
}

int main(int argc, char *argv[]) {
    Flags flags = {0, 0, 0};
    char **paths = NULL;
    int path_count = 0;
    int capacity = 10;

    /* выделение памяти для массива путей*/
    paths = malloc(capacity * sizeof(char *));
    if (!paths) {
        perror("malloc");
        return 1;
    }

    /* обрабатываем аргументы командной строки */
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            /* орабатываем флаги */
            for (int j = 1; argv[i][j] != '\0'; j++) {
                switch (argv[i][j]) {
                    case 'R': flags.recursive = 1; break;
                    case 'l': flags.long_format = 1; break;
                    case 'g': flags.show_group = 1; break;
                    default:
                        fprintf(stderr, "ls: invalid option -- '%c'\n", argv[i][j]);
                        fprintf(stderr, "Try 'ls -R -l -g' for more information.\n");
                        free(paths);
                        return 1;
                }
            }
        } else {
            /* добавляем путь в массив  */
            if (path_count >= capacity) {
                capacity *= 2;
                char **new_paths = realloc(paths, capacity * sizeof(char *));
                if (!new_paths) {
                    perror("realloc");
                    break;
                }
                paths = new_paths;
            }
            paths[path_count++] = argv[i];
        }
    }

    /* если -g, то делаем подробный вывод*/
    if (flags.show_group && !flags.long_format) {
        flags.long_format = 1;
    }

    /* если путей нет, используем текущий каталог */
    if (path_count == 0) {
        list_directory(".", flags, 1);
    } else {
        /* обрабатываем каждый путь */
        for (int i = 0; i < path_count; i++) {
            struct stat path_stat;

            if (stat(paths[i], &path_stat) == 0) {
                if (S_ISDIR(path_stat.st_mode)) {
                    /* если это каталог и у нас несколько путей, выводим его имя */
                    if (path_count > 1) {
                        printf("%s:\n", paths[i]);
                    }
                    list_directory(paths[i], flags, 1);
                    if (i < path_count - 1) {
                        printf("\n");
                    }
                } else {
                    /* если это файл, просто выводим информацию о нем */
                    char *last_slash = strrchr(paths[i], '/');
                    const char *filename = last_slash ? last_slash + 1 : paths[i];
                    const char *dir_path = ".";

                    /* если путь содержит директорию, извлекаем ее */
                    if (last_slash) {
                        char dir_copy[PATH_MAX];
                        strncpy(dir_copy, paths[i], sizeof(dir_copy) - 1);
                        dir_copy[last_slash - paths[i]] = '\0';
                        dir_path = dir_copy;
                    }

                    print_file_info(dir_path, filename, flags);
                }
            } else {
                fprintf(stderr, "ls: cannot access '%s': ", paths[i]);
                perror("");
            }
        }
    }

    free(paths);
    return 0;
}
