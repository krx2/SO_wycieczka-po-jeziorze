#include <iostream>
#include <time.h>
#include "classes.h"

using namespace std;

#define Tp 1
#define Tk 2

int* pamiec;

void signal_handler(int sig) {
    if (shmdt(pamiec) == -1) error("shmdt error");
}

int main() {

    signal(SIGINT, signal_handler);

    MsgQueue kolejka_komunikatow(1234);
    kolejka_komunikatow.msg_attach();

    Sem semafor(1234);
    semafor.sem_attach();

    semafor.sem_op(0, -1); // start symulacji po zainicjowaniu procesów

    printf("[POLICJANT] start\n");

    sleep(Tp);
    kolejka_komunikatow.msg_send(20); // Tp1
    kolejka_komunikatow.msg_send(21); // Tp2
    
    sleep(Tk-Tp);

    printf("[POLICJANT]: Wysyłanie sygnału Tk do %d...\n", pamiec[7]);
    killpg(getpgrp(), SIGINT);
}