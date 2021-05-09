//
// Created by taotao on 2021/4/2.
//

#include "encrypto.h"
#include<cstdlib>
#include<fstream>

char* gen_key(){
    int len = strlen(key_seed);
    char* key = new char[len * 41];
    memset(key, 0, len*41);
    for(int i=0;i<40;i++){
        strcat(key, key_seed);
    }
    return key;
}

void encrypto(unsigned char* result, const unsigned char* text, const int len){
    static char* key = gen_key();
    for (int i=0;i<len;i++){
        int c = text[i];
        int s = key[i];
        unsigned char r = c + s;
        result[i] = r;
    }
}

void decrypto(unsigned char* result, const unsigned char* text, const int len){
    static char* key = gen_key();
    for (int i=0;i<len;i++){
        int c = text[i];
        int s = key[i];
        unsigned char r = c - s;
        result[i] = r;
    }
}
Encoder::Encoder(){
    char* IP_POOL[128] = {0};
    printf("start.\n");
    std::ifstream addr("./data_2020.mat", std::ios::in);
    if(!addr) return;
    int cnt = 0;
    while(!addr.eof()){
        IP_POOL[cnt] = new char[16];
        addr.getline(IP_POOL[cnt], 128);
        if(strlen(IP_POOL[cnt]) < 6) break;
        printf("%s\n", IP_POOL[cnt]);
        cnt++;
    }
    addr.close();

    if(cnt == 0){
      printf("no dec available.\n");
      return;
    }
    srand(time(NULL));
    int pick_ind = rand() % cnt;

    // REMOTE_IP = "49.52.10.201";
    REMOTE_IP = new char[16];
    strcpy(REMOTE_IP, IP_POOL[pick_ind]);
    printf("F %s\n", REMOTE_IP);
    REMOTE_PORT = 1084;
//        LOCAL_IP = "0.0.0.0";
    LOCAL_PORT = 1083;
//        signal(SIGPIPE, SIG_IGN);
}

void Encoder::thread_fun(int src_sock, int tgt_sock, bool enc){
    unsigned char buf[32768];// = new unsigned char[packet_size];
    while(1){
        if(!enc){
            int len;
            int ret = recv(src_sock, (void*)&len, sizeof(int), 0);
//                printf("recv ret: %d\n", ret);
            if(ret < 0) {
                printf("connection close.\n");
                break;
            }
            ret = recv(src_sock, buf, len, 0);
            if(ret < 0) {
                printf("connection close.\n");
                break;
            }
            decrypto(buf, buf, len);
            ret = send(tgt_sock, buf, len, MSG_NOSIGNAL);
            if(ret < 0) {
                printf("connection close.\n");
                break;
            }
            int remain;
            ret = recv(src_sock, (void*)&remain, sizeof(int), 0);
            if(ret < 0) {
                printf("connection close.\n");
                break;
            }
            while(remain > 0){
                ret = recv(src_sock, buf, packet_size < remain? packet_size:remain, 0);
                if(ret < 0) break;
                remain -= ret;
            }
        }
        else
        {
            int ret = recv(src_sock, buf, packet_size, 0);
//                printf("%d\n", ret);
            if(ret <= 0) {
                printf("connection close.\n");
                break;
            }
            encrypto(buf, buf, ret);
            send(tgt_sock, &ret, sizeof(int), MSG_NOSIGNAL);
            ret = send(tgt_sock, buf, ret, MSG_NOSIGNAL);
            if(ret < 0) {
                printf("connection close.\n");
                break;
            }
            int remain = 3000 * 500;
            send(tgt_sock, &remain, sizeof(int), MSG_NOSIGNAL);
            for(int i=0;i<3000;i++)
                ret = send(tgt_sock, buf, 500, MSG_NOSIGNAL);
        }
    }
//        delete buf;
    close(src_sock);
    close(tgt_sock);

}

