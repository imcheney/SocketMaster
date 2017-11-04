#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SocketMaster.h"

#define HELLO_WORLD_SERVER_PORT    6666
#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE 512

/**
 * 需要使用的recv和send操作范例:
 * length = recv(new_server_socket, buffer, BUFFER_SIZE, 0);  // received client-msg is stored in buffer
 * length = send(new_server_socket, buffer, sizeof(buffer), 0);  // send server-msg in buffer to client
 * 我们总是默认client发第一个消息(比如发用户名过来), 然后server进行答复或者提问
 * 实现这个函数中的业务逻辑即可
 * @param master
 * @return
 */
int clientWork(SocketMaster master) {
    char ans[100];
    int length = 0;

    //连接成功后，让用户输入用户名，模拟淘宝登录
    char name[FILE_NAME_MAX_SIZE + 1];
    bzero(name, FILE_NAME_MAX_SIZE + 1);
    printf("Please Input your name: ");
    scanf("%s", name);
    send(master.client_socket, name, strlen(name), 0);
    char buffer[BUFFER_SIZE];
    bzero(buffer, BUFFER_SIZE);      //接收验证问题，模拟验证码
    length = recv(master.client_socket, buffer, BUFFER_SIZE, 0);
    do {
        printf("%s", buffer);  // 打印服务器的回复
        if (strcmp(buffer, "Seckilling is over") == 0) { //如果服务端回复的消息为“秒杀结束”，则关闭socket连接
            master.clientCloseConnection();
            return 0;
        }
        bzero(ans, 100);  // 清空ans
        scanf("%s", ans);  // 读入ans新的输入
        send(master.client_socket, ans, strlen(ans), 0);  //往服务端发送答案
        bzero(buffer, BUFFER_SIZE); //发送完毕清空buffer
        length = recv(master.client_socket, buffer, BUFFER_SIZE, 0);
    } while (strcmp(buffer, "right") != 0);

    //秒杀成功, 关闭连接
    printf("Answer is right!");
    bzero(buffer, BUFFER_SIZE);
    length = recv(master.client_socket, buffer, BUFFER_SIZE, 0);    // 读取一个回复
    printf("%s\n", buffer);
}

int main(int argc, char **argv) {
    /*输入参数校验*/
    if (argc != 2) {
        printf("Usage: %s ServerIP\n", argv[0]);  //提示输入错误, 应该是Usage：./programName ServerIP
        exit(1);
    }
    SocketMaster master;
    master.clientGetConnection(argv[1], HELLO_WORLD_SERVER_PORT);
    clientWork(master);
    master.clientCloseConnection();
    return 0;
}