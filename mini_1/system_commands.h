#ifndef __SYSTEM_H
#define __SYSTEM_H

#include "headers.h"
#include "pastevents.h"
#include "warp.h"
#include "peek.h"
#include "seek.h"
#include "proclore.h"
#include "iman.h"
#include "neonate.h"
#include "fgbg.h"
#include "activities.h"
#include "ping.h"

void exit_handler();
void ctrl_c_handler(int signal);
void check_background_processes();
void execute_command(char *command, int background, struct PastEventStorage *pastEventsStorage, char *homeDirectory, char *prev_directory);
void child_signal_handler(int signum);
void execcom(char *command, int background);

void exit_handler() ;

void ctrl_c_handler(int signal);


void zhandler(int signal);

void handle_redirection(char* input_cmd, struct PastEventStorage* pastEventsStorage,char* homeDirectory, char* prev_directory,int red_flag) ;

//assumtion it can only handle 1024 commands in pipe for more explicitly have to change the size of the command array <tokens>
void pipe_handle(char* input_cmd, struct PastEventStorage* pastEventsStorage,char* homeDirectory, char* prev_directory);

void executeCommandinforeground(const char* input_cmd) ;

void sigchld_handler(int signo);

#endif