#include "neonate.h"

void die(const char *s) {
    perror(s);
    exit(1);
}

int kbhit() {
    struct timeval tv = {0L, 0L};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
}

void disableRawMode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
        die("tcsetattr");
}

void enableRawMode() {
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");
    atexit(disableRawMode);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

long long getMostRecentPID() {
    struct dirent **namelist;
    int n;
    
    n = scandir("/proc", &namelist, NULL, alphasort);
    
    if (n == -1) {
        die("scandir");
    }

    long long max_pid = -1;

    for (int i = 0; i < n; i++) {
        if (isdigit(namelist[i]->d_name[0])) {
            long long pid = atoll(namelist[i]->d_name);
            if (pid > max_pid) {
                max_pid = pid;
            }
        }
        free(namelist[i]);
    }

    free(namelist);
    return max_pid;
}