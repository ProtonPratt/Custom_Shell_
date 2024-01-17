#ifndef __SEEK_H
#define __SEEK_H

#include "headers.h"

char *get_relative_path(const char *full_path);
char* warp_peek(char* input,char* home_directory, char* prev_directory);
void search_directory(const char *path, const char *target_filename, int flag_d, int flag_f, int flag_e,char* currpath);

#endif