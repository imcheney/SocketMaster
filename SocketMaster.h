//
// Created by Xiaolong Chen on 04/11/2017.
//

#ifndef CLIENTSERVER01_SOCKETCONNECTWIZ_H
#define CLIENTSERVER01_SOCKETCONNECTWIZ_H

#include <stdio.h>

class SocketMaster {
public:  //client
    int clientGetConnection(const char * serverIP, int portNumber);
    void clientCloseConnection();

public:  //server
    int serverBindAndListen(int portNumber);
    int serverAccept();
    void serverStopService();
    void printBits(size_t const size, void const *const ptr);

public:
    int client_socket;
    int server_socket;
};


#endif //CLIENTSERVER01_SOCKETCONNECTWIZ_H
