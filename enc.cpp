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

#include "encrypto.h"

using std::cout;
using std::endl;


int main() {
    Encoder encoder;
    encoder.run();
    return 0;
}
