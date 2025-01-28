#include <iostream>
#include <time.h>
#include "classes.h"

using namespace std;

#define Tp 1

int main() {

    SharedMem memory(1234, 1024);
    memory.shm_attach();
    int* pamiec = memory.shm_get();

    MsgQueue kolejka_komunikatow(1234);
    kolejka_komunikatow.msg_attach();

    Sem semafor(1234);
    semafor.sem_attach();

    semafor.sem_op(0, -1); // start symulacji po zainicjowaniu procesów

    printf("[POLICJANT] start\n");

    sleep(Tp);
    kolejka_komunikatow.msg_send(20); // Tp1
    kolejka_komunikatow.msg_send(21); // Tp2

    sleep(3);
    
    pause();
    memory.shm_detach(pamiec);
}