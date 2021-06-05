#include "server.h"
#include <semaphore.h>
#define MAX_MESSAGE_LENGTH 256

sem_t sem; //name of semaphore
int main() {
    int sock, new_sock, pid, clientLength;
    const int serverPort = 5678;
    const char* NON_EXISTENT_TAG = "key_nonexistent";
    const char* DELETED_TAG = "key_deleted";
    const char* UNEXPECTED_ERROR_TAG = "unexpected_error";
    const char* KEY_OVERWRITTEN_TAG = "key_overwritten";
    char messageFromServer[MAX_MESSAGE_LENGTH], messageFromClient[MAX_MESSAGE_LENGTH];
    struct sockaddr_in server, client;
    UserInput userInput;
    OperationResult operationResult;

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
    puts("Now listening on socket.\n");

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
            printf("New client has connected to the server.\n");

            // answer client with a welcome message
            memset(messageFromServer, '\0', sizeof(messageFromServer));
            strcpy(messageFromServer, "\nWelcome to Wood's Super Duper key-value store!\n");
            write(new_sock, messageFromServer, strlen(messageFromServer));

            // This loop will receive messages of the client until "QUIT".
            while(1) {
                int resultOfOperations;

                // receive message and parse it to UserInput
                memset(messageFromClient, '\0', sizeof(messageFromClient));          // fill up with zeroes to "empty" the String
                read(new_sock, messageFromClient, MAX_MESSAGE_LENGTH);                  // read new message from socket
                userInput = stringToUserInput(messageFromClient);                       // parse message to UserInput

                // validate user input
                operationResult = validateUserInput(userInput);
                if (operationResult.code != 1) {
                    memset(messageFromServer, '\0', sizeof(messageFromServer));
                    sprintf(messageFromServer, "> %s:%s\n", "invalid_input", operationResult.message);
                    write(new_sock, messageFromServer, strlen(messageFromServer));
                    continue;
                }

                // start of marius' part: userInput is valid
                memset(messageFromServer, '\0', sizeof(messageFromServer));         // empty response String
                if (strncmp("PUT", userInput.command, 3) == 0) {                    // if else ladder because switch case is not applicable
                    // enter critical area
                    operationResult.code = put(userInput.key, userInput.value);
                    if(operationResult.code == 1){
                        strcat(userInput.value, KEY_OVERWRITTEN_TAG);
                    }
                    // leave critical area
                } else if (strncmp("GET", userInput.command, 3) == 0) {
                    // enter critical area
                    operationResult.code = get(userInput.key, userInput.value);
                    if (operationResult.code == -2){
                        sprintf(userInput.value, "%s", NON_EXISTENT_TAG);
                    }
                    // leave critical area
                } else if (strncmp("DEL", userInput.command, 3) == 0) {             // fill userInput.value based on function result to
                    memset(userInput.value, '\0', sizeof(userInput.value));
                    // enter critical area
                    operationResult.code = del(userInput.key);
                    // leave critical area
                    switch (operationResult.code) {
                        case -2:
                            sprintf(userInput.value, "%s", NON_EXISTENT_TAG);
                            break;
                        case -1:
                            sprintf(userInput.value, "%s", UNEXPECTED_ERROR_TAG);
                            break;
                        default:
                            sprintf(userInput.value, "%s", DELETED_TAG);
                            break;
                    }
                } else if (strncmp("QUIT", userInput.command, 4) == 0)  {
                    strcpy(messageFromServer, "> bye bye\n");
                    write(new_sock, messageFromServer, strlen(messageFromServer)); // send back response Value
                    break;
                }
                sprintf(messageFromServer, "> %s:%s:%s\n", userInput.command, userInput.key, userInput.value);
                write(new_sock, messageFromServer, strlen(messageFromServer)); // send back response Value

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
            printf("Closed connection to a client.\n");
            exit(0);    // terminate the child process
        }
    }

    return 0;
}
