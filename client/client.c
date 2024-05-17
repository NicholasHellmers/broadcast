/**
 * @file server.c
 * 
 * @author Nicholas Hellmers Davalos (nhellmers2.0@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2024-05-13
 * 
 * This is the client side of the streaming service. This will request HLS streaming from the server through a TCP connection.
 * 
 * The body of the HTTP request is a GET request for the file "`index`.m3u8" from the server.
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

#define SERV_PORT 3000
#define MAXLINE 1024

int main(int argc, char *argv) {
    int sockfd, n;
    char recvline[MAXLINE + 1];
    struct sockaddr_in servaddr;
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
    if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Problem in creating the socket");
        exit(2);
    }

    // Enable TCP keep-alive on the socket
    optval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0) {
        perror("setsockopt(SO_KEEPALIVE) failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Set up the server address struct
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    servaddr.sin_addr.s_addr = inet_addr(INADDR_ANY);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1) {
        perror("Problem in connecting to the server");
        exit(3);
    }

    // Send the HTTP request
    char *request = "GET /index.m3u8 HTTP/1.1\r\nHost: localhost:3000\r\n\r\n";

    ssize_t bytes_sent;

    if ((bytes_sent = send(sockfd, request, strlen(request), 0)) == -1) {
        perror("Problem in sending the request");
        exit(4);
    }

    // Read the response from the server
    while ((n = recv(sockfd, recvline, MAXLINE, 0)) > 0) {
        printf("%s", "Received:\n");

        recvline[n] = "\0";

        if (fputs(recvline, stdout) == EOF) {
            perror("fputs error");
            exit(5);
        }

    }

    if (n < 0) {
        perror("Read error");
        return -1;
    }

    return 0;

}

        