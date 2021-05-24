#include "server.h"
#define MAX_MESSAGE_LENGTH 256

int main() {
    int sock, new_sock, pid, clientLength, numOfConnections;
    const int serverPort = 5678;
    char messageFromServer[MAX_MESSAGE_LENGTH], messageFromClient[MAX_MESSAGE_LENGTH];
    struct sockaddr_in server, client;

    // create socket for IPv4 address, TCP-protocol, IP-protocol
    sock = socket(AF_INET,SOCK_STREAM,0);
    if (sock == -1) {
        puts("Failed to create a socket for the server. Please try again.");
        return -1;
    }
    puts("Creating socket for the server was successful.");

    // fill sockaddr_in struct
    server.sin_family = AF_INET;            // IPv4 address
    server.sin_addr.s_addr = INADDR_ANY;    // on every interface
    server.sin_port = htons(serverPort);    // on port 5678

    // bind
    if (bind(sock, (struct sockaddr *)&server, sizeof (server)) < 0) {
        printf("Failed to bind socket to port %i. Please try again", serverPort);
        return -1;
    }
    printf("Binding socket to port %i was successful.\n", serverPort);

    // listen to socket with a maximum of 100 connections
    listen(sock,100);
    puts("Now listening on socket.");

    // accept connection
    clientLength = sizeof(client);

    for (numOfConnections=0; numOfConnections<100; ++numOfConnections) {
        new_sock = accept(sock, (struct sockaddr *) &client, (socklen_t*) &clientLength);
        pid = fork();
        if (pid < 0) {
            puts("An error ocurred while forking.");
            continue;
        } else if (pid > 0) {
            close(new_sock); // why?
        } else {
            if (new_sock == -1) {
                printf("Client %i failed to connect to the server.\n", numOfConnections);
                continue;
            }
            printf("\nClient %i has connected to the server.\n", numOfConnections);

            // answer client with a simple string
            strcpy(messageFromServer, "Hello there ...\n");
            write(new_sock, messageFromServer, strlen(messageFromServer));
            printf("Send a message to client %i:\n", numOfConnections);
            puts(messageFromServer);

            // waiting for a response from client
            printf("Now waiting for a response from client %i.\n\n", numOfConnections);
            read(new_sock, messageFromClient, MAX_MESSAGE_LENGTH);
            printf("Client %i sent a response:\n", numOfConnections);
            puts(messageFromClient);

            close(new_sock);
        }
    }

    return 0;
}
