#ifndef __PASTEVENTS_H
#define __PASTEVENTS_H

#include "headers.h"

void savePastEvents(const struct PastEventStorage *storage);
void loadPastEvents(struct PastEventStorage *storage);
void addPastEvent(struct PastEventStorage *storage, const char *command);
void listPastEvents(const struct PastEventStorage *storage);

#endif