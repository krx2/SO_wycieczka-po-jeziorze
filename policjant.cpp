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
        semafor.sem_set_value(SIGUSR1, SIGUSR1);
    } else if(sig == SIGUSR2) {
        semafor.sem_set_value(SIGUSR2, SIGUSR2); // bez pętli bo kolejność ma znaczenie
    } else if(sig == SIGCONT) {
        kill(pids[2], sig);
        kill(pids[1], sig);
        kill(pids[4], sig);
        kill(pids[5], sig);
        kill(pids[3], sig);
    }
}

int main() {

    struct sigaction sa;
    sa.sa_handler = signal_handler; // Funkcja obsługi sygnałów
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    SharedMem memory(1234, 1024);
    memory.shm_attach();
    int* pamiec = memory.shm_get();

    MsgQueue kolejka_komunikatow(1234);
    kolejka_komunikatow.msg_attach();

    Sem semafor(1234);
    semafor.sem_attach();

    semafor.sem_op(0, 0); // start symulacji po zainicjowaniu procesów

    printf("[POLICJANT] start\n");


    kolejka_komunikatow.msg_rcv(30); // czekaj na przekazanie pidu kasjera
    pids[1] = pamiec[0];
    kolejka_komunikatow.msg_send(31); // daje znać procesowi pasażer
    kolejka_komunikatow.msg_rcv(32); // czekanie na przekazanie pidu pasażera
    pids[2] = pamiec[0];
    kolejka_komunikatow.msg_send(33); // daje znać sternikowi
    kolejka_komunikatow.msg_rcv(34); // czekanie na przekazanie pidu sternika
    pids[3] = pamiec[0];
    kolejka_komunikatow.msg_send(35); // do sternik1
    kolejka_komunikatow.msg_rcv(36); // czeka na sternik1
    pids[4] = pamiec[0];
    kolejka_komunikatow.msg_send(37); // do sternik2
    kolejka_komunikatow.msg_rcv(38); // czeka na sternik2
    pids[5] = pamiec[0];

    pamiec[0] = 0; // resetuje pamiec0 bo ma wszystkie pidy

    printf("Policjant działa\n");


    sleep(Tp);
    semafor.sem_op(1, -1); // start sternika

    usleep(1);
    kill(pids[4], SIGKILL);
    

    memory.shm_detach(pamiec);
}