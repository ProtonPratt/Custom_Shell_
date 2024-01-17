#include "headers.h"

void warp_command(char *args[], int num_args, char* home_directory, char* prev_directory) {
    char new_directory[1024];

    char* current_directory=getcwd(NULL, 0);
    

    if (num_args <= 1 || strcmp(args[1], "~") == 0) {
        strcpy(new_directory, home_directory);
        printf("\x1b[1;32m%s\x1b[0m\n",new_directory);
    } 
    else if (strcmp(args[1], "-") == 0) {
        strcpy(new_directory, prev_directory); 
        printf("\x1b[1;32m%s\x1b[0m\n",new_directory);
    }
    else if(strcmp(args[1], "..") == 0) {
        snprintf(new_directory, sizeof(new_directory), "%s", current_directory);
        char *last_slash = strrchr(new_directory, '/');
        if (last_slash != NULL) {
            *last_slash = '\0'; 
        }

        printf("\x1b[1;32m%s\x1b[0m\n",new_directory);
        
        for (int i = 2; args[i]!= NULL; i++)
        {
            char temp_dir[1024];
            strcpy(temp_dir,new_directory);
            strcat(temp_dir,"/");
            strcat(temp_dir,args[2]);
            strcpy(new_directory,temp_dir);
            // int temp; 
            
            printf("\x1b[1;32m%s\x1b[0m\n",new_directory);
        }
    }
    else {
        if (args[1][0] == '~') {
            strcpy(new_directory, home_directory);
            strcat(new_directory, args[1] + 1); 
            printf("\x1b[1;32m%s\x1b[0m\n",new_directory);
        } else {
            strcpy(new_directory,current_directory);
            char temp_dir[1024];
            strcpy(temp_dir,new_directory);
            strcat(temp_dir,"/");
            strcat(temp_dir,args[1]);
            strcpy(new_directory,temp_dir);
            printf("\x1b[1;32m%s\x1b[0m\n",new_directory);
        }
    }

    char *prev_temp;
    prev_temp=getcwd(NULL,0);
    strcpy(prev_directory,prev_temp);
    if (chdir(new_directory) != 0) {
        perror("chdir");
    } 
}
