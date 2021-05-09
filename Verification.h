//
// Created by taotao on 2021/4/2.
//

#ifndef PORTMAPPING_VERIFICATION_H
#define PORTMAPPING_VERIFICATION_H

#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <thread>
#include <signal.h>
#include <vector>
#include <map>
#include <mutex>
#include <sys/time.h>

class VerServer{
private:
    int listener;
    std::map<std::string, long> name2sec;
    std::mutex mutex;

    const long max_wait_time = 300;
    const int dog_period = 30;
public:
    VerServer();
    void watch_dog();
    long get_sec();
    long timeval2sec(const timeval t);
    bool check_in(std::string name);
    void thread_fun(int cli);
    void run();
    ~VerServer();
};

class VerClient{
private:
    char* ip;
    char* name;
    int sock;
    const int sleep_time = 60;
public:
    VerClient(char* server_ip="170.106.176.229", char* cli_name="zt");
    void thread_fun(int sock);
    bool verify();
    ~VerClient();
};
#endif //PORTMAPPING_VERIFICATION_H
