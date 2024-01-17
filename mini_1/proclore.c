#include "headers.h"

void print_permission(mode_t mode) {
    printf((mode & S_IRUSR) ? "r" : "-");
    printf((mode & S_IWUSR) ? "w" : "-");
    printf((mode & S_IXUSR) ? "x" : "-");
    printf((mode & S_IRGRP) ? "r" : "-");
    printf((mode & S_IWGRP) ? "w" : "-");
    printf((mode & S_IXGRP) ? "x" : "-");
    printf((mode & S_IROTH) ? "r" : "-");
    printf((mode & S_IWOTH) ? "w" : "-");
    printf((mode & S_IXOTH) ? "x" : "-");
}

void get_process_information(int pid) {
    // Construct the file path for process status
    char status_file_path[128];
    snprintf(status_file_path, sizeof(status_file_path), "/proc/%d/status", pid);

    // Open the process status file for reading
    FILE *status_file = fopen(status_file_path, "r");
    if (status_file == NULL) {
        perror("Error opening process status file");
        return;
    }

    // Variables to store process information
    int processGroup = -1;
    long long int virtualMemory = -1;
    char executablePath[256];
    char processStatus = 'U'; // 'U' for Unknown
    
    // Read process status information
    // char formatted_vm[20];
    // format_virtual_memory(virtualMemory, formatted_vm, sizeof(formatted_vm));

    char line[256];
    while (fgets(line, sizeof(line), status_file)) {
        if (sscanf(line, "State:\t%c", &processStatus) == 1) {
            // Process status found
        } else if (sscanf(line, "VmSize:\t%lld kB", &virtualMemory) == 1) {
            // Virtual memory found
        } else if (sscanf(line, "PPid:\t%d", &processGroup) == 1) {
            // Parent process ID (process group) found
        }
    }
    
    // Close the status file
    fclose(status_file);

    // Construct the file path for executable information
    char exe_file_path[128];
    snprintf(exe_file_path, sizeof(exe_file_path), "/proc/%d/exe", pid);

    // Read the symbolic link pointing to the executable path
    ssize_t len = readlink(exe_file_path, executablePath, sizeof(executablePath) - 1);
    if (len != -1) {
        executablePath[len] = '\0';
    }

    int terminalForegroundProcessGroup = tcgetpgrp(STDIN_FILENO);

    // Get the process group ID of the provided PID
    // int processGroup = getpgid(pid);

    // Display the gathered information
    printf("\x1b[1;36m pid : %d \x1b[0m\n", pid);
    printf("\x1b[1;36m process status : %c%s \x1b[0m\n", processStatus, (terminalForegroundProcessGroup == getpgid(pid)) ? "+" : "");
    printf("\x1b[1;36m Process Group : %d \x1b[0m\n", processGroup);
    // printf("Virtual memory : %s\n", formatted_vm);
    printf("\x1b[1;36m Virtual memory : %lld kB \x1b[0m\n", virtualMemory);
    printf("\x1b[1;36m Executable path : %s \x1b[0m\n", executablePath);
}