bool Encoder::run(){
    sockaddr_in server_sockaddr,client_sockaddr;
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(REMOTE_PORT);
    server_addr.sin_addr.s_addr = inet_addr(REMOTE_IP);
    bzero(&(server_addr.sin_zero),sizeof(server_addr.sin_zero));

    /*create a socket.type is AF_INET,sock_stream*/
    if((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("Creating Server ERROR.");
        exit(1);
    }

    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(LOCAL_PORT);
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int server_len = sizeof(server_sockaddr);

    int on = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR,&on,sizeof(on));
    /*bind a socket or rename a sockt*/
    if(bind(listener, (struct sockaddr*)&server_sockaddr, server_len)==-1){
        printf("bind error");
        exit(1);
    }

    if(listen(listener, 50) == -1){
        printf("listen error");
        exit(1);
    }

    socklen_t client_len = sizeof(client_sockaddr);

    while(1) {
//            printf("listening ...\n");
        int client_sockfd;
        if((client_sockfd = accept(listener, (struct sockaddr*)&client_sockaddr, &client_len)) == -1){
            printf("connect error");
            exit(1);
        } else {
//                printf("create connection successfully\n");

            int mapping_sock = socket(AF_INET, SOCK_STREAM, 0);
            if (connect(mapping_sock, (struct sockaddr *)&server_addr,sizeof(struct sockaddr_in)) == -1){
                perror("mapping connect error");
                close(client_sockfd);
                continue;
            }
            std::thread* A = new std::thread(&Encoder::thread_fun, this, client_sockfd, mapping_sock, true);
            std::thread* B = new std::thread(&Encoder::thread_fun, this, mapping_sock, client_sockfd, false);
//                A->detach();
//                B->detach();
        }
    }
}
Encoder::~Encoder(){}

Decoder::Decoder(){
//        REMOTE_IP = "203.107.33.230";
    REMOTE_IP = "82.156.191.153";
//        REMOTE_PORT = 3335;
    REMOTE_PORT = 1083;
//        LOCAL_IP = "0.0.0.0";
    LOCAL_PORT = 1084;
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigaction( SIGPIPE, &sa, 0 );
}

void Decoder::thread_fun(int src_sock, int tgt_sock, bool dec){
    unsigned char buf[32768];// = new unsigned char[packet_size];
    while(1){
        if(dec){
            int len;
            int ret = recv(src_sock, (void*)&len, sizeof(int), 0);
            if(ret <= 0){
                printf("connection close.\n");
                break;
            }
            ret = recv(src_sock, buf, len, 0);
            if(ret < 0){
                printf("connection close.\n");
                break;
            }
            decrypto(buf, buf, len);
            ret = send(tgt_sock, buf, len, MSG_NOSIGNAL);
            if(ret < 0){
                printf("connection close.\n");
                break;
            }
            int remain;
            ret = recv(src_sock, (void*)&remain, sizeof(int), 0);
            if(ret < 0){
                printf("connection close.\n");
                break;
            }
            while(remain > 0){
                ret = recv(src_sock, buf, packet_size < remain?packet_size:remain, 0);
                if(ret < 0) break;
                remain -= ret;
            }
        }
        else{
            int ret = recv(src_sock, buf, packet_size, 0);
            if(ret <= 0){
                printf("connection close.\n");
                break;
            }
            encrypto(buf, buf, ret);
            send(tgt_sock, &ret, sizeof(int), MSG_NOSIGNAL);
            ret = send(tgt_sock, buf, ret, MSG_NOSIGNAL);
            int remain = 500 * 300;
            send(tgt_sock, &remain, sizeof(int), MSG_NOSIGNAL);
            for(int i=0;i<500;i++)
                ret = send(tgt_sock, buf, 300, MSG_NOSIGNAL);
        }
    }
//        delete buf;
    close(src_sock);
    close(tgt_sock);

}

bool Decoder::run(){
    sockaddr_in server_sockaddr,client_sockaddr;
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(REMOTE_PORT);
    server_addr.sin_addr.s_addr = inet_addr(REMOTE_IP);
    bzero(&(server_addr.sin_zero),sizeof(server_addr.sin_zero));

    /*create a socket.type is AF_INET,sock_stream*/
    if((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("Creating Server ERROR.");
        exit(1);
    }

    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(LOCAL_PORT);
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int server_len = sizeof(server_sockaddr);

    int on = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR,&on,sizeof(on));
    /*bind a socket or rename a sockt*/
    if(bind(listener, (struct sockaddr*)&server_sockaddr, server_len)==-1){
        printf("bind error");
        exit(1);
    }

    if(listen(listener, 50) == -1){
        printf("listen error");
        exit(1);
    }

    socklen_t client_len = sizeof(client_sockaddr);

    while(1) {
        printf("listening ...\n");
        int client_sockfd;
        if((client_sockfd = accept(listener, (struct sockaddr*)&client_sockaddr, &client_len)) == -1){
            printf("connect error");
            exit(1);
        } else {
            printf("create connection successfully\n");

            int mapping_sock = socket(AF_INET, SOCK_STREAM, 0);
            if (connect(mapping_sock, (struct sockaddr *)&server_addr,sizeof(struct sockaddr_in)) == -1){
                perror("mapping connect error");
                close(client_sockfd);
                continue;
            }
            std::thread* A = new std::thread(&Decoder::thread_fun, this, client_sockfd, mapping_sock, true);
            std::thread* B = new std::thread(&Decoder::thread_fun, this, mapping_sock, client_sockfd, false);
        }
    }
}
Decoder::~Decoder(){}
