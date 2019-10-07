#include <arpa/inet.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define BUF_SIZE 100
#define NAME_SIZE 20

void * send_msg(void * arg);
void * recv_msg(void * arg);
void error_handling(const char * msg);

static char name[NAME_SIZE]="[DEFAULT]";
static char msg[BUF_SIZE];

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in serv_addr;
    pthread_t snd_thread, rcv_thread;
    void * thread_return;
    if(argc!=4) {
        printf("Usage : %s <IP> <port> <name>\n", argv[0]);
        exit(1);
     }

    sprintf(name, "[%s]", argv[3]);
    sock=socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
    serv_addr.sin_port=htons(static_cast<uint16_t>(atoi(argv[2])));

    if(connect(sock, reinterpret_cast<struct sockaddr*>(&serv_addr), sizeof(serv_addr)) == -1) {
        error_handling("connect() error");
        return -1;
    }

    pthread_create(&snd_thread, nullptr, send_msg, static_cast<void*>(&sock));
    pthread_create(&rcv_thread, nullptr, recv_msg, static_cast<void*>(&sock));
    pthread_join(snd_thread, &thread_return);
    pthread_join(rcv_thread, &thread_return);
    close(sock);
    return 0;
}

void * send_msg(void * arg)   // send thread main
{
    int sock=*(reinterpret_cast<int*>(arg));
    char name_msg[NAME_SIZE+BUF_SIZE];
    while(1)
    {
        fgets(msg, BUF_SIZE, stdin);
        if(!strcmp(msg,"q\n")||!strcmp(msg,"Q\n"))
        {
            close(sock);
            exit(0);
        }
        sprintf(name_msg,"%s %s", name, msg);
        write(sock, name_msg, strlen(name_msg));
    }
}

void * recv_msg(void * arg)   // read thread main
{
    int sock=*(reinterpret_cast<int*>(arg));
    char name_msg[NAME_SIZE+BUF_SIZE];
    int str_len;
    while(1)
    {
        str_len = static_cast<int>(read(sock, name_msg, NAME_SIZE+BUF_SIZE-1));
        if(str_len == -1)
            return reinterpret_cast<void *>(-1);
        name_msg[str_len]=0;
        fputs(name_msg, stdout);
    }
}

void error_handling(const char *msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    return;
}
