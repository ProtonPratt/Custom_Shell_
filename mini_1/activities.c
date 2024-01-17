#include "activities.h"

int cmp_activity(const void *a, const void *b) {
    return ((struct Activity *)a)->pid - ((struct Activity *)b)->pid;
}

void readAndPrintSortedActivities() {
    FILE *file = fopen("activities.txt", "r");
    if (!file) {
        perror("\x1b[1;31m fopen \x1b[0m\n");
        return;
    }

    // Read and store the data in an array of Activity structures
    struct Activity activities[1024];
    int activity_count = 0;

    char line[256];
    while (fgets(line, sizeof(line), file) != NULL) {
        int current_pid;
        char name[256];
        char state_str[256];
        if (sscanf(line, "%d : %s - %s", &current_pid, name, state_str) == 3) {
            activities[activity_count].pid = current_pid;
            strcpy(activities[activity_count].name, name);
            strcpy(activities[activity_count].state_str, state_str);
            activity_count++;
        }
    }

    fclose(file);

    // Sort the activities based on pid
    qsort(activities, activity_count, sizeof(struct Activity), cmp_activity);

    // Print the sorted activities
    for (int i = 0; i < activity_count; i++) {
        printf("%d : %s - %s\n", activities[i].pid, activities[i].name, activities[i].state_str);
    }

    FILE* act_output_file=fopen("activities.txt","w");
    fclose(act_output_file);
}


void printActivities(int target_pid) {
    printf("Parent PID (current process) %d\n",getpid());
    // Open the /proc directory
    DIR *proc_dir = opendir("/proc");
    if (proc_dir == NULL) {
        perror("\x1b[1;31m opendir \x1b[0m\n");
        return;
    }

    int ppid;

    struct dirent *entry;
    while ((entry = readdir(proc_dir))) {
        // Check if the entry is a directory and its name consists of digits (process directory)
        if (entry->d_type == DT_DIR && isdigit(entry->d_name[0])) {
            char proc_path[512]; // Increase buffer size
            snprintf(proc_path, sizeof(proc_path), "/proc/%s/cmdline", entry->d_name);

            char stat_path[256];
            int pid=atoi(entry->d_name);
            snprintf(stat_path, sizeof(stat_path), "/proc/%d/stat", pid);

            if(pid>0){
                FILE *stat_file = fopen(stat_path, "r");
                if (stat_file) {
                    int current_pid;
                    char comm[256];
                    char state;
                    int parent_pid;

                    // Read the fields from the stat file
                    if (fscanf(stat_file, "%d %s %c %d", &current_pid, comm, &state, &parent_pid) == 4) {
                        if (parent_pid == target_pid) {
                            ppid = parent_pid;

                            char proc_path_inif[256];
                            snprintf(proc_path_inif, sizeof(proc_path_inif), "/proc/%d", current_pid);
                            // printf("%dcurrrr__\n",current_pid);
                            FILE *stat_file_1;
                            char stat_path[1024];
                            snprintf(stat_path, sizeof(stat_path), "%s/stat", proc_path_inif);

                            stat_file_1 = fopen(stat_path, "r");
                            if (!stat_file_1) {
                                perror("\x1b[1;31m fopen \x1b[0m\n");
                                return;
                            }

                            char name[256];
                            char state;
                            fscanf(stat_file_1, "%*d (%[^)]) %c", name, &state);

                            //GPT<
                            const char *state_str;
                            switch (state) {
                                case 'R':
                                    state_str = "Running";
                                    break;
                                case 'S':
                                    state_str = "Sleeping";
                                    break;
                                case 'D':
                                    state_str = "Disk Sleep";
                                    break;
                                case 'T':
                                    state_str = "Stopped";
                                    break;
                                case 'Z':
                                    state_str = "Zombie"; //Zombie
                                    break;
                                default:
                                    state_str = "Unknown";
                                    break;
                            }
                            //>

                            FILE *file = fopen("child.txt", "r");
                            if (file != NULL) {
                                char line[256];
                                while (fgets(line, sizeof(line), file) != NULL) {
                                    pid_t pid_read;
                                    // char *command;
                                    char temp[256];
                                    if (sscanf(line, "[%d] %s", &pid_read, temp) == 2) {
                                        if(pid_read==current_pid){
                                            strcpy(name,temp);
                                        }
                                    }
                                }
                                fclose(file);

                            
                            FILE* act_output_file=fopen("activities.txt","a");
                            fprintf(act_output_file, "%d : %s - %s\n", current_pid, name, state_str);
                            fclose(act_output_file);

                            fclose(stat_file_1);
                            }
                        }
                    }
                }
            }
            
        }
    }
    closedir(proc_dir);
    readAndPrintSortedActivities();
}