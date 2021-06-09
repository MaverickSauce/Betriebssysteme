#include "server.h"
#define MAX_MESSAGE_LENGTH 256
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>

int semread,semwrite,semdelete,exclusive;
int main() {
    int *ex;
    int shm_id;
    //setup semaphores                                              //nsems = number of semaphores in set
    //IPC_PRIVATE = opens private key | IPC_CREAT = creates KEY

    semwrite = semget(IPC_PRIVATE,1,IPC_CREAT);
    exclusive = semget(IPC_PRIVATE,1,IPC_CREAT);

    //crud on semaphore set
    semctl(semwrite,1,SETALL,1);
    semctl(exclusive,1,SETALL,1);

    int semop(int semid,struct sembuf sem_array[],size_t n_op); //operation on semaphore set
    /*struct sembuf {
        unsigned short sem_num;                                 //semaphornummer in der menge
        short sem_op;                                           //semaphoroperation
        short sem_flg;                                          //Flags: IPC_NOWAIT,SEM_UNDO
    };
     */
    struct sembuf semaphore_lock[1] = {0,-1,SEM_UNDO};
    struct sembuf semaphore_unlock[1] = {0,1,SEM_UNDO};

    //setup shared memory
    shm_id = shmget(IPC_PRIVATE, sizeof(int), 0644 | IPC_CREAT);
    ex = shmat(shm_id,NULL,0);
    *ex= 0;




    int sock, new_sock, pid, clientLength;
    const int serverPort = 5678;
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
            printf("Sent a welcome message to client.\n");

            // This loop will receive messages of the client until "QUIT".
            while(1) {
                int resultOfOperations;

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
                    // enter critical area
                    semop(semwrite, &semaphore_lock[0], 1);
                    operationResult = put(userInput.key, userInput.value);
                    semop(semwrite, &semaphore_unlock[0], 1);
                    strcat(userInput.value, operationResult.message);
                    // leave critical area
                } else if (strncmp("GET", userInput.command, 3) == 0) {
                    // enter critical area
                    semop(semwrite,&semaphore_lock[0],1);
                    operationResult = get(userInput.key, userInput.value);
                    semop(semdelete,&semaphore_unlock[0],1);
                    if (operationResult.code < 0) {
                        sprintf(userInput.value, "%s", operationResult.message);
                    }

                    // leave critical area
                } else if (strncmp("DEL", userInput.command, 3) == 0) {             // fill userInput.value based on function result to
                    memset(userInput.value, '\0', sizeof(userInput.value));
                    // enter critical area
                    semop(semwrite,&semaphore_lock[0],1);
                    operationResult = del(userInput.key);
                    semop(semwrite,&semaphore_unlock[0],1);
                    // leave critical area
                    sprintf(userInput.value, "%s", operationResult.message);
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
