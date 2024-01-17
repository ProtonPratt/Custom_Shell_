#include "peek.h"

int compare_file_names(const void *a, const void *b) {
    return strcmp(((struct FileInfo *)a)->name, ((struct FileInfo *)b)->name);
}

void list_directory(const char *path, int show_hidden, int show_details) {
    // printf("%s\n",path);
    DIR *dir = opendir(path);
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    struct FileInfo *file_info_array = NULL;
    size_t num_files = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (!show_hidden && entry->d_name[0] == '.') {
            continue;
        }

        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        struct stat entry_stat;
        if (lstat(full_path, &entry_stat) == 0) {
            // Allocate memory for a new FileInfo structure
            file_info_array = realloc(file_info_array, (num_files + 1) * sizeof(struct FileInfo));

            // Store file name and stat info in the struct
            strncpy(file_info_array[num_files].name, entry->d_name, PATH_MAX);
            file_info_array[num_files].stat_info = entry_stat;
            num_files++;
        } else {
            perror("lstat");
        }
    }

    closedir(dir);

    // Sort the FileInfo array
    qsort(file_info_array, num_files, sizeof(struct FileInfo), compare_file_names);

    // Print or do whatever you want with the sorted FileInfo array
    for (size_t i = 0; i < num_files; i++) {
        if (!show_hidden && file_info_array[i].name[0] == '.') {
            continue;
        }

        if (show_details || (show_hidden && show_details && file_info_array[i].name[0] == '.')) {
            printf("File name: %s\n", file_info_array[i].name);
            if (S_ISDIR(file_info_array[i].stat_info.st_mode)) {
                printf("File type: \x1B[34mDirectory\x1B[0m\n");
            } else if (S_ISREG(file_info_array[i].stat_info.st_mode)) {
                if (file_info_array[i].stat_info.st_mode & S_IXUSR) {
                    printf("File type: \x1B[32mExecutable\x1B[0m\n");
                } else {
                    printf("File type: Regular File\n");
                }
            } else {
                printf("File type: Other\n");
            }
            printf("File size: %ld bytes\n", file_info_array[i].stat_info.st_size);
            printf("File permissions: ");
            print_permission(file_info_array[i].stat_info.st_mode);
            printf("\n");

            struct passwd *owner_info = getpwuid(file_info_array[i].stat_info.st_uid);
            struct group *group_info = getgrgid(file_info_array[i].stat_info.st_gid);
            printf("Owner: %s\n", owner_info ? owner_info->pw_name : "N/A");
            printf("Group: %s\n", group_info ? group_info->gr_name : "N/A");

            char time_buffer[80];
            strftime(time_buffer, sizeof(time_buffer), "%b %d %Y %H:%M:%S", localtime(&file_info_array[i].stat_info.st_mtime));
            printf("Last modified: %s\n", time_buffer);

            printf("\n");
        } else {
            printf("%s%s\x1B[0m ",
                (S_ISDIR(file_info_array[i].stat_info.st_mode) ? "\x1B[34m" :
                 ((file_info_array[i].stat_info.st_mode & S_IXUSR) ? "\x1B[32m" : "\x1B[0m")),
                file_info_array[i].name);
            printf("\n");
        }
    }

    // Free the allocated memory for the FileInfo array
    free(file_info_array);
}