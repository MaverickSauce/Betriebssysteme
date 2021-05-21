//
// Created by fatma on 08.05.2021.
//
#include "server.h"
#include<sys/socket.h>
#include<stdio.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(){                     //nicht ausführbar wenn main.c bereits existiert

    int socket_sock; // create socket
    struct sockaddr_in server; //contains server attributes
    socket_sock = socket(AF_INET , SOCK_STREAM , 0);

    if(socket_sock == -1){
        puts("Could not create socket");
    }

    server.sin_addr.s_addr = inet_addr("255.255.255.255");
    server.sin_port = htons(80);
    server.sin_family = AF_INET;
    server.sin_zero;

    if (connect(socket_sock,(struct sockaddr *)&server, sizeof (server)<0)){
        puts("connection error");
        return 1;
    }

    puts("connected");

    puts("hallo");
    close(socket_sock);
    return 0;
}
