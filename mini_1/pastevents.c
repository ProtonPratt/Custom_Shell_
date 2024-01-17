#include "pastevents.h"

void savePastEvents(const struct PastEventStorage *storage) {
    FILE *file = fopen("pastevents.txt", "a");
    if (file == NULL) {
        FILE *file = fopen("pastevents.txt", "w");
        if (file == NULL) {
            perror("sbvilhrbvhs");
            return;
        }
    }
    int temp=storage->numEvents;
    if (storage->numEvents >= MAX_PAST_EVENTS) {
        fclose(file);
        FILE *file = fopen("pastevents.txt", "w");
        for (int i = 0; i < MAX_PAST_EVENTS; i++) {
            // strcpy(storage->events[i].command, storage->events[i + 1].command);
            fprintf(file, "%s\n", storage->events[i].command);
        }
        temp--;
    }
    else if(storage->numEvents>0){
        fprintf(file, "%s\n", storage->events[storage->numEvents-1].command);
    }


    fclose(file);
}

void loadPastEvents(struct PastEventStorage *storage) {
    FILE *file = fopen("pastevents.txt", "r");
    if (file == NULL) {
        FILE *file = fopen("pastevents.txt", "w+");
        if (file == NULL) {
            perror("sbvilhrbvhs");
            return;
        }
        // perror("Failed to open pastevents.txt for reading");
        return;
    }

    char line[MAX_PAST_EVENT_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL && storage->numEvents < MAX_PAST_EVENTS) {
        line[strcspn(line, "\n")] = '\0'; // Remove trailing newline
        strcpy(storage->events[storage->numEvents].command, line);
        storage->numEvents++;
    }

    fclose(file);
}

void addPastEvent(struct PastEventStorage *storage, const char *command) {
    //immidiatly non-repeating command
    if(command[0]=='\0')
        return;
    char temp_com[1024];
    strcpy(temp_com,command);
    if(strcmp(storage->events[storage->numEvents-1].command,command)==0)
        return;

    if (storage->numEvents >= MAX_PAST_EVENTS) {
        for (int i = 0; i < MAX_PAST_EVENTS - 1; i++) {
            strcpy(storage->events[i].command, storage->events[i + 1].command);
        }
        storage->numEvents--;
    }

    strcpy(storage->events[storage->numEvents].command, temp_com);
    storage->numEvents++;
}

void listPastEvents(const struct PastEventStorage *storage) {
    for (int i = 0; i < storage->numEvents; i++) {
        printf("%d: %s\n", i + 1, storage->events[i].command);
    }
}