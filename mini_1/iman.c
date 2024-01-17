#include "iman.h"

void printError(const char *message) {
    perror(message);
}

void printimanpage(char *html, int fl) {
    char *name = "NAME\n";
    char *author_descp = "AUTHOR";

    char *pos = strstr(html, name);
    if (pos == NULL && fl == 0) {
        printf("\x1b[1;31mERROR\n\x1b[0m\n");
        printf("        \x1b[1;31mNo such command\n\x1b[0m\n");
        return;
    }

    char *end = strstr(html, author_descp);
    long long int length = end - pos;
    if (pos != NULL)
        printf("%.*s\n", (int)length, pos);
}

void fetchiManPage(const char *command_name) {
    //Resolve DNS
    struct hostent *host = gethostbyname("man.he.net");
    if (host == NULL) {
        printError("Resolving DNS error");
        return;
    }
    usleep(10000);

    //TCP socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printError("Socket error");
        return;
    }
    usleep(10000);
    //server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(80); // HTTP port
    memcpy(&server_addr.sin_addr.s_addr, host->h_addr_list[0], host->h_length);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printError("\x1b[1;31mConnection to the server failed\x1b[0m\n");
        return;
    }
    usleep(10000);


    char url[2048];
    char request[4096];
    snprintf(url, sizeof(url), "http://man.he.net/?topic=%s&section=all", command_name);
    snprintf(request, sizeof(request), "GET %s HTTP/1.1\r\nHost: man.he.net\r\n\r\n", url);
    if (send(sockfd, request, strlen(request), 0) < 0) {
        printError("GET request failed");
        close(sockfd);
        return;
    }

    char response[65536];
    ssize_t bytesRead;
    int fl = 0;
    while ((bytesRead = recv(sockfd, response, sizeof(response), 0)) > 0) {
        // FILE *somefile = fopen("some.html", "w");
        // fwrite(response, 1, bytesRead, somefile);
        printimanpage(response, fl);
        fl = 1;
        usleep(10000);
    }

    close(sockfd);
}