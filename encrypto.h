//
// Created by taotao on 2021/4/2.
//

#ifndef PORTMAPPING_ENCRYPTO_H
#define PORTMAPPING_ENCRYPTO_H

#define key_seed "djq%5cu#-jeq15abg$z9_i#_w=$o88m!*alpbedlbat8cr74sd"

#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <thread>

//const char* key_seed = "djq%5cu#-jeq15abg$z9_i#_w=$o88m!*alpbedlbat8cr74sd";
void encrypto(unsigned char* result, const unsigned char* text, const int len);
void decrypto(unsigned char* result, const unsigned char* text, const int len);

class Encoder{
private:
    char *REMOTE_IP;
    int REMOTE_PORT;
//    char* LOCAL_IP;
    int LOCAL_PORT;
    const int packet_size = 2048;

    int listener;
public:
    Encoder();
    void thread_fun(int src_sock, int tgt_sock, bool enc);
    bool run();
    ~Encoder();
};

class Decoder{
private:
    char *REMOTE_IP;
    int REMOTE_PORT;
//    char* LOCAL_IP;
    int LOCAL_PORT;
    int packet_size = 2048;

    int listener;
public:
    Decoder();
    void thread_fun(int src_sock, int tgt_sock, bool dec);
    bool run();
    ~Decoder();
};
#endif //PORTMAPPING_ENCRYPTO_H
