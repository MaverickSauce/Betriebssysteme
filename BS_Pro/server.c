#include "server.h"
#include <semaphore.h>
#define MAX_MESSAGE_LENGTH 256

sem_t sem; //name of semaphore
int main() {
    int sock, new_sock, pid, clientLength;
    const int serverPort = 5678;
    char messageFromServer[MAX_MESSAGE_LENGTH], messageFromClient[MAX_MESSAGE_LENGTH];
    struct sockaddr_in server, client;
    UserInput userInput;

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

    // listen to socket with a maximum of 10.000 simultaneous connections
    listen(sock,10000);
    puts("Now listening on socket.");

    // accept connection
    clientLength = sizeof(client);

    while(1) {
        new_sock = accept(sock, (struct sockaddr *) &client, (socklen_t*) &clientLength);
        pid = fork();
        if (pid < 0) {
            puts("An error occurred while forking.");
            exit(1);
        } else if (pid > 0) {
            close(new_sock);
        } else {
            if (new_sock == -1) {
                puts("A client failed to connect to the server.\n");
                exit(1);
            }
            printf("\nNew client has connected to the server.\n");

            // answer client with a welcome message
            memset(messageFromServer, '\0', sizeof(messageFromServer));
            strcpy(messageFromServer, "Hello there...\n");
            write(new_sock, messageFromServer, strlen(messageFromServer));
            printf("Sent a welcome message to client.\n");

            // This loop will receive messages of the client until "QUIT".
            while(1) {

                // receive message, parse to UserInput and validate it
                memset(messageFromClient, '\0', sizeof(messageFromClient));          // fill up with zeroes to "empty" the String
                read(new_sock, messageFromClient, MAX_MESSAGE_LENGTH);                  // read new message from socket
                userInput = stringToUserInput(messageFromClient);                       // parse message to UserInput
                if (!isValidUserInput(userInput)) {                                     // check if input is not valid
                    memset(messageFromServer, '\0', sizeof(messageFromServer));      // fill up with zeroes to "empty" the String
                    strcpy(messageFromServer, "> invalid_input\n");                 // copy new message to the String
                    write(new_sock, messageFromServer, strlen(messageFromServer));      // send new message to the client
                    continue;                                                           // start at the beginning of the loop
                }

                // start of marius' part: userInput is valid

                // Feel free to change this part.
                if (strncmp("QUIT", messageFromClient, 4) == 0)  {
                    memset(messageFromServer, '\0', sizeof(messageFromServer));
                    strcpy(messageFromServer, "> bye bye\n");
                    write(new_sock, messageFromServer, strlen(messageFromServer));
                    break;
                }

                // end of marius' part

                //emre's Part                                                                                           //discuss critical areas before continuing
                /*
                 if (strncmp("BEG", messageFromClient, 4) == 0)  {
                    sem_wait(&sem);
                    memset(messageFromServer, '\0', sizeof(messageFromServer));
                    strcpy(messageFromServer, "> entering exclusive mode\n");
                    write(new_sock, messageFromServer, strlen(messageFromServer));
                  }


                if (strncmp("END", messageFromClient, 4) == 0)  {
                    sem_post(&sem);
                    memset(messageFromServer, '\0', sizeof(messageFromServer));
                    strcpy(messageFromServer, "> exiting exclusive mode\n");
                    write(new_sock, messageFromServer, strlen(messageFromServer));
                }
                */
            }

            close(new_sock);
            printf("Closed connection to a client.\n\n");
            exit(0);    // terminate the child process
        }
    }

    return 0;
}
