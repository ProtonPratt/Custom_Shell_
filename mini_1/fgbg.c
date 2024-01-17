#include "fgbg.h"

void resumeProcess(pid_t pid) {
    if (pid > 0) {
        // Send SIGCONT to the specified process to resume it
        if (kill(pid, SIGCONT) == -1) {
            perror("\x1b[1;31m There is no such process \x1b[0m\n");
        } else {
            // int status;
            global_foregorund_process=pid;
            for(int i=0;i<1000;i++){}
            usleep(100000); 
            
            if (kill((pid+1), SIGCONT) == -1) {
                // perror("There is no such process");
            } 
            global_foregorund_process=0;
            
        }
    }
    else{
        printf("\x1b[1;31mpid is invalid\n\x1b[0m\n");
    }
}

void resumeProcesstoforeground(pid_t pid) {
    int result=0;
    if (pid > 0) {
        
        // Send SIGCONT to the specified process to resume it
        if (kill(pid, SIGCONT) == -1) {
            perror("\x1b[1;31m There is no such process \x1b[0m\n");
        } else {
            int status;

            for(int i=0;i<1000;i++){}
            usleep(100000);

            global_foregorund_process=pid;

            if (kill((pid+1), SIGCONT) == -1) {
                // perror("There is no such process");
            } 
            else {
                // wait(NULL);
                while(result==0){
                    result = waitpid(pid+1, &status, WUNTRACED | WNOHANG);

                    if (result == pid+1) {
                        // Process has terminated or stopped
                        if (WIFEXITED(status)) {
                            printf("\x1b[1;32mProcess %d executed successfully.\n\x1b[0m\n", pid+1);
                        } else if (WIFSIGNALED(status)) {
                            printf("\x1b[1;32mProcess %d was terminated by signal %d.\n\x1b[0m\n", pid+1, WTERMSIG(status));
                        } else if (WIFSTOPPED(status)) {
                            printf("\x1b[1;32mProcess %d was stopped by signal %d.\n\x1b[0m\n", pid+1, WSTOPSIG(status));
                        } else {
                            printf("\x1b[1;32mProcess %d did not exit normally.\n\x1b[0m\n", pid+1);
                        }
                        return;
                    }
                }
            }
            result=0;

            while(result==0){
                result = waitpid(pid, &status, WUNTRACED | WNOHANG);

                if (result == pid) {
                    // Process has terminated or stopped
                    if (WIFEXITED(status)) {
                        printf("\x1b[1;32mProcess %d executed successfully.\n\x1b[0m\n", pid);
                    } else if (WIFSIGNALED(status)) {
                        printf("\x1b[1;32mProcess %d was terminated by signal %d.\n\x1b[0m\n", pid, WTERMSIG(status));
                    } else if (WIFSTOPPED(status)) {
                        printf("\x1b[1;32mProcess %d was stopped by signal %d.\n\x1b[0m\n", pid, WSTOPSIG(status));
                    } else {
                        printf("\x1b[1;32mProcess %d did not exit normally.\n\x1b[0m\n", pid);
                    }
                    return;
                }
            }

        }
    }
    else{
        printf("process id is invalid\n");
    }
}