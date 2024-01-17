#include "system_commands.h"

void exit_handler() {
    FILE *f1ile = fopen("child.txt", "w");
    fclose(f1ile);
    FILE *f1ile1 = fopen("activities.txt", "w");
    fclose(f1ile1);
}

void ctrl_c_handler(int signal) {
    
    int foreground_pid;

    if (tcgetpgrp(STDIN_FILENO) == getpgrp()) {
            foreground_pid = 0;
        } else {
            foreground_pid = getpid();
            printf("%d in foreground\n",foreground_pid);
        }
    
    // printf("%doutfore\n",foreground_pid);
    // printf("%doutfore_glove\n",global_foregorund_process);
    if (foreground_pid > 0) {
        kill(foreground_pid, SIGKILL);
    } else {
        // printf("\nNo FOREGROUND(%d) found\n",foreground_pid);
    }
}

void zhandler(int signal){

    // printf("%d-globfore\n",global_foregorund_process);

    if (global_foregorund_process > 0) {
        if(kill(global_foregorund_process, SIGTSTP)==-1){
            perror("\x1b[1;31m kill not working correct in zhandler \x1b[0m\n");
        }
        else{
            int status;

            usleep(100000);

            if(kill(global_foregorund_process+1, SIGTSTP)==-1){
                // perror("kill not working correct in zhandler");
            }
            
        }
    } 

}

void handle_redirection(char* input_cmd, struct PastEventStorage* pastEventsStorage,char* homeDirectory, char* prev_directory,int red_flag) {
    int fd_in=0, fd_out=0;
    char *input_file=NULL, *output_file=NULL;
    char *command[1024]={NULL};
    int arg_count = 0;

    int append_flag=0;

    // Read command and arguments
    char buffer[4096];
    strcpy(buffer,input_cmd);
    char *saveptr;

    int fdpipein=fileno(stdin);
    int fdpipeout=fileno(stdout);

    // printf("fdpipein %d, fdpipeout %d\n",fdpipein,fdpipeout);

    // printFileDescriptorType(0);
    
    char *token = strtok_r(buffer, " \t\n", &saveptr);
    while (token != NULL) {
        if (strcmp(token, "<") == 0) {
            // Input redirection
            token = strtok_r(NULL, " \t\n", &saveptr);
            input_file = token;
        } else if (strcmp(token, ">") == 0) {
            // Output redirection
            token = strtok_r(NULL, " \t\n", &saveptr);
            output_file = token;
        } else if (strcmp(token, ">>") == 0) {
            // Append output redirection
            append_flag=1;
            token = strtok_r(NULL, " \t\n", &saveptr);
            output_file = token;
        } else {
            // Command or argument
            command[arg_count++] = token;
        }
        token = strtok_r(NULL, " \t\n", &saveptr);
    }
    command[arg_count] = NULL;

    // Create a child process
    pid_t pid = fork();
    if (pid == -1) {
        perror("\x1b[1;31m fork \x1b[0m\n");
    } else if (pid == 0) {
        // Child process

        // Input redirection
        if (input_file != NULL) {
            fd_in = open(input_file, O_RDONLY);
            if (fd_in == -1) {
                perror("\x1b[1;31m open read_only \x1b[0m\n");

            }
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }

        // Output_redirection
        if (output_file != NULL) {
            if (append_flag == 1) {
                fd_out = open(output_file, O_WRONLY | O_CREAT | O_APPEND, 0666);
            } 
            else {
                fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            }

            if (fd_out == -1) {
                // printf("wtf dum stfu\n");
                perror("\x1b[1;31m open Output_redirection \x1b[0m\n");

            }
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }

        // Redirect stderr to /dev/null
        int dev_null_fd = open("/dev/null", O_WRONLY);
        if (dev_null_fd == -1) {
            perror("\x1b[1;31m open \x1b[0m\n");

        }
        dup2(dev_null_fd, STDERR_FILENO);
        close(dev_null_fd);

        // Execute the command using execv
        // printf("%s %s\n",command[0],command[1]);
        // printf("%s",command);

        char full_command[4096];
        full_command[0]='\0';

        for (int i = 0; i < 1024; i++)
        {
            if(command[i]!=NULL){
                strcat(full_command,command[i]);
                strcat(full_command," ");
            }
            else{
                break;
            }
        }
        
        const char *forbiddenWords[] = {"warp ", "peek ", "seek ", "proclore ", "pastevents "};
        int fl_local=0;
        for (int i = 0; i < 5; i++)
        {
            if (strstr(input_cmd, forbiddenWords[i]) != NULL) {
                fl_local=1;
                break;
            }
        }
        
        if(fl_local==0){
            char *args[] = { "/bin/sh", "-c", full_command, NULL };
            execv("/bin/sh", args);
        }
        else{
            execute_command(full_command,2,pastEventsStorage,homeDirectory,prev_directory);
        }
        
        if(red_flag==0)
            freopen("/dev/tty", "w", stdout);
        
        exit(2);
        
    } else {
        // Parent process
        wait(NULL);
        if(red_flag!=0){
        dup2(fdpipein,STDIN_FILENO);
        dup2(fdpipeout,STDOUT_FILENO);
        close(fdpipein);
        close(fdpipeout);
        }
    }

}

