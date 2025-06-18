#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define CACHE_DIR "./cache/"

/**
 * Extracts the file path of length `m` in the first line of the request stored in 
 * `buffer` of size `n` and stores it in `dest`, 
 */
void extract_file_path(char* buffer, char* dest, int n, int* m) {
    int j = 0;
    *m = 0;
    for (int i = 5; i < n; i++) {
        if (buffer[i] != ' ') {
            dest[j++] = buffer[i];
            (*m)++;
        } else {
            break;
        }
    }
}

/**
 * Extracts the host name from the URL stored in `url` and stores it in `dest`.
 * The URL is of the form "<host_name>/<path>" (i.e. no http:// or https://).
 * The host name is the part before the first "/".
 * The path is the part after the first "/".
 * Returns the index of the first "/" in the URL (i.e. the start of the path)
 * The host name is stored in `dest`
 */
int extract_host_name(char* url, char* dest) {
    int j = 0;
    for (int i = 0; i < strlen(url); i++) {
        if (url[i] == '/') {
            return i; 
        }
        dest[j++] = url[i];
    }
    return -1;
}

void extract_path(char* url, char* dest) {
    char junk[50];
    int j = extract_host_name(url, junk);
    if (j == -1) return;
    strcpy(dest, url + j);
}

int main() {

    // AF_INET: IPv4
    // SOCK_STREAM: TCP
    // 0: default protocol for this config
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        printf("Error in creating socket\n");
        return -1;
    }

    // bind

    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(3000);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    int yes = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if (bind(socket_fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        printf("Failed to bind errno %d\n", errno);
        return -1;
    }

    if (listen(socket_fd, 10) == -1) {
        printf("Failed to listen %d\n", errno);
        return -1;
    }

    int clientfd = accept(socket_fd, NULL, NULL);
    printf("Connected with client %d\n", clientfd);


    char buffer[1024] = {0};
    char file_path[512] = {0};
    while (1) {
        int bytes_read = recv(clientfd, buffer, 1024, 0);
        if (bytes_read == -1) {
            printf("Error in recv %d\n", errno);
            return -1;
        } else if (bytes_read == 0) {
            printf("Client disconnected \n");
            shutdown(clientfd, 0);
            return 0;
        }

        printf("[RECV %d bytes] %s\n", bytes_read, buffer);

        // extract requested object, which is between "GET " amd " HTTP/1.1"
        int path_len = 0;
        extract_file_path(buffer, file_path, 1024, &path_len);

        // we basically fetch this from our server
        printf("Extracted path: %s\n", file_path);

        // we check if we have this file stored in this directory 
        char catted[520] = CACHE_DIR;
        strcat(catted, file_path);
        catted[strlen(catted) - 1] = '\0';
        FILE* fp = fopen(catted, "r");
        printf("catted %s\n", catted);
        if (fp != NULL) {
            // cache hit
            printf("Cache hit on file %s!\n", file_path);

            return 0;
        }
        printf("Cache miss for file %s\n", file_path);


        struct addrinfo* fetchaddr = malloc(sizeof(struct addrinfo));
        struct addrinfo* dns_info = malloc(sizeof(struct addrinfo));
        dns_info->ai_family = AF_INET;
        dns_info->ai_socktype = SOCK_STREAM;

        char host_name[512] = {0};
        char path[1024] = {0};
        extract_host_name(file_path, host_name);
        extract_path(file_path, path);
        printf("Original URL %s hostname %s\n", file_path, host_name);
        printf("Path %s\n", path);
        // get the IP address of the host

        int rv = getaddrinfo(host_name, "80", dns_info, &fetchaddr);
        if (rv != 0) {
            printf("Error in getaddrinfo %d %s\n", rv, gai_strerror(rv));
            return -1;
        }
        char *ip = inet_ntoa(((struct sockaddr_in *) (fetchaddr->ai_addr))->sin_addr);
        printf("%s\n", ip);
        int fetch_fd = socket(fetchaddr->ai_family, fetchaddr->ai_socktype, fetchaddr->ai_protocol);
        connect(fetch_fd, fetchaddr->ai_addr, fetchaddr->ai_addrlen);
        if (fetch_fd == -1) {
            printf("Error in creating socket %d\n", errno);
            return -1;
        }

        printf("Created fetch socket to fetch %s.\n", file_path);

        char request[1024] = {0};
        sprintf(request, "GET %s HTTP/1.1\r\nHost: %s\r\n", path, host_name);
        // copy the rest of the header into the request
        // find the second set of \r\n
        int flag = 0;
        for (int i = 0; i < bytes_read; i++) {
            if (buffer[i] == '\r' && buffer[i+1] == '\n') {
                if (flag == 0) flag = 1;
                else { // skip the first \r\n used for GET line
                    strcat(request, &buffer[i+2]);
                    break;
                }
            }
        }
        printf("Request: %s\n", request);
        int bytes_sent = send(fetch_fd, request, strlen(request), 0);
        if (bytes_sent == -1) {
            printf("Error in send %d\n", errno);
            return -1;
        }
        printf("Sent %d bytes to fetch server\n", bytes_sent);
        char* response = malloc(1000000); // 1 Mb
        memset(response, 0, 1000000);
        int bytes_received = recv(fetch_fd, response, 65536, 0);
        if (bytes_received == -1) {
            printf("Error in recv %d\n", errno);
            return -1;
        }
        fclose(fp);





        fp = fopen(catted, "w");
        if (fp == NULL) {
            printf("Error in opening file %s %d\n", catted, errno);
            return -1;
        }
        fwrite(response, 1, bytes_received, fp);
        fclose(fp);

        printf("Received %d bytes from fetch server\n", bytes_received);
        printf("Fetch Response: %s\n", response);
        send(clientfd, response, bytes_received, 0);

        free(response);
        free(fetchaddr);
        free(dns_info);


    }
    







    return 0;
}