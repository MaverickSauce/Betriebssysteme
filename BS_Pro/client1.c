
#include<sys/socket.h>
#include<stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>

int client(){

    int socket_sock; // create socket
    struct sockaddr_in server; //contains  attributes
    socket_sock = socket(AF_INET , SOCK_STREAM , 0);

    if(socket_sock == -1){
        puts("Could not create socket");
    }

    //fill attributes
    server.sin_addr.s_addr = inet_addr("255.255.255.255");            //insert ip address
    server.sin_port = htons(80);                                //insert port number of desired service here
    server.sin_family = AF_INET;                                        //ipv4 protocol

    if (connect(socket_sock,(struct sockaddr *)&server, sizeof (server)<0)){
        puts("connection error");
        return 1;
    }
    puts("connected");

    ssize_t send ();
    ssize_t recv();

    close(socket_sock);
    return 0;
}
