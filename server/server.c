/**
 * @file server.c
 * 
 * @author Nicholas Hellmers Davalos (nhellmers2.0@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2024-05-13
 * 
 * This is the server side of the streaming service. This will implement HLS streaming to requesting clients though a TCP connection.
 * 
 * The body of the HTTP response is a 200 OK response with the file "`index`.m3u8" as the body.
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h> // For TCP keep-alive options
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define SERV_PORT 80
#define MAX_CLIENTS 10
#define MAXLINE 1024

int main(int argc, char *argv) {
    int listenfd, connfd, n;
    pid_t childpid;
    socklen_t clilen;
    char buf[MAXLINE];
    struct sockaddr_in cliaddr, servaddr;
    int optval; // For TCP keep-alive options
    socklen_t optlen = sizeof(optval);

    // Here we check if we're running on macOS or Linux
    #ifdef __APPLE__
        printf("Running on macOS\n");
        #define TCP_KEEPIDLE TCP_KEEPALIVE
    #elif __linux__
        printf("Running on Linux\n");
    #else
        printf("Running on an unknown OS\n");
    #endif

    // Create a socket for the soclet
    // If sockfd<0 there was an error in the creation of the socket
    if ((listenfd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Problem in creating the socket");
        exit(2);
    }

    // Enable TCP keep-alive on the socket
    optval = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0) {
        perror("setsockopt(SO_KEEPALIVE) failed");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    // Set the keep-alive idle time to 10 seconds
    optval = 10; // Idle time in seconds before sending keepalive probes
    if (setsockopt(listenfd, IPPROTO_TCP, TCP_KEEPALIVE, &optval, optlen) < 0) {
        perror("setsockopt(TCP_KEEPIDLE) failed");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    // Preparation of the socket address
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if (argc == 2) {
        servaddr.sin_port = htons(atoi(argv[1]));
    } else {
        servaddr.sin_port = htons(SERV_PORT);
    }

    // Bind the socket
    if (bind (listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("Problem in binding the socket");
        exit(2);
    }

    // Listen to the socket by creating a connection queue, then wait for clients
    listen (listenfd, MAX_CLIENTS);

    printf("%s\n","Server running...waiting for connections.");

    while (1) {
        clilen = sizeof(cliaddr);

        // Accept a connection
        if ((connfd = accept (listenfd, (struct sockaddr *) &cliaddr, &clilen)) == -1) {
            perror("Problem in accepting the connection");
            exit(2);
        }

        printf("%s\n","Received request...");

        // Enable TCP keep-alive on the connection
        int optval = 1;
        socklen_t optlen = sizeof(optval);
        if (setsockopt(connfd, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) == -1) {
            perror("setsockopt(SO_KEEPALIVE) failed");
            close(connfd);
            continue; // Depending on how critical this setting is, you might choose to continue or terminate
        }

        optval = 10; // macOS and BSD might use TCP_KEEPALIVE for similar purpose, if you're on Linux, you might want to use TCP_KEEPIDLE instead
        if (setsockopt(connfd, IPPROTO_TCP, TCP_KEEPIDLE, &optval, optlen) < 0) {
            perror("setsockopt(TCP_KEEPIDLE) failed on connfd");
            close(connfd);
            continue; // or exit
        }

        printf("Connection from %s, port %d\n", inet_ntop(AF_INET, &cliaddr.sin_addr, buf, sizeof(buf)), ntohs(cliaddr.sin_port));

        if ((childpid = fork()) == 0) { // If it is 0, it is the child process
            printf ("%s\n","Child created for dealing with client requests");

            close(listenfd);

            while ((n = recv(connfd, buf, MAXLINE,0)) > 0) {
                printf("%s","String received from and resent to the client:");
                puts(buf);
                send(connfd, buf, n, 0);
            }

            if (n == -1) {
                perror("Read error");
                exit(3);
            }

            printf("%s","Child exiting...");
            close(connfd);
            exit(0);
        }

        close(connfd);

    }

    close(listenfd);

    return 0;
}