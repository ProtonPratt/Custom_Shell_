#include "ping.h"

int sendSignalToProcess(int pid, int sig_num) {
    if (kill(pid, 0) != 0) {
        perror("\x1b[1;31m No such process found \x1b[0m\n");
        return 1;
    }

    sig_num = sig_num % 32;

    if (kill(pid, sig_num) == 0) {
        printf("Sent signal %d to process with pid %d\n", sig_num, pid);
    } else {
        perror("\x1b[1;31m Error sending signal \x1b[0m\n");
        return 1;
    }

    return 0;
}