//
// Created by taotao on 2021/4/7.
//

#include "Verification.h"

int main(){
    VerClient client("127.0.0.1", "zt");
    bool result = client.verify();
    if(result){
        printf("verified.\n");
    }
    else{
        printf("denied.\n");
    }

    while (1){
        sleep(1);
    }

    return 0;
}