void pipe_handle(char* input_cmd, struct PastEventStorage* pastEventsStorage,char* homeDirectory, char* prev_directory){
    char* saveptr;
    char* cpy_input=strdup(input_cmd);
    char* token=strtok_r(cpy_input,"|",&saveptr);
    char* commands[1024];
    int num=0;
    while( token!=NULL ){
        num++;
        commands[num-1]=strdup(token);
        token=strtok_r(NULL,"|",&saveptr);
    }

    int fd[num][2];
    for (int i = 0; i < num; i++){
        pipe(fd[i]);
    }

    //main part here mf
    int redirect_fl=0;
    for (int i = 0; i < num; i++){
        pid_t pid=fork();
        if(pid<0){
            perror("\x1b[1;31m child unsuccessfull \x1b[0m\n");
        }
        else if(pid==0){
            if(i>0){
                close(fd[i-1][1]);
                dup2(fd[i-1][0],STDIN_FILENO);
                close(fd[i-1][0]);
            }
            if(i < num-1){
                close(fd[i][0]);
                dup2(fd[i][1],STDOUT_FILENO);
                close(fd[i][1]);
            }

            if(strchr(commands[i], '<') != NULL || strchr(commands[i], '>') != NULL || strstr(commands[i], ">>") != NULL){
                redirect_fl=1;
                handle_redirection(commands[i],pastEventsStorage,homeDirectory,prev_directory,1);
            }


            if(redirect_fl==0){
                char* args[]={"/bin/sh","-c",commands[i],NULL};
                execv("/bin/sh",args);
                
                perror("\x1b[1;31m execv in pipe failed \x1b[0m\n");
            }
            exit(2);
        }
        if(i>0){
            close(fd[i-1][0]);
            close(fd[i-1][1]);
        }
    }

    //free the mems
    for (int i = 0; i < num; i++){
        close(fd[i][0]);
        close(fd[i][1]);
    }

    for (int i = 0; i < num; i++){
        free(commands[i]);
    }

    for (int i = 0; i < num; i++){
        wait(NULL);
    }

}

void executeCommandinforeground(const char* input_cmd) {
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        int dev_null_fd = open("/dev/null", O_WRONLY);
        if (dev_null_fd == -1) {
            perror("open");
            
        }
        dup2(dev_null_fd, STDERR_FILENO);
        close(dev_null_fd);

        // This is the child process
        execlp("/bin/sh", "/bin/sh", "-c", input_cmd, NULL);
        // fprintf(stdout, "\x1B[31m/bin/sh: 1: %s: not found\x1B[0m\n", input_cmd);
        // If execlp returns, an error occurred
        perror("execlp");
        exit(1);
    } else {
        // printf("%d\n",pid);
        FILE *file = fopen("child.txt", "a");
            if (file != NULL) {
                fprintf(file, "[%d] %s\n", pid, input_cmd);
                fclose(file);
            } else {
                perror("File opening failed");
            }

        global_foregorund_process=pid;

        int status;
        waitpid(pid, &status, WUNTRACED);
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            fprintf(stdout, "\x1b[1;31mError: '%s' is not a valid Command\x1b[0m\n", input_cmd);
        }
        if (WIFSTOPPED(status)) {
            printf("\x1b[1;31m Foreground process (PID %d) stopped. \x1b[0m\n", pid);
        } else {
            // printf("Foreground process (PID %d) exited with status %d.\n", pid, WEXITSTATUS(status));
            global_foregorund_process = 0; // Reset the foreground process PID
        }
        global_foregorund_process = 0;
    }
}

