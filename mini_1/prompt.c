#include "prompt.h"

void print_prompt(char* usrname, char* sysname,char* curdir, char* homedir){
    char* subtempchar=strstr(curdir,homedir);
    if(strcmp(homedir,curdir)==0){
        printf("<%s%s@%s\x1b[0m:%s~%s>", "\x1b[36m",usrname,sysname,"\x1b[33m","\x1b[0m");
    }
    else if(subtempchar){
        subtempchar+=strlen(homedir);
        printf("<\x1b[36m%s@%s\x1b[0m:\x1b[33m~%s\x1b[0m>", usrname,sysname,subtempchar);
    }
    else{
        printf("<\x1b[36m%s@%s\x1b[0m:\x1b[33m%s\x1b[0m>", usrname,sysname,curdir);
    }

}