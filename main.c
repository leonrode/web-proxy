#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>


int main() {

    // AF_INET: IPv4
    // SOCK_STREAM: TCP
    // 0: default protocol for this config
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        printf("Error in creating socket\n");
        return -1;
    }
    




    return 0;
}