void sigchld_handler(int signo) {
    pid_t child_pid;
    int status;

    // // Reap all terminated child processes
    // while ((child_pid = waitpid(-1, &status, WNOHANG)) > 0) {
    //     if (WIFEXITED(status)) {
    //         printf("Child process %d exited with status %d\n", child_pid, WEXITSTATUS(status));
    //     } else if (WIFSIGNALED(status)) {
    //         printf("Child process %d terminated by signal %d\n", child_pid, WTERMSIG(status));
    //     }
    // }
}

void check_background_processes()
{
    pid_t pid;
    int status;
    int carry = 0;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        if (WIFEXITED(status))
        {
            carry = pid;
        }
        else if (WIFSIGNALED(status))
        {
            printf("\x1b[1;31m Background process %d exited due to signal \x1b[0m\n", pid);
        }
    }

    FILE *file = fopen("child.txt", "r");
    if (file != NULL)
    {
        char line[256];
        while (fgets(line, sizeof(line), file) != NULL)
        {
            pid_t pid_read;

            char temp[256];
            if (sscanf(line, "[%d] %s", &pid_read, temp) == 2)
            {
                if (pid_read == carry)
                {

                    printf("\x1b[1;31m %s exited normally (%d)\n \x1b[0m\n", temp, carry);
                }
            }
        }
        fclose(file);
    }
}

