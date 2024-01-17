#ifndef __NEONATE_H
#define __NEONATE_H

#include "headers.h"

int kbhit();

void die(const char *s) ;

void disableRawMode() ;

void enableRawMode() ;

long long getMostRecentPID() ;

#endif