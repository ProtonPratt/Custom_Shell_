#ifndef HEADERS_H_
#define HEADERS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <fcntl.h>
#include "ctype.h"
#include <errno.h>
#include <signal.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <termios.h>
#include <sys/select.h>
#include <time.h>
#include <sys/time.h>

#define MAX_PAST_EVENTS 15
#define MAX_PAST_EVENT_LENGTH 256
#define MAX_PATH_LENGTH 4096
#define MAX_LINE_LENGTH 1024
#define MAX_RESULT_COUNT 100

struct PastEvent {
    char command[MAX_PAST_EVENT_LENGTH];
};

struct PastEventStorage {
    struct PastEvent events[MAX_PAST_EVENTS];
    int numEvents;
};

struct FileInfo {
    char name[PATH_MAX];
    struct stat stat_info;
};

struct Activity {
        int pid;
        char name[256];
        char state_str[256];
};

extern int global_dirCount;
extern char* global_dir;
extern struct PastEventStorage *storage_global;
extern volatile sig_atomic_t global_foregorund_process;
extern struct termios orig_termios;

#endif