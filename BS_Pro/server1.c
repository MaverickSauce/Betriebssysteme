//
// Created by emre on 22.05.21.
//

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>

int server(){

    int sock = socket(AF_INET,SOCK_STREAM,0);
    int new_sock;

    struct sockaddr_in server, client;

    //create socket
    if(sock == -1)
    {
        puts("could not create socket");
    }

    //fill structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr =INADDR_ANY;
    server.sin_port = htons(80);

    //bind
    if(bind(sock,(struct sockaddr *)&server,sizeof (server))<0)
    {
        puts("bind failed");
    }
    puts("bind succesful");

    //LISTEN
    listen(sock,5);

    //accept connection
    new_sock = accept(sock,(struct sockaddr *)&client,(socklen_t*)sizeof (struct sockaddr_in));
    if(new_sock<0)
    {
        perror("connection failed");
    }
    puts("connection succesful");

    write(new_sock,"hallo und tschüss",255);  //message

    return 0;
}
