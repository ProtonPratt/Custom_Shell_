#include "headers.h"
#include "pastevents.c"
#include "peek.c"
#include "seek.c"
#include "system_commands.c"
#include "warp.c"
#include "proclore.c"
#include "prompt.h"
#include "iman.c"
#include "neonate.c"
#include "fgbg.c"
#include "activities.c"
#include "ping.c"

int global_dirCount;
char* global_dir;
struct PastEventStorage *storage_global;
volatile sig_atomic_t global_foregorund_process;
struct termios orig_termios;

int main() {
    signal(SIGINT, ctrl_c_handler);
    signal(SIGTSTP,zhandler);
    signal(SIGCHLD,sigchld_handler);
    atexit(exit_handler);

    struct PastEventStorage pastEventsStorage;
    pastEventsStorage.numEvents = 0;
    storage_global=&pastEventsStorage;

    loadPastEvents(&pastEventsStorage);

    // printf("\x1b[1;31m error in sending signal to the process \x1b[0m\n");

    char *currentDirectory = getcwd(NULL, 0);
    if (currentDirectory == NULL) {
        perror("\x1b[1;31m getcwd \x1b[0m\n");
        return 1;
    }
    char* homeDirectory=strdup(currentDirectory);
    char* prev_directory=(char*)malloc(1024);
    strcpy(prev_directory,currentDirectory);

    struct timeval start_time, end_time;

    char *username = getenv("USER");
    if (username == NULL) {
        fprintf(stderr, "Could not retrieve username.\n");
        return 1;
    }

    // Get the system name
    struct utsname sys_info;
    if (uname(&sys_info) != 0) {
        perror("\x1b[1;31m uname \x1b[0m\n");
        return 1;
    }

    size_t len=0;
    char* input_cmd=NULL;
    char cpy[1024];
    char cpy1[1024];
    char temp1[1024];
    char* prin;
    int flag=0;
    //initialization
    // printf("<%s@%s:~>", username,sys_info.nodename);
    printf("<%s%s@%s\x1b[0m:%s~%s>", "\x1b[36m",username,sys_info.nodename,"\x1b[33m","\x1b[0m");

    int readeof=0;

    while(1){
        
        readeof=getline(&input_cmd,&len,stdin);
        input_cmd[strlen(input_cmd)-1]='\0';

        if(readeof==-1){
            if (feof(stdin)) {
                // EOF was reached, handle it here
                savePastEvents(&pastEventsStorage);
                printf("Ctrl+D encountered. Exiting...\n");
                exit(0);
                break;  // Exit the loop when EOF is encountered
            } 
        }

        strcpy(cpy,input_cmd);
        strcpy(cpy1,input_cmd);

        // char cpy2[1024];
        if(strstr(cpy1,";")!=NULL){
            // strcpy(cpy2,cpy1);
            if(strstr(cpy1,"pastevents execute")!=NULL){
                int index;
                *(strstr(cpy1,";")+1)='\0';
                char* offset=strstr(input_cmd,"pastevents execute");
                if(sscanf(strstr(offset,"pastevents execute"), "pastevents execute %d", &index) == 1 && index >= 1 && index <= pastEventsStorage.numEvents){
                    strcat(cpy1,pastEventsStorage.events[pastEventsStorage.numEvents-index].command);
                }
            }
            // printf("%s\n",cpy2);
            addPastEvent(&pastEventsStorage, cpy1);
        }
        // printf("%s\n",cpy1);
        const char delim[]=";";
        char* saveptr;
        char *command = strtok_r(cpy1, delim, &saveptr);
        while (command != NULL && strcmp(command,cpy)!=0 && strstr(input_cmd,"&")==NULL) {
            flag=1;
            int background = 0; 
            strcpy(temp1,command);

                gettimeofday(&start_time, NULL);
                execute_command(temp1,background,&pastEventsStorage,homeDirectory,prev_directory);
                gettimeofday(&end_time, NULL);

                double elapsed_time = (end_time.tv_sec - start_time.tv_sec) +(end_time.tv_usec - start_time.tv_usec) / 1000000.0;
                if (elapsed_time >= 2.0) {
                    printf("Command ran for %.2f seconds.\n", elapsed_time);
                }
            // }
            // execcom(temp1,background);

            // printf("%s -- in ; \n",command);
            command = strtok_r(NULL, delim, &saveptr);
        }

        int andcounter=0;
        int it=0;
        while(cpy[it]!='\0'){
            if(cpy[it++]=='&'){
                andcounter++;
            }
        }
        it=0;
        // printf("%d\n",andcounter);

        //&
        const char delim1[]="&";
        char* temp;
        prin = strtok_r(cpy, delim1, &temp);
        if(prin != NULL && strcmp(prin,cpy1)!=0 && flag==0){
            addPastEvent(&pastEventsStorage, input_cmd);
        }
        while (prin != NULL && strcmp(prin,cpy1)!=0 && strstr(input_cmd,"&")!=NULL) {
            flag=1;
            int background = 1; 
            strcpy(temp1,prin);
            if(it==andcounter){
                gettimeofday(&start_time, NULL);
                execute_command(temp1,0,&pastEventsStorage,homeDirectory,prev_directory);
                gettimeofday(&end_time, NULL);

                double elapsed_time = (end_time.tv_sec - start_time.tv_sec) +(end_time.tv_usec - start_time.tv_usec) / 1000000.0;
                if (elapsed_time >= 2.0) {
                    printf("%s ran for %.2f seconds.\n", input_cmd,elapsed_time);
                }
            }
            else{
                execcom(temp1,background);
                // execcom(temp1,background);
            }
            it++;

            // printf("%s -- in & \n",prin);
            prin = strtok_r(NULL, delim1, &temp);
        }


        //all the commands below here ----


        pid_t pid;
        int err=0;
        char* warptemp;
        const char warpdelim[]=" ";
        if(strchr(input_cmd, '|') != NULL && flag!=1){
            addPastEvent(&pastEventsStorage, input_cmd);
            pipe_handle(input_cmd,&pastEventsStorage,homeDirectory,prev_directory);
        }
        else if ((strchr(input_cmd, '<') != NULL || strchr(input_cmd, '>') != NULL || strstr(input_cmd, ">>") != NULL) && flag!=1) {
            addPastEvent(&pastEventsStorage, input_cmd);
            handle_redirection(input_cmd,&pastEventsStorage,homeDirectory, prev_directory,0);

        }
        else if((strstr(input_cmd, "bg") != NULL || strstr(input_cmd, "fg") != NULL) && flag!=1){
            pid_t resume_pid=0;
            addPastEvent(&pastEventsStorage, input_cmd);
            if(strstr(input_cmd, "bg") != NULL){
                char* offset=strstr(input_cmd,"bg");
                sscanf(offset,"bg %d",&resume_pid);
                resumeProcess(resume_pid);
            }
            else if(strstr(input_cmd, "fg") != NULL){
                char* offset=strstr(input_cmd,"fg");
                sscanf(offset,"fg %d",&resume_pid);
                resumeProcesstoforeground(resume_pid);
            }
            else{
                perror("\x1b[1;31m fg bg some error \x1b[0m\n");
            }
            
        }
        else if (strstr(input_cmd, "warp") != NULL && flag!=1) {
            char *warp_args[128] = {NULL};
            int num_warp_args = 0;

            addPastEvent(&pastEventsStorage, input_cmd);

            char *warp_token = strtok_r(input_cmd, warpdelim, &warptemp);
            while (warp_token != NULL) {
                warp_args[num_warp_args++] = warp_token;
                warp_token = strtok_r(NULL, warpdelim, &warptemp);
            }

            warp_command(warp_args, num_warp_args,homeDirectory,prev_directory);

        }
        else if (strstr(input_cmd, "peek") != NULL && flag!=1) {
            // addPastEvent(&pastEventsStorage, input_cmd);
            int show_hidden = 0;
            int show_details = 0;

            addPastEvent(&pastEventsStorage, input_cmd);

            // Check for -a and -l flags
            if (strstr(input_cmd, "a") != NULL) {
                show_hidden = 1;
            }
            if (strstr(input_cmd, "l") != NULL) {
                show_details = 1;
            }
            char* path=NULL;
            // Extract the path/name after flags
            // char *path = strtok_r(NULL, delim, &saveptr);
            if(strstr(input_cmd, "~") != NULL || strstr(input_cmd, "..") != NULL || strstr(input_cmd, "-") != NULL){
                path=warp_peek(input_cmd,homeDirectory,prev_directory);
                // printf("%s\n",path);
            }
            else{
                path=strstr(input_cmd,"/");
                // printf("%s\n",path);
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
        else if (strstr(input_cmd, "pastevents") != NULL && flag!=1) {
            if (strstr(input_cmd, "pastevents purge") != NULL) {
                pastEventsStorage.numEvents = 0; // Clear past events
                char filename[] = "pastevents.txt";

                if (remove(filename) == 0) {
                    printf("File '%s' deleted successfully.\n", filename);
                } else {
                    perror("\x1b[1;31m Error deleting the file \x1b[0m\n");
                }

                // FILE *file = fopen("pastevents.txt", "w");
                // fclose(file);
            } else if (strstr(input_cmd, "pastevents execute") != NULL) {
                int index;
                char* offset=strstr(input_cmd,"pastevents execute");
                if (sscanf(offset, "pastevents execute %d", &index) == 1 && index >= 1 && index <= pastEventsStorage.numEvents) {
                    // printf("Executing past event: %s num %d\n", pastEventsStorage.events[pastEventsStorage.numEvents-index].command,index);
                    gettimeofday(&start_time, NULL);
                    execute_command(pastEventsStorage.events[pastEventsStorage.numEvents-index].command, 0, &pastEventsStorage,homeDirectory,prev_directory);
                    gettimeofday(&end_time, NULL);

                    double elapsed_time = (end_time.tv_sec - start_time.tv_sec) +(end_time.tv_usec - start_time.tv_usec) / 1000000.0;
                    if (elapsed_time >= 2.0) {
                        printf("%s ran for %.2f seconds.\n", input_cmd,elapsed_time);
                    }
                    addPastEvent(&pastEventsStorage, pastEventsStorage.events[pastEventsStorage.numEvents-index].command);
                }
            } else {
                // Store the pastevent command itself
                // addPastEvent(&pastEventsStorage, input_cmd);

                // List past events
                listPastEvents(&pastEventsStorage);
            }
        }
        else if (strstr(input_cmd, "proclore") != NULL && flag==0) {
            addPastEvent(&pastEventsStorage, input_cmd);
            char* offset=strstr(input_cmd,"proclore");
            if (sscanf(offset, "proclore %d", &pid) == 1) {
                // User provided a PID, retrieve and display process information
                get_process_information(pid);
            } else {
                // No PID provided, display shell information
                get_process_information(getpid()); // Get shell's PID
            }
        }
        else if (strstr(input_cmd, "seek") != NULL && flag==0) {
            addPastEvent(&pastEventsStorage, input_cmd);
            char *temp=strdup(input_cmd);
            char *token = strtok_r(temp, " \n", &temp);

            char *search = NULL;

            char *path=currentDirectory;
            // printf("%s---\n",path);

            if(strstr(input_cmd, "~") != NULL || strstr(input_cmd, "..") != NULL){
                path=warp_peek(input_cmd,homeDirectory,prev_directory);
            }
            else if(strstr(input_cmd,"/")!=NULL){
                char *temp;
                char *pathori=(char*)malloc(1024);
                strcpy(pathori,currentDirectory);
                temp=strstr(input_cmd,"/");
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
                // else if(strstr(token,"/")==NULL){
                    
                // }

                token = strtok_r(NULL, " \n", &temp);
            }

            if (strstr(input_cmd, "-d -f") != NULL || strstr(input_cmd, "-f -d") != NULL ) {
                    printf("Invalid flags!\n");
                    Invalid_flags=1;
                    // continue;
            }

            int match_found=0;
            if (access(path, F_OK) != 0) {
                printf("No match found!\n");
                // continue;
            }
            
            // chdir(global_dir);

            global_dirCount=0;
            if(Invalid_flags==0)
                search_directory(path, search, flag_d, flag_f, flag_e, currentDirectory);
            
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
                    perror("\x1b[1;31m Failed to open file \x1b[0m\n");
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
        else if(strstr(input_cmd,"activities")!=NULL && flag!=1){
            addPastEvent(&pastEventsStorage,input_cmd);
            printActivities(getpid());
        }
        else if(strstr(input_cmd,"ping")!=NULL && flag!=1){
            addPastEvent(&pastEventsStorage,input_cmd);
            int pid=0,sig_num=0;
            char* offset=strstr(input_cmd,"ping");
            sscanf(offset,"ping %d %d",&pid,&sig_num);

            if(sendSignalToProcess(pid,sig_num)!=0){
                perror("\x1b[1;31m error in sending signal to the process \x1b[0m\n");
            }
        }
        else if(strstr(input_cmd,"iman")!=NULL && flag!=1){
            addPastEvent(&pastEventsStorage,input_cmd);
            char man_page_name[256];
            char* offset=strstr(input_cmd,"iman");
            sscanf(offset,"iman %s",man_page_name);

            fetchiManPage(man_page_name);
        }
        else if(strstr(input_cmd,"neonate -n") && flag!=1){
            addPastEvent(&pastEventsStorage,input_cmd);
            gettimeofday(&start_time, NULL);
            int time_arg = 0;
            char* offset=strstr(input_cmd,"neonate -n");
            sscanf(offset, "neonate -n %d", &time_arg);

            if(time_arg<=0){
                printf("Invalid Time : time_arg should be integer and >0\n");
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
            gettimeofday(&end_time, NULL);
            double elapsed_time = (end_time.tv_sec - start_time.tv_sec)+(end_time.tv_usec - start_time.tv_usec) / 1000000.0;
            if (elapsed_time >= 2.0) {
                printf("%s ran for %.2f seconds.\n", input_cmd,elapsed_time);
            }

        }
        else if (flag == 0) {
            addPastEvent(&pastEventsStorage,input_cmd);
            if(strlen(input_cmd)!=0){
                gettimeofday(&start_time, NULL);
                executeCommandinforeground(input_cmd);
                gettimeofday(&end_time, NULL);
                double elapsed_time = (end_time.tv_sec - start_time.tv_sec)+(end_time.tv_usec - start_time.tv_usec) / 1000000.0;
                if (elapsed_time >= 2.0) {
                    printf("%s ran for %.2f seconds.\n", input_cmd,elapsed_time);
                }
            }

        }
        


        if(err==0 && (strstr(input_cmd,"pastevents execute")!=NULL||strstr(input_cmd,"pastevents")==NULL))
            savePastEvents(&pastEventsStorage);
        
        //background printer
        check_background_processes();

        // printf("%s\n",input_cmd);
        currentDirectory = getcwd(NULL, 0);
        
        print_prompt(username,sys_info.nodename,currentDirectory,homeDirectory);
        

        flag=0;
    }

    return 0;
}