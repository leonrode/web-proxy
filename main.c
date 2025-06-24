#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "utils.h"

#define BUFFER_SIZE 1000000 // 1 MB buffer

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


    char* buffer = calloc(1, BUFFER_SIZE); // 1 MB buffer
    char object[1024] = {0};
    char hostname[512] = {0};
    char path[512] = {0};

    int host_read = 0;

    while (1) {

        int bytes_read = recv(clientfd, buffer, BUFFER_SIZE, 0);
        if (bytes_read == -1) {
            printf("Error in recv %d\n", errno);
            return -1;
        } else if (bytes_read == 0) {
            printf("Client disconnected \n");
            shutdown(clientfd, 0);
            return 0;
        }

        printf("[RECV %d bytes] %s\n", bytes_read, buffer);

        printf("[END RECV]\n");




        if (host_read == 0) {
            // then the object is the hostname
            int hostname_len = extract_object_from_buffer(buffer, hostname, bytes_read);
            host_read = 1;
        } 

        printf("Host %s\n", hostname);

        
    

        // extract requested object, which is between "GET " amd " HTTP/1.1"

        struct addrinfo* fetchaddr = malloc(sizeof(struct addrinfo));
        struct addrinfo* dns_info = malloc(sizeof(struct addrinfo));
        dns_info->ai_family = AF_INET;
        dns_info->ai_socktype = SOCK_STREAM;
        // get the IP address of the host

        int rv = getaddrinfo(hostname, "80", dns_info, &fetchaddr);
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

        char request[1024] = {0};
        sprintf(request, "GET %s HTTP/1.1\r\nHost: %s\r\n", "/", hostname);
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
        char* response = calloc(1, 1000000); // 1 Mb
        int bytes_received = recv(fetch_fd, response, 65536, 0);
        if (bytes_received == -1) {
            printf("Error in recv %d\n", errno);
            return -1;
        }

        printf("RECV %d Response: %s\n", bytes_received, response);

        send(clientfd, response, bytes_received, 0);
        // printf("Sent %d bytes to client\n", bytes_received);



        // printf("Waiting for client request\n");
        // bytes_received = recv(clientfd, buffer, BUFFER_SIZE, 0);
        // printf("RECV %d bytes\n", bytes_received);
        // printf("Request: %s\n", buffer);        

        
        
        free(buffer);
        free(response);
        freeaddrinfo(fetchaddr);
        freeaddrinfo(dns_info);
        
        



    }
    







    return 0;
}