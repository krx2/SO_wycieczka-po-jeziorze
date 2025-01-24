#include <iostream>
#include <time.h>
#include "classes.h"

using namespace std;

int main() {

    SharedMem memory(1234, 1024);
    memory.shm_attach();
    int* pamiec = memory.shm_get();

    MsgQueue kolejka_komunikatow(1234);
    kolejka_komunikatow.msg_attach();

    Sem semafor(1234);
    semafor.sem_attach();

    semafor.sem_op(0, 0); // czekanie na start symulacji

    printf("Policjant działa\n");

    

    memory.shm_detach(pamiec);
}