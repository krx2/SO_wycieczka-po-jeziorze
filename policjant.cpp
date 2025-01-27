#include <iostream>
#include <time.h>
#include "classes.h"

using namespace std;

#define Tp 1

pid_t pids[6];

void signal_handler(int sig) {
    Sem semafor(1234);
    semafor.sem_attach();
    if(sig == SIGUSR1) {
        printf("Odebrano sygnał SIGUSR1\n");
        semafor.sem_set_value(SIGUSR1, SIGUSR1);
    } else if(sig == SIGUSR2) {
        semafor.sem_set_value(SIGUSR2, SIGUSR2);
        printf("Odebrano sygnał SIGUSR2\n");
    } else if(sig == SIGCONT) {
        kill(pids[2], sig);
        kill(pids[1], sig);
        kill(pids[4], sig);
        kill(pids[5], sig);
        kill(pids[3], sig);
    }
}

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
    kolejka_komunikatow.msg_send(21); // Tp1
    kolejka_komunikatow.msg_send(22); // Tp2

    sleep(3);
    
    pause();
    memory.shm_detach(pamiec);
}