#include "funkcje.h"

int main(){

    printf("Komunikat 1");

    if(fork()){

        printf("Jestem procesem rodzicem, wykonuje sie w petli jako 1. proces");
        printf("Moj PID to: %d",getpid());
        sleep(3);
        printf("(sleep) Dzialanie konczy proces o PID: %d", getpid());
    }
    else{
        printf("Jestem procesem dzieckiem, wykonuje sie w petli jako 2. proces");
        printf("Moj PID to: %d",getpid());
        printf("Dzialanie konczy proces o PID: %d", getpid());
    }



    return 0;
}