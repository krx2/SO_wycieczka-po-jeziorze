#include <iostream>
#include <time.h>
#include "classes.h"

using namespace std;



int* pamiec;

void signal_handler(int sig) {

    if(sig == SIGUSR1) { // tk
        kill(pamiec[5], SIGINT);
    }
    else if(sig == SIGINT) {
        printf("\033[36m[POLICJANT]\033[0m: Kończenie działania...\n");
        kill(pamiec[7], SIGINT);
        if (shmdt(pamiec) == -1) error("shmdt error");
        exit(EXIT_SUCCESS);
    }
}

int main() {

    signal(SIGINT, signal_handler);
    signal(SIGUSR1, signal_handler);

    MsgQueue kolejka_komunikatow(1234);
    kolejka_komunikatow.msg_attach();

    Sem semafor(1234);
    semafor.sem_attach();

    SharedMem pamiec_dzielona(1234, 1024);
    pamiec_dzielona.shm_attach();
    pamiec = pamiec_dzielona.shm_get();

    pamiec[6] = getpid();

    semafor.sem_op(0, -1); // start symulacji po zainicjowaniu procesów

    

    sleep(Tp);
    
    kolejka_komunikatow.msg_send(20); // Tp1
    kolejka_komunikatow.msg_send(21); // Tp2


    //printf("czekanie na 23\n");
    kolejka_komunikatow.msg_rcv(23);
    //printf("czekanie na 24\n");
    kolejka_komunikatow.msg_rcv(24);
    kolejka_komunikatow.msg_rcv(25);
    //printf("czekanie na 27\n");
    kolejka_komunikatow.msg_rcv(27);

    pid_t pid_sternik1 = pamiec[3];
    pid_t pid_sternik2 = pamiec[4];
    pid_t pid_mainp = pamiec[7];

    char c;

    printf("\033[36m[POLICJANT]\033[0m: start\n");
    
    while (1) {
        cin >> c;

        //printf("[POLICJANT]: Przechwycono %c\n", c);


        if(c == '1') {
            //printf("c = 1\n");

            printf("\033[36m[POLICJANT]\033[0m: Wysyłanie SIGUSR1 do %d\n", pid_sternik1);
            kill(pid_sternik1, SIGUSR1);
        } else if(c == '2') {
            printf("\033[36m[POLICJANT]\033[0m: Wysyłanie SIGUSR2 do %d\n", pid_sternik2);
            kill(pid_sternik2, SIGUSR2);
        } else if(c == 'c') {
            printf("\033[36m[POLICJANT]\033[0m: Wysyłanie SIGINT\n");
            kill(pid_mainp, SIGINT);
        }

        //printf("po ifie\n");
    }
    
}