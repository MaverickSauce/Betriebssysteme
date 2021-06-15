#include "server.h"

int main() {
    const int serverPort = 5678;
    int exclusiveAccessRights = 0;

    // create semaphores
    int semStorage = semget(IPC_PRIVATE, 1, IPC_CREAT | 0777);
    int semReadCounter = semget(IPC_PRIVATE, 1, IPC_CREAT | 0777);
    int semSubscriptionList = semget(IPC_PRIVATE, 1, IPC_CREAT | 0777);

    // initialize semaphores
    unsigned short marker[1] = { 1 };
    semctl(semStorage, 1, SETALL, marker);
    semctl(semReadCounter, 1, SETALL, marker);
    semctl(semSubscriptionList, 1, SETALL, marker);

    // build semaphore operations
    struct sembuf semaphore_lock = {0, -1, SEM_UNDO};
    struct sembuf semaphore_unlock = {0, 1, SEM_UNDO};

    // setup shared memory for readerCount
    int sharedReadCounterID = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0644);
    if (sharedReadCounterID == -1) {
        puts("Some error occurred while setting up shared memory for readerCount.");
        exit(1);
    }
    int *sharedReadCounter = (int *)shmat(sharedReadCounterID, 0, 0);
    *sharedReadCounter = 0;

    // setup shared memory for subscriptionList
    int sharedSubscriptionListID = shmget(IPC_PRIVATE, sizeof(subscriptionList), IPC_CREAT | 0644);
    if (sharedSubscriptionListID == -1) {
        puts("Some error occurred while setting up shared memory for subscriptionList.");
        exit(1);
    }
    subscriptionList *sharedSubscriptionList = (subscriptionList *)shmat(sharedSubscriptionListID, 0, 0);
    sharedSubscriptionList->nextFree = 0;

    // initialize message queue for subscription messages
    int messageQueue = msgget(IPC_PRIVATE, IPC_CREAT | 0777);

    // create socket for IPv4 address, TCP-protocol, IP-protocol
    int sock = socket(AF_INET,SOCK_STREAM,0);
    if (sock == -1) {
        puts("Failed to create a socket for the server. Please try again.");
        return -1;
    }
    puts("Created socket for the server.");

    // fill sockaddr_in struct for server
    struct sockaddr_in server;
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

    // prepare sockaddr_in for client
    struct sockaddr_in client;
    int clientLength = sizeof(client);

    while(1) {
        char messageFromServer[MAX_MESSAGE_LENGTH], messageFromClient[MAX_MESSAGE_LENGTH];

        UserInput userInput;
        OperationResult operationResult;

        // accept connection
        int new_sock = accept(sock, (struct sockaddr *) &client, (socklen_t*) &clientLength);
        int firstPid = fork();
        if (firstPid < 0) {
            puts("An error occurred while forking.");
            exit(1);
        } else if (firstPid > 0) {
            close(new_sock);
        } else {
            if (new_sock == -1) {
                puts("A client failed to connect to the server.\n");
                exit(1);
            }
            printf("New client has connected to the server.\n");

            // answer client with a welcome message
            memset(messageFromServer, '\0', sizeof(messageFromServer));
            strcpy(messageFromServer, "\n> Welcome to Wood's Super Duper key-value store!\n");
            write(new_sock, messageFromServer, strlen(messageFromServer));

            // second fork for receiving sub-messages
            int childPid = fork();
            if (childPid < 0) {
                puts("An error occured while forking.");
                exit(1);
            } else if (childPid > 0) {

                // This loop will receive messages of the client until "QUIT".
                while (1) {

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
                        puts(userInput.value);

                        memset(messageFromServer, '\0', sizeof(messageFromServer));
                        sprintf(messageFromServer, "> %s:%s:%s\n", userInput.command, userInput.key, userInput.value);
                        write(new_sock, messageFromServer, strlen(messageFromServer));                 // send back response Value

                        if (!exclusiveAccessRights) semop(semSubscriptionList, &semaphore_lock, 1);           // enter critical area: subscriptionList
                        notifySubscribers(messageQueue, sharedSubscriptionList, userInput.key, messageFromServer);
                        if (!exclusiveAccessRights) semop(semSubscriptionList, &semaphore_unlock, 1);         // leave critical area: subscriptionList
                    } else if (strncmp("GET", userInput.command, 3) == 0) {
                        if (!exclusiveAccessRights) {
                            semop(semReadCounter, &semaphore_lock,1);             // enter critical area: sharedReadCounter
                            *sharedReadCounter = *sharedReadCounter + 1;
                            if (*sharedReadCounter == 1) {
                                semop(semStorage, &semaphore_lock, 1);            // enter critical area: storage
                            }
                            semop(semReadCounter, &semaphore_unlock,1);           // leave critical area: sharedReadCounter
                        }

                        sleep(5);      // only for testing
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
                        memset(messageFromServer, '\0', sizeof(messageFromServer));
                        sprintf(messageFromServer, "> %s:%s:%s\n", userInput.command, userInput.key, userInput.value);
                        write(new_sock, messageFromServer, strlen(messageFromServer)); // send back response Value

                    } else if (strncmp("DEL", userInput.command, 3) == 0) {             // fill userInput.value based on function result to

                        if (!exclusiveAccessRights) semop(semStorage, &semaphore_lock, 1);      // enter critical area: storage
                        operationResult = del(userInput.key);
                        if (!exclusiveAccessRights) semop(semStorage, &semaphore_unlock, 1);    // leave critical area: storage

                        memset(userInput.value, '\0', sizeof(userInput.value));
                        sprintf(userInput.value, "%s", operationResult.message);

                        memset(messageFromServer, '\0', sizeof(messageFromServer));
                        sprintf(messageFromServer, "> %s:%s:%s\n", userInput.command, userInput.key, userInput.value);
                        write(new_sock, messageFromServer, strlen(messageFromServer)); // send back response Value

                        if (!exclusiveAccessRights) semop(semSubscriptionList, &semaphore_lock, 1);           // enter critical area: subscriptionList
                        notifySubscribers(messageQueue, sharedSubscriptionList, userInput.key, messageFromServer);
                        if (!exclusiveAccessRights) semop(semSubscriptionList, &semaphore_unlock, 1);         // leave critical area: subscriptionList
                    } else if (strncmp("BEG", userInput.command, 3) == 0) {
                        if (!exclusiveAccessRights) {
                            semop(semStorage, &semaphore_lock, 1);                  // enter exclusive access
                            semop(semSubscriptionList, &semaphore_lock, 1);
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
                            semop(semSubscriptionList, &semaphore_unlock, 1);
                            strcpy(messageFromServer, "> END:deactivated\n");
                        } else {
                            strcpy(messageFromServer, "> END:already_inactive\n");
                        }
                        write(new_sock, messageFromServer, strlen(messageFromServer));
                        continue;
                    } else if (strncmp("SUB", userInput.command, 3) == 0) {             // fill userInput.value based on function result to

                        if (!exclusiveAccessRights) semop(semStorage, &semaphore_lock, 1);      // enter critical area: storage
                        operationResult = get(userInput.key, userInput.value);
                        if (!exclusiveAccessRights) semop(semStorage, &semaphore_unlock, 1);    // leave critical area: storage
                        if (operationResult.code < 0) {
                            memset(userInput.value, '\0', sizeof(userInput.value));
                            strcpy(userInput.value, operationResult.message);
                        }

                        if (!exclusiveAccessRights) semop(semSubscriptionList, &semaphore_lock, 1);           // enter critical area: subscriptionList
                        operationResult = subscribe(sharedSubscriptionList, getpid(), userInput.key);
                        if (!exclusiveAccessRights) semop(semSubscriptionList, &semaphore_unlock, 1);         // leave critical area: subscriptionList
                        sprintf(userInput.value, "%s:%s", userInput.value, operationResult.message);

                        sprintf(messageFromServer, "> %s:%s:%s\n", userInput.command, userInput.key, userInput.value);
                        write(new_sock, messageFromServer, strlen(messageFromServer)); // send back response Value
                    } else if (strncmp("QUIT", userInput.command, 4) == 0) {
                        strcpy(messageFromServer, "> bye bye\n");
                        write(new_sock, messageFromServer, strlen(messageFromServer)); // send back response Value
                        break;
                    }
                }

                close(new_sock);
                printf("Closed connection to a client.\n");

               /* if (!exclusiveAccessRights) semop(semSubscriptionList, &semaphore_lock, 1);           // enter critical area: subscriptionList
                unsubscribeFromAllKeys(sharedSubscriptionList, getpid());
                if (!exclusiveAccessRights) semop(semSubscriptionList, &semaphore_unlock, 1);  */       // leave critical area: subscriptionList

                kill(childPid, SIGKILL); // terminate the child process
                exit(0);
            } else {
                while(1) {
                    struct messageBuffer newMessage;
                    memset(newMessage.mtext, '\0', sizeof(newMessage.mtext));
                    msgrcv(messageQueue, &newMessage, MAX_MESSAGE_LENGTH , getppid(), 0);
                    write(new_sock, newMessage.mtext, strlen(newMessage.mtext));
                }
            }
        }
    }

    return 0;
}