void execute_command(char *command, int background, struct PastEventStorage* pastEventsStorage, char* homeDirectory,char* prev_directory ) {
    int pid_1=0;
    int flag=0;
    int pid_proclore=0;

    struct timeval start_time, end_time;

    // printf("in excec\n");

    // pid_t pid = fork();
    if (pid_1 < 0) {
        perror("Fork failed");
    } else if (pid_1 == 0) {
        char cpy1[1024];
        strcpy(cpy1,command);

        const char delim[]=";";
        char* saveptr;
        char *comm = strtok_r(cpy1, delim, &saveptr);
        while (comm != NULL && strcmp(comm,command)!=0 && strstr(command,";")!=NULL) {
            flag=1;
            int background = 0;
            char temp1[1024]; 
            strcpy(temp1,comm);

            // addPastEvent(&pastEventsStorage, cpy1);
            gettimeofday(&start_time, NULL);
            execute_command(temp1,background,pastEventsStorage,homeDirectory,prev_directory);
            gettimeofday(&end_time, NULL);
            double elapsed_time = (end_time.tv_sec - start_time.tv_sec) +(end_time.tv_usec - start_time.tv_usec) / 1000000.0;
            if (elapsed_time >= 2.0) {
                printf("\x1b[1;31m %s ran for %.2f seconds. \x1b[0m\n", temp1,elapsed_time);
            }
            // execcom(temp1,background);

            // printf("%s -- in ; \n",command);
            comm = strtok_r(NULL, delim, &saveptr);
        }
        
        char* warptemp;
        const char warpdelim[]=" ";
        if(strchr(command, '|') != NULL && flag!=1){
            // addPastEvent(pastEventsStorage, command);
            pipe_handle(command,pastEventsStorage,homeDirectory,prev_directory);
        }
        else if ((strchr(command, '<') != NULL || strchr(command, '>') != NULL || strstr(command, ">>") != NULL) && flag!=1) {
            // addPastEvent(pastEventsStorage, command);
            handle_redirection(command,pastEventsStorage,homeDirectory, prev_directory,0);

        }
        else if((strstr(command, "bg") != NULL || strstr(command, "fg") != NULL) && flag!=1){
            pid_t resume_pid=0;
            if(strstr(command, "bg") != NULL){
                char* offset=strstr(command,"fg");
                sscanf(offset,"bg %d",&resume_pid);
                resumeProcess(resume_pid);
            }
            else if(strstr(command, "fg") != NULL){
                char* offset=strstr(command,"fg");
                sscanf(offset,"fg %d",&resume_pid);
                resumeProcesstoforeground(resume_pid);
            }
            else{
                perror("\x1b[1;31m fg bg some error \x1b[0m\n");
            }
            
        }
        else if (strstr(command, "warp") != NULL && flag==0) {
            char *warp_args[128] = {NULL};
            int num_warp_args = 0;

            // addPastEvent(pastEventsStorage, command);

            char *warp_token = strtok_r(command, warpdelim, &warptemp);
            while (warp_token != NULL) {
                warp_args[num_warp_args++] = warp_token;
                warp_token = strtok_r(NULL, warpdelim, &warptemp);
            }

            warp_command(warp_args, num_warp_args,homeDirectory,prev_directory);

        }
        else if (strstr(command, "peek") != NULL && flag==0) {
            int show_hidden = 0;
            int show_details = 0;

            // addPastEvent(pastEventsStorage, command);

            // Check for -a and -l flags
            if (strstr(command, "a") != NULL) {
                show_hidden = 1;
            }
            if (strstr(command, "l") != NULL) {
                show_details = 1;
            }
            char* path=NULL;
            // Extract the path/name after flags
            // char *path = strtok_r(NULL, delim, &saveptr);
            if(strstr(command, "~") != NULL || strstr(command, "..") != NULL || strstr(command, "-") != NULL){
                path=warp_peek(command,homeDirectory,prev_directory);
            }
            else{
                path=strstr(command,"/");
            }
            //

            if (path == NULL) {
                // path=warp_peek(input_cmd,homeDirectory,prev_directory);
                path = getcwd(NULL, 0); // Peek at current working directory if no path provided
            }
            // printf("%s\n",path);

            list_directory(path, show_hidden, show_details);

            // if (path != NULL && path != getcwd(NULL, 0)) {
            //     free(path);
            // }

            // ... (other code)
        }
        else if (strstr(command, "pastevents") != NULL && flag==0) {
            if (strstr(command, "pastevents purge") != NULL) {
                pastEventsStorage->numEvents = 0; // Clear past events
                char filename[] = "pastevents.txt";

                if (remove(filename) == 0) {
                    printf("\x1b[1;31m File '%s' deleted successfully. \x1b[0m\n", filename);
                } else {
                    perror("Error deleting the file");
                }

                // FILE *file = fopen("pastevents.txt", "w");
                // fclose(file);
            } else if (strstr(command, "pastevents execute") != NULL) {
                int index;
                char* offset=strstr(command,"pastevents execute");
                if (sscanf(offset, "pastevents execute %d", &index) == 1 && index >= 1 && index <= pastEventsStorage->numEvents) {
                    printf("Executing past event: %s\n", pastEventsStorage->events[index-1].command);
                    // addPastEvent(pastEventsStorage, pastEventsStorage->events[index-1].command);
                    gettimeofday(&start_time, NULL);
                    execute_command(pastEventsStorage->events[index-1].command, 0, pastEventsStorage,homeDirectory,prev_directory);
                    gettimeofday(&end_time, NULL);
                    double elapsed_time = (end_time.tv_sec - start_time.tv_sec) +(end_time.tv_usec - start_time.tv_usec) / 1000000.0;
                    if (elapsed_time >= 2.0) {
                        printf("\x1b[1;31m %s ran for %.2f seconds. \x1b[0m\n", pastEventsStorage->events[index-1].command,elapsed_time);
                    }
                }
            } else {
                listPastEvents(pastEventsStorage);
            }
        }
        else if (strstr(command, "proclore") != NULL && flag==0) {
            char* offset=strstr(command,"proclore");
            if (sscanf(offset, "proclore %d", &pid_proclore) == 1) {
                // User provided a PID, retrieve and display process information
                get_process_information(pid_proclore);
            } else {
                // No PID provided, display shell information
                get_process_information(getpid()); // Get shell's PID
            }
        }
        else if (strstr(command, "seek") != NULL && flag==0) {
            char *temp=strdup(command);
            char *token = strtok_r(temp, " \n", &temp);

            char *search = NULL;

            char *path=getcwd(NULL,0);
            // printf("%s---\n",path);

            if(strstr(command, "~") != NULL || strstr(command, "..") != NULL){
                path=warp_peek(command,homeDirectory,prev_directory);
            }
            else if(strstr(command,"/")!=NULL){
                char *temp;
                char *pathori=(char*)malloc(1024);
                strcpy(pathori,path);
                temp=strstr(command,"/");
                strcat(pathori,temp);
                path=pathori;
            }
            // if(path=NULL){
            //     path=warp_peek(input_cmd,homeDirectory,prev_directory);
            // }
            // printf("%s------\n",path);
            int flag_d = 1;
            int flag_f = 1;
            int flag_e = 0;
            int Invalid_flags=0;

            while (token != NULL) {
                if (strcmp(token, "-d") == 0) {
                    flag_d = 1;
                    flag_f = 0;
                } else if (strcmp(token, "-f") == 0) {
                    flag_f = 1;
                    flag_d = 0;
                } else if (strcmp(token, "-e") == 0) {
                    flag_e = 1;
                }
                else if(strstr(token,"/")==NULL){
                    search = token;
                    // printf("%s\n",search);
                }

                token = strtok_r(NULL, " \n", &temp);
            }

            if (strstr(command, "-d -f") != NULL || strstr(command, "-f -d") != NULL) {
                    printf("\x1b[1;31m Invalid flags! \x1b[0m\n");
                    Invalid_flags=1;
                    // continue;
            }

            int match_found=0;
            if (access(path, F_OK) != 0) {
                printf("\x1b[1;31m No match found! \x1b[0m\n");
                // continue;
            }
            
            // chdir(global_dir);

            char* curr_dirr=getcwd(NULL,0);

            global_dirCount=0;
            if(Invalid_flags==0)
                search_directory(path, search, flag_d, flag_f, flag_e, curr_dirr);
            // if(match_found==0)
            //     printf("No match found!\n");

            // printf("%d-global\n",global_dirCount);
            if(global_dirCount==1 && flag_d==1){
                // printf("%s-global\n",global_dir);
                chdir(global_dir);
                free(global_dir);
            }
            else if(global_dirCount==1 && flag_f==1){
                char line[256];
                FILE *file = fopen(global_dir, "r");
                if (file == NULL) {
                    perror("Failed to open file");
                    // return 1;
                }
                while (fgets(line, sizeof(line), file)) {
                    printf("%s", line);
                }
                fclose(file);
            }
            flag_d = 0;
            flag_f = 0;
        }
        else if(strstr(command,"activities")!=NULL && flag!=1){
            // addPastEvent(&pastEventsStorage,command);
            printActivities(getpid());
        }
        else if(strstr(command,"ping")!=NULL && flag!=1){
            // addPastEvent(&pastEventsStorage,command);
            int pid=0,sig_num=0;
            char* offset=strstr(command,"ping");
            sscanf(command,"ping %d %d",&pid,&sig_num);

            if(sendSignalToProcess(pid,sig_num)!=0){
                perror("\x1b[1;31m error in sending signal to the process \x1b[0m\n");
            }
        }
        else if(strstr(command,"iman")!=NULL && flag!=1){
            // addPastEvent(&pastEventsStorage,command);
            char man_page_name[256];
            char* offset=strstr(command,"iman");
            sscanf(command,"iman %s",man_page_name);

            fetchiManPage(man_page_name);
        }
        else if(strstr(command,"neonate -n") && flag!=1){
            int time_arg = 0;
            char* offset=strstr(command,"neonate -n");
            sscanf(offset, "neonate -n %d", &time_arg);

            if(time_arg<=0){
                printf("\x1b[1;31m Invalid Time : time_arg should be integer and >0 \x1b[0m\n");
            }
            else{
                int current_pid;
                setbuf(stdout, NULL);
                enableRawMode();

                int brk=0;
                pid_t neo_pid=fork();
                
                if(neo_pid==0){
                    
                    while (1) {
                        current_pid = getMostRecentPID();
                        printf("%d\n", current_pid);

                        int delay_counter = 0;
                        while (delay_counter < time_arg*10) {
                            if (kbhit()) {
                                char key = getchar();
                                if (key == 'x') {
                                    disableRawMode();
                                    brk=1;
                                    break;
                                }
                            }
                            usleep(100000);
                            delay_counter++;
                        }
                        if(brk==1)
                            break;
                    }
                    exit(2);
                }
                else{
                    global_foregorund_process=neo_pid;
                    wait(NULL);
                    global_foregorund_process = 0;
                }
            }

        }
        else if (flag==0){
            // char command_with_redirect[256];
            // snprintf(command_with_redirect, sizeof(command_with_redirect), "%s 2>/dev/null", command);
            // int err=system(command_with_redirect);
            // if(err!=0){
            //     printf("ERROR : '%s' is not a valid command\n",command);
            //     //ERROR : 'sleeeep' is not a valid command
            //     // perror("system");
            // }
            pid_t child_pid = fork(); // Fork a new process

            if (child_pid == -1) {
                perror("fork"); // Error occurred
            } else if (child_pid == 0) {
                // This is the child process

                // Redirect stderr to /dev/null
                int dev_null_fd = open("/dev/null", O_WRONLY);
                if (dev_null_fd == -1) {
                    perror("open");
                    
                }
                dup2(dev_null_fd, STDERR_FILENO);
                close(dev_null_fd);

                char *args[] = { "/bin/sh", "-c", command, NULL };
                execv("/bin/sh", args); // Replace the child process with the new command

                // If execv fails, the following code will be executed
                perror("execv"); // Print an error message
                // Terminate the child process
            } else {
                // This is the parent process
                int status;
                waitpid(child_pid, &status, 0); // Wait for the child process to complete
                if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                    // addPastEvent(&pastEventsStorage, command);
                } else {
                    printf("\x1b[1;31m ERROR: '%s' is not a valid command \x1b[0m\n", command);
                }
            }
        }
        // exit();
    } 
    else{
        wait(NULL);
    }

}


