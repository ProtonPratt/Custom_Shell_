#ifndef __PEEK_H
#define __PEEK_H

#include "headers.h"
#include "proclore.h"

int compare_file_names(const void *a, const void *b);

void list_directory(const char *path, int show_hidden, int show_details);

#endif