//
// Created by Xiaolong Chen on 01/11/2017.
//

#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "SocketMaster.h"
#define HELLO_WORLD_SERVER_PORT 6666 //使用的端口号
#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE 512


void clearBuffer(char *buffer) {
    bzero(buffer, BUFFER_SIZE);
}

/*
 * 需要使用的recv和send操作范例:
 * length = recv(new_server_socket, buffer, BUFFER_SIZE, 0);  // received client-msg is stored in buffer
 * length = send(new_server_socket, buffer, sizeof(buffer), 0);  // send server-msg in buffer to client
 * 我们总是默认client发第一个消息(比如发用户名过来), 然后server进行答复或者提问
 * */
int serverWork(int new_server_socket) {
    socklen_t length = 0;
    int tag;
    char buffer[BUFFER_SIZE];
    bzero(buffer, BUFFER_SIZE);
    //默认是client发第一个消息, 然后server进行答复或者提问
    length = recv(new_server_socket, buffer, BUFFER_SIZE, 0);  //received msg is stored in buffer
    if (length < 0) {
        printf("Server receive of username failed\n");
        return -1;
    }
    char name[FILE_NAME_MAX_SIZE + 1];  // +1 为了放'\0'
    bzero(name, FILE_NAME_MAX_SIZE + 1);
    strncpy(name, buffer,
            strlen(buffer) > FILE_NAME_MAX_SIZE ? FILE_NAME_MAX_SIZE : strlen(buffer));  //只复制放得下的部分
    printf("%s is requesting...\n", name);

    FILE *fp = fopen("tag.txt", "r+");
    if (NULL == fp) {
        printf("File: tag.txt cannot be open\n");
        _exit(1);
    }
    fscanf(fp, "%d", &tag);
    bzero(buffer, BUFFER_SIZE);
    if (tag == 1) {  // 商品没了
        strcpy(buffer, "Seckilling is over");
        printf("%s\n", buffer);
        length = send(new_server_socket, buffer, sizeof(buffer), 0); //tag = 1, 秒杀结束, 发给客户端
        fclose(fp);
    } else {  // 还有商品
        strcpy(buffer, "Please answer the question: 3 + 5 = ?\n Input your answer:\n");
        printf("Sending question to the client...\n");
        length = send(new_server_socket, buffer, sizeof(buffer), 0);
        bzero(buffer, BUFFER_SIZE);
        length = recv(new_server_socket, buffer, BUFFER_SIZE, 0);
        while (strcmp(buffer, "8") != 0) {
            char ans[5];
            strcpy(ans, buffer);
            bzero(buffer, BUFFER_SIZE);
            printf("Answer %s from user %s is wrong!\n", ans, name);
            int n = sprintf(buffer, "Your answer %s is wrong. Please send a new one: \n", ans);
            send(new_server_socket, buffer, n, 0);
            bzero(buffer, BUFFER_SIZE);
            length = recv(new_server_socket, buffer, BUFFER_SIZE, 0);
        }
        printf("Answer %s from user %s is right!\n", buffer, name);
        clearBuffer(buffer);
        strcpy(buffer, "right");
        send(new_server_socket, buffer, sizeof(buffer), 0);  // tell user that he gets right anwer

        fseek(fp, 0, SEEK_SET);  // fseek(fp, offset, offset_origin) , 从offset_origin后数offset个位置开始写入
        fscanf(fp, "%d", &tag);  // read from file
        clearBuffer(buffer);
        if (tag == 1) {  //发现已经没了
            strcpy(buffer, "Seckilling is over!\n");
            send(new_server_socket, buffer, sizeof(buffer), 0);
        } else {  // 还在
            tag = 1;
            fseek(fp, 0, SEEK_SET);
            fprintf(fp, "%d", tag);
            strcpy(buffer, "Seckilling succeed.\n");
            send(new_server_socket, buffer, sizeof(buffer), 0);
        }
        fclose(fp);
    }
    return 0;
}

void reaper(int sig) {
    int status;
    //调用wait3读取子进程的返回值, 使僵死状态的子进程彻底释放
    while (wait3(&status, WNOHANG, (struct rusage *) 0) >= 0);
}
int main(int argc, char **argv) {
    SocketMaster master;
    master.serverBindAndListen(HELLO_WORLD_SERVER_PORT);  // bind & listen

    //inform OS to run reaper as soon as it gets signal of child process exit...
    (void) signal(SIGCHLD, reaper);

    while (true) { //服务端是一直运行着一个死循环
        //accept返回一个新的socket fd, 代表着与客户端通信的连接, 也是一个文件
        int new_server_socket = master.serverAccept();
        int child_process_pid = fork();  //开启子进程
        if (child_process_pid == 0) {  // 子进程专门用来和具体一个客户端进行通信
            int res = serverWork(new_server_socket);
            if (res == -1) {
                break;
            }
        } else { // father process
//            close(server_socket);  // close fd for father
        }
    }

    //close listening socket
    master.serverStopService();
    return 0;
}