void child_signal_handler(int signum)
{
    return;
}


void execcom(char *command, int background){
    // pid_t pid;

    signal(SIGUSR1, child_signal_handler);

    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
    } 
    else if (pid == 0) {
        int dev_null_fd = open("/dev/null", O_WRONLY);
        if (dev_null_fd == -1) {
            perror("open");
            
        }
        dup2(dev_null_fd, STDERR_FILENO);
        close(dev_null_fd);
       
        
        char *args[] = { "/bin/sh", "-c", command, NULL };
        execvp("/bin/sh", args);

        // If execvp fails, the following code will be executed
        perror("execvp"); // Print an error message
        printf("\x1b[1;31m ERROR: '%s' is not a valid command\n \x1b[0m\n", command);

        kill(getppid(), SIGUSR1);

        exit(0);
    } else {
        // printf("somehitng\n");
        FILE *file = fopen("child.txt", "a");
            if (file != NULL) {
                fprintf(file, "[%d] %s\n", pid, command);
                fclose(file); // Close the file after writing
            } else {
                perror("File opening failed");
            }
        if (background!=1) {
            int status;
            waitpid(pid, &status, 0); // Wait for child to finish
            if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
                fprintf(stdout, "\x1B[31mError: '%s' is not a valid Command\x1B[0m\n", command);
            }
            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                printf("\x1b[1;31m%s exited normally (%d)\n\x1b[0m\n", command, pid);
            }
        }  
    }
}

