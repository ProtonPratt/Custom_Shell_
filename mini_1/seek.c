#include "headers.h"

char *get_relative_path(const char *full_path) {
    char *current_dir;
    // if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
    //     perror("getcwd");
    //     return NULL;
    // }
    current_dir=getcwd(NULL,0);

    size_t current_dir_len = strlen(current_dir);
    size_t full_path_len = strlen(full_path);

    if (strlen(current_dir)<strlen(full_path)) {
        // Skip the common prefix
        return strstr(full_path,current_dir)+strlen(current_dir);
    } else {
        // Return the full path if there's no common prefix
        return strdup(full_path);
    }
}

char* warp_peek(char* input,char* home_directory, char* prev_directory){
    // char new_directory[1024];
    char* new_directory=(char*)malloc(1024*sizeof(char));
    char* current_directory=getcwd(NULL, 0);

    if (strstr(input, "~") != NULL && *(strstr(input, "~")+1)!='\0') {
        strcpy(new_directory, home_directory);
        strcat(new_directory,(strstr(input,"~")+1));
        // printf("%s\n",new_directory);
        return new_directory;
    } 
    else if (strstr(input, "-") != NULL) {
        strcpy(new_directory, prev_directory); 
        // printf("%s\n",new_directory);
        return new_directory;
    }
    else if(strstr(input, "..") != NULL) {
        snprintf(new_directory, 1024, "%s", current_directory);
        char *last_slash = strrchr(new_directory, '/');
        if (last_slash != NULL) {
            *last_slash = '\0'; // Remove the last directory component
        }
        // printf("instrs--%s\n",new_directory);
        char cmptemp=*(strstr(input,"..")+3);
        if(cmptemp!='\0' && cmptemp!='/'){
                char temp_dir[1024];
                strcpy(temp_dir,new_directory);
                strcat(temp_dir,"/");
                strcat(temp_dir,(strstr(input,"..")+3));
                strcpy(new_directory,temp_dir);
                printf("%s\n",new_directory);
        }

        return new_directory;
    }
    else {
        if (strstr(input, "~") != NULL) {
            strcpy(new_directory, home_directory);
            strcat(new_directory, strstr(input,"~")+1); // Exclude the ~ character
            printf("%s\n",new_directory);
            return new_directory;
        } else {
            // strcpy(new_directory,current_directory);
            // char temp_dir[1024];
            // strcat(temp_dir,"/");
            // strcat(temp_dir,args[1]);
            // strcpy(new_directory,temp_dir);
            // printf("%s\n",new_directory);
            return current_directory;
        }
    }
}

void search_directory(const char *path, const char *target_filename, int flag_d, int flag_f, int flag_e,char* currpath) {
    DIR *dir;
    struct dirent *entry;
    int match_found=0;

    if ((dir = opendir(path)) == NULL) {
        // perror("opendir");
        // printf("%s\n",path);
        return ;
    }

    // char* currpath=getcwd(NULL,0);
    // printf("%s\n",currpath);

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char full_path[1024];
        char* filename = (char*)malloc(1024);
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        strcpy(filename, entry->d_name);

        char* temp = strstr(filename, ".");
        if (temp != NULL) { 
            *temp = '\0';
        }

        if (entry->d_type == DT_DIR /*&& flag_d && strcmp(filename, target_filename) == 0*/) {
            if(flag_d && strcmp(filename, target_filename) == 0){
                const char *relativePath = strstr(full_path, currpath);
                if (relativePath != NULL && relativePath == full_path) {
                    printf("%s./%s%s\n", "\x1b[34m", (full_path + strlen(currpath) + 1),"\x1b[0m");
                } else {
                    printf("%s%s%s\n", "\x1b[34m", full_path,"\x1b[0m");
                }

                match_found=1;
                if (flag_e) {
                    global_dirCount++;
                    if(global_dirCount==1)
                        global_dir=strdup(full_path);
                    // printf("This is a new folder!\n"); // Simulated content of newfolder.txt
                }
            }
            search_directory(full_path, target_filename, flag_d, flag_f, flag_e, currpath);
        } else if (entry->d_type == DT_REG && flag_f && strcmp(filename, target_filename) == 0) {
            const char *relativePath = strstr(full_path, currpath);
            if (relativePath != NULL && relativePath == full_path) {
                printf("%s./%s%s\n", "\x1b[32m", (full_path + strlen(currpath) + 1),"\x1b[0m");
            } else {
                printf("%s%s%s\n", "\x1b[32m", full_path,"\x1b[0m");
            }

            match_found=1;
            if (flag_e) {
                if (access(full_path, R_OK) == 0) {
                    // printf("This is a new folder!\n"); // Simulated content of newfolder.txt
                    global_dirCount++;
                    if(global_dirCount==1)
                        global_dir=strdup(full_path);
                } else {
                    printf("Missing permissions for task!\n");
                }
            }
        }
    }
    
    closedir(dir);

}