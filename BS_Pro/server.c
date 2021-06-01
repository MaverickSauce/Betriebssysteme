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
    puts("Created socket for the server.");

    // fill sockaddr_in struct
    server.sin_family = AF_INET;            // IPv4 address
    server.sin_addr.s_addr = INADDR_ANY;    // on every interface
    server.sin_port = htons(serverPort);    // on port 5678

    // bind
    if (bind(sock, (struct sockaddr *)&server, sizeof (server)) < 0) {
        printf("Failed to bind socket to port %i. Please try again.", serverPort);
        return -1;
    }
    printf("Bound socket to port %i.\n", serverPort);

    // listen to socket with a maximum of 100 connections
    listen(sock,100);
    puts("Now listening on socket.");

    // accept connection
    clientLength = sizeof(client);

    for (numOfConnections=0; numOfConnections<100; numOfConnections++) {
        new_sock = accept(sock, (struct sockaddr *) &client, (socklen_t*) &clientLength);
        pid = fork();
        if (pid < 0) {
            puts("An error occurred while forking.");
            exit(1);
        } else if (pid > 0) {
            close(new_sock); // why?
        } else {
            if (new_sock == -1) {
                printf("Client %i failed to connect to the server.\n", numOfConnections);
                exit(1);
            }
            printf("\nClient %i has connected to the server.\n", numOfConnections);

            // answer client with a simple string
            memset(messageFromServer, '\0', sizeof(messageFromServer)); // fill up the String with zeroes to effectively empty it.
            strcpy(messageFromServer, "Welcome, please enter your commands. Enter QUIT to close the connection.\n");
            write(new_sock, messageFromServer, strlen(messageFromServer));
            printf("Sent a message to client %i:\n", numOfConnections);
            puts(messageFromServer);

            // This loop will echo every input of the client until receiving "QUIT".
            // Everything below the first read() will be deleted later.
            while(1) {
                // fill up the messages with zeroes to reuse them
                memset(messageFromServer, '\0', sizeof(messageFromServer));
                memset(messageFromClient, '\0', sizeof(messageFromClient));

                // waiting for a message from client
                read(new_sock, messageFromClient, MAX_MESSAGE_LENGTH);    // @MaverickSauce: input will be in messageFromClient
                //-> put the input validation around here

                // printing the client-message
                printf("Client %i sent a message:\n", numOfConnections);
                puts(messageFromClient);

                // building the response
                strcpy(messageFromServer, "> Your command was: ");
                strcat(messageFromServer, messageFromClient);

                // sending the response
                write(new_sock, messageFromServer, strlen(messageFromServer));
                printf("Sent a message to client %i:\n", numOfConnections);
                puts(messageFromServer);

                // This comparison is a bit dirty but it should not be a problem when dealing with tokens.
                if (strncmp("QUIT", messageFromClient, 4) == 0)  {
                    memset(messageFromServer, '\0', sizeof(messageFromServer)); // fill up the String with zeroes to effectively empty it.
                    strcpy(messageFromServer, "> bye bye\n");
                    write(new_sock, messageFromServer, strlen(messageFromServer));
                    break;
                }
            }

            close(new_sock);
            printf("Closed connection to client %i.\n\n", numOfConnections);
            exit(0);
        }
    }

    return 0;
}
