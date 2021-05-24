#include "server.h"
#define MAX_MESSAGE_LENGTH 256

int main() {
    int sock, new_sock, lengthOfsockaddrIn;
    const int serverPort = 5678;
    char messageFromServer[MAX_MESSAGE_LENGTH], messageFromClient[MAX_MESSAGE_LENGTH];
    struct sockaddr_in server, client;

    // create socket for IPv4 address, TCP-protocol, IP-protocol
    sock = socket(AF_INET,SOCK_STREAM,0);
    if (sock == -1)
    {
        puts("Failed to create a socket for the server. Please try again.");
        return -1;
    }
    puts("Creating socket for the server was successful.");

    // fill sockaddr_in struct
    server.sin_family = AF_INET;            // IPv4 address
    server.sin_addr.s_addr = INADDR_ANY;    // on every interface
    server.sin_port = htons(serverPort);    // on port 5678

    // bind
    if (bind(sock, (struct sockaddr *)&server, sizeof (server)) < 0)
    {
        printf("Failed to bind socket to port %i. Please try again", serverPort);
        return -1;
    }
    printf("Binding socket to port %i was successful.\n", serverPort);

    // listen to socket with a maximum of 100 connections
    listen(sock,100);

    // accept connection
    printf("Now listening on socket and waiting for connections ... \n\n");
    lengthOfsockaddrIn = sizeof(struct sockaddr_in);
    new_sock = accept(sock, (struct sockaddr *) &client, (socklen_t*) &lengthOfsockaddrIn);
    if (new_sock == -1)
    {
        puts("A client failed to connect to the server.");
        return -1;
    }
    puts("A client has connected to the server.");

    // answer client with a simple string
    strcpy(messageFromServer, "Hello there ...\n");
    write(new_sock, messageFromServer, strlen(messageFromServer));
    puts("Send a message to the client:");
    puts(messageFromServer);
    printf("\nNow waiting for a response from client.\n\n");

    // waiting for a response from client
    read(new_sock, messageFromClient, MAX_MESSAGE_LENGTH);
    puts("The client sent a response:");
    puts(messageFromClient);

    return 0;
}
