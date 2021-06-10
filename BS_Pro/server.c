#include "server.h"
#define MAX_MESSAGE_LENGTH 256
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/shm.h>


int main() {
    int sock, new_sock, pid, clientLength;
    int sharedReadCounterID, *sharedReadCounter;
    int semStorage, semReadCounter;
    int exclusiveAccessRights = 0;
    unsigned short marker[1] = { 1 };
    const int serverPort = 5678;
    char messageFromServer[MAX_MESSAGE_LENGTH], messageFromClient[MAX_MESSAGE_LENGTH];

    struct sockaddr_in server, client;
    UserInput userInput;
    OperationResult operationResult;

    // create semaphores
    semStorage = semget(IPC_PRIVATE, 1, IPC_CREAT | 0777);
    semReadCounter = semget(IPC_PRIVATE, 1, IPC_CREAT | 0777);

    // initialize semaphores
    semctl(semStorage, 1, SETALL, marker);
    semctl(semReadCounter, 1, SETALL, marker);

    struct sembuf semaphore_lock = {0, -1, SEM_UNDO};
    struct sembuf semaphore_unlock = {0, 1, SEM_UNDO};

    // setup shared memory for readerCount
    sharedReadCounterID = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0644);
    if (sharedReadCounterID == -1) {
        puts("Some error occurred while setting up shared memory for readerCount.");
        exit(1);
    }
    sharedReadCounter = (int *)shmat(sharedReadCounterID, 0, 0);
    *sharedReadCounter = 0;

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

                // receive message and parse it to UserInput
                memset(messageFromClient, '\0', sizeof(messageFromClient));          // fill up with zeroes to "empty" the String
                read(new_sock, messageFromClient, MAX_MESSAGE_LENGTH);                  // read new message from socket
                userInput = stringToUserInput(messageFromClient);                       // parse message to UserInput

                // validate user input
                operationResult = validateUserInput(userInput);
                if (operationResult.code != 0) {
                    memset(messageFromServer, '\0', sizeof(messageFromServer));
                    sprintf(messageFromServer, "> %s:%s\n", "invalid_input", operationResult.message);
                    write(new_sock, messageFromServer, strlen(messageFromServer));
                    continue;
                }

                // execute command of client
                memset(messageFromServer, '\0', sizeof(messageFromServer));       // empty response String
                if (strncmp("PUT", userInput.command, 3) == 0) {                    // if else ladder because switch case is not applicable

                    if (!exclusiveAccessRights) semop(semStorage, &semaphore_lock, 1);      // enter critical area: storage
                    operationResult = put(userInput.key, userInput.value);
                    if (!exclusiveAccessRights) semop(semStorage, &semaphore_unlock, 1);    // leave critical area: storage

                    strcat(userInput.value, operationResult.message);
                } else if (strncmp("GET", userInput.command, 3) == 0) {
                    if (!exclusiveAccessRights) {
                        semop(semReadCounter, &semaphore_lock,1);             // enter critical area: sharedReadCounter
                        *sharedReadCounter = *sharedReadCounter + 1;
                        if (*sharedReadCounter == 1) {
                            semop(semStorage, &semaphore_lock, 1);            // enter critical area: storage
                        }
                        semop(semReadCounter, &semaphore_unlock,1);           // leave critical area: sharedReadCounter
                    }

                    sleep(10);      // only for testing
                    operationResult = get(userInput.key, userInput.value);

                    if (!exclusiveAccessRights) {
                        semop(semReadCounter, &semaphore_lock,1);              // enter critical area: sharedReadCounter
                        *sharedReadCounter = *sharedReadCounter - 1;
                        if (*sharedReadCounter == 0) {
                            semop(semStorage, &semaphore_unlock, 1);           // leave critical area: storage
                        }
                        semop(semReadCounter, &semaphore_unlock,1);            // leave critical area: sharedReadCounter
                    }

                    if (operationResult.code < 0) {
                        sprintf(userInput.value, "%s", operationResult.message);
                    }
                } else if (strncmp("DEL", userInput.command, 3) == 0) {             // fill userInput.value based on function result to

                    if (!exclusiveAccessRights) semop(semStorage, &semaphore_lock, 1);      // enter critical area: storage
                    operationResult = del(userInput.key);
                    if (!exclusiveAccessRights) semop(semStorage, &semaphore_unlock, 1);    // leave critical area: storage

                    memset(userInput.value, '\0', sizeof(userInput.value));
                    sprintf(userInput.value, "%s", operationResult.message);
                } else if (strncmp("BEG", userInput.command, 3) == 0) {
                    if (!exclusiveAccessRights) {
                        semop(semStorage, &semaphore_lock, 1);                  // enter exclusive access
                        exclusiveAccessRights = 1;
                        strcpy(messageFromServer, "> BEG:activated\n");
                    } else {
                        strcpy(messageFromServer, "> BEG:already_active\n");
                    }
                    write(new_sock, messageFromServer, strlen(messageFromServer));
                    continue;
                } else if (strncmp("END", userInput.command, 3) == 0) {
                    if (exclusiveAccessRights) {
                        exclusiveAccessRights = 0;
                        semop(semStorage, &semaphore_unlock, 1);                // leave exclusive access
                        strcpy(messageFromServer, "> END:deactivated\n");
                    } else {
                        strcpy(messageFromServer, "> END:already_not_active\n");
                    }
                    write(new_sock, messageFromServer, strlen(messageFromServer));
                    continue;
                } else if (strncmp("QUIT", userInput.command, 4) == 0)  {
                    strcpy(messageFromServer, "> bye bye\n");
                    write(new_sock, messageFromServer, strlen(messageFromServer)); // send back response Value
                    break;
                }
                sprintf(messageFromServer, "> %s:%s:%s\n", userInput.command, userInput.key, userInput.value);
                write(new_sock, messageFromServer, strlen(messageFromServer)); // send back response Value
            }

            close(new_sock);
            printf("Closed connection to a client.\n");
            exit(0);    // terminate the child process
        }
    }

    return 0;
}
