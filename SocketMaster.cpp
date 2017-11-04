//
// Created by Xiaolong Chen on 04/11/2017.
//

#include "SocketMaster.h"
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <unistd.h>
#define LENGTH_OF_LISTEN_QUEUE 20

int SocketMaster::clientGetConnection(const char * serverIP, int portNumber) { //创建socket fd给用户, 好像一个文件指针似的可以直接使用
    //创建用于TCP协议的client_socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);  //创建socket
    if (client_socket < 0) {
        printf("Create Socket Failed!\n");
        exit(1);
    }

    /*server_addr*/
    //设置一个socket地址结构server_addr，代表服务器的internet地址，端口
    struct sockaddr_in server_addr;  // 要连接的服务器的地址
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if (inet_aton(serverIP, &server_addr.sin_addr) == 0) { //inet_aton: 把arg1转化为二进制, 然后交付给arg2, 返回值为
        printf("Server IP Address Error!\n");
        exit(1);
    }
    server_addr.sin_port = htons(portNumber);
    socklen_t server_addr_length = sizeof(server_addr);

    //向服务器发起连接
    if (connect(client_socket, (struct sockaddr *) &server_addr, server_addr_length) < 0) {  //使得刚创立好的fd成为客户端与服务器交流的接口
        printf("Can Not Connect To %s!\n", serverIP);
        exit(1);
    }
    return 0;
}

void SocketMaster::clientCloseConnection() {
    close(client_socket);
    printf("client_socket closed.\n");
}

int SocketMaster::serverBindAndListen(int portNumber) {
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(portNumber);

    //创建基于TCP协议的socket
    server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        printf("Create Socket Failed!\n");
        _exit(1);
    }
    {
        int opt = 1;
        setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    }

    //把socket fd和socket地址结构联系起来
    if (bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr))) {
        printf("Server bind port: %d failed\n", portNumber);
        _exit(1);
    }
    //监听客户端的请求
    if (listen(server_socket, LENGTH_OF_LISTEN_QUEUE)) {
        printf("Server Listen Failed!");
        _exit(1);
    }
    printf("Server starts listening to port %d...\n", portNumber);
    return 0;
}

int SocketMaster::serverAccept() {
    //define client socket addr
    struct sockaddr_in client_addr;
    socklen_t length = sizeof(client_addr);
    int new_server_socket = accept(server_socket, (struct sockaddr *) &client_addr, &length);
    if (new_server_socket < 0) {
        //fail
        printf("Server accept failed\n");
    }
    printf("Server accpet a new connection request. A corresponding Unix-fd number %d is created.\n", new_server_socket);
    return new_server_socket;
}

void SocketMaster::serverStopService() {
    close(server_socket);
    printf("server_socket closed.\n");
}

void SocketMaster::printBits(size_t const size, void const *const ptr) {
    unsigned char *b = (unsigned char *) ptr;
    unsigned char byte;
    int i, j;

    for (i = size - 1; i >= 0; i--) {
        for (j = 7; j >= 0; j--) {
            byte = (b[i] >> j) & 1;
            printf("%u", byte);
        }
    }
    puts("");  //append a '\n'
}