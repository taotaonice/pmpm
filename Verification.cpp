//
// Created by taotao on 2021/4/2.
//

#include "Verification.h"

VerServer::VerServer(){
    name2sec.clear();
}

void VerServer::watch_dog(){
    while (1){
        sleep(dog_period);
        mutex.lock();
        printf("dog has a look.\n");
        long now = get_sec();
        for(auto i=name2sec.begin(); i!=name2sec.end(); ){
            printf("dog watch %s %d\n", i->first.c_str(), i->second);
            if(now - i->second > max_wait_time){
                i = name2sec.erase(i);
                printf("erase %s.\n", i->first.c_str());
            }
            else i++;
        }
        mutex.unlock();
    }
}

long VerServer::get_sec(){
    timeval t;
    gettimeofday(&t, NULL);
    return timeval2sec(t);
}

long VerServer::timeval2sec(const timeval t){
    return t.tv_sec;
}

bool VerServer::check_in(std::string name){
    mutex.lock();
    bool in = name2sec.count(name);
    mutex.unlock();
    return in;
}

void VerServer::thread_fun(int cli){
    char buf[1024];
    while(1){
        int ret = recv(cli, (unsigned char*)buf, 1024, 0);
        if(ret <= 0){
            break;
        }
        printf("get a name %s\n", buf);
        std::string name = buf;
        mutex.lock();
        name2sec[name] = get_sec();
        mutex.unlock();
    }
    close(cli);
}

void VerServer::run() {
    new std::thread(&VerServer::watch_dog, this);
    sockaddr_in server_sockaddr, client_sockaddr;
    /*create a socket.type is AF_INET,sock_stream*/
    if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Creating Server ERROR.");
        exit(1);
    }

    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(3080);
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int server_len = sizeof(server_sockaddr);

    int on = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    /*bind a socket or rename a sockt*/
    if (bind(listener, (struct sockaddr *) &server_sockaddr, server_len) == -1) {
        printf("bind error");
        exit(1);
    }

    if (listen(listener, 50) == -1) {
        printf("listen error");
        exit(1);
    }

    socklen_t client_len = sizeof(client_sockaddr);

    while (1) {
        printf("listening ...\n");
        int client_sockfd;
        if ((client_sockfd = accept(listener, (struct sockaddr *) &client_sockaddr, &client_len)) == -1) {
            printf("accept error");
            exit(1);
        } else {
            printf("accepted.\n");
            char buf[1024];
            int ret = recv(client_sockfd, (unsigned char*)buf, 1024, 0);
            if (ret <= 0){
                close(client_sockfd);
                continue;
            }
            buf[ret] = 0;
            std::string name = buf;
            if(check_in(name)){
                char ans = 0;
                send(client_sockfd, (unsigned char*)&ans, 1, MSG_NOSIGNAL);
                close(client_sockfd);
            }
            else{
                mutex.lock();
                name2sec[name] = get_sec();
                mutex.unlock();
                char ans = 1;
                send(client_sockfd, (unsigned char*)&ans, 1, MSG_NOSIGNAL);
                new std::thread(&VerServer::thread_fun, this, client_sockfd);
            }
        }
    }
}
VerServer::~VerServer(){}

VerClient::VerClient(char* server_ip, char* cli_name){
    ip = server_ip;
    name = cli_name;
}

void VerClient::thread_fun(int sock){
    while (1){
        int ret = send(sock, name, strlen(name), MSG_NOSIGNAL);
        printf("send a name.\n");
        if(ret <= 0) break;
        sleep(sleep_time);
    }
}

bool VerClient::verify(){
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(3080);
    server_addr.sin_addr.s_addr = inet_addr(ip);
    bzero(&(server_addr.sin_zero),sizeof(server_addr.sin_zero));
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(sock, (struct sockaddr *)&server_addr,sizeof(struct sockaddr_in)) == -1){
        perror("connecting to server error.\n");
        close(sock);
        return false;
    }
    int len = strlen(name);
    int ret = send(sock, name, len, MSG_NOSIGNAL);
    if(ret <= 0) return false;
    char result[1];
    ret = recv(sock, result, 1, MSG_NOSIGNAL);
    if(ret <= 0) return false;
    if(result[0]){
        std::thread *t = new std::thread(&VerClient::thread_fun, this, sock);
        t->detach();
    }

    return result[0];
}
VerClient::~VerClient(){}
//int main(){
//    VerServer server;
//    server.run();
//    return 0;
//}
