#include <iostream>
#include "classes.h"

using namespace std;

#define K 10
#define N1 30
#define N2 25

int main() {
    
    pid_t pid_sternik = getpid();

    pid_t pid_sternik1 = fork();
    if (pid_sternik1 < 0) {
        error("fork policjant");
    }

    pid_t pid_sternik2;
    if(getpid() == pid_sternik) {
        pid_t pid_sternik1 = fork();
        if (pid_sternik1 < 0) {
            error("fork policjant");
        }
    }

    while(true) {
        if(getpid() == pid_sternik1) {
            SharedMem pamiec_dzielona(1234, 1024);
            pamiec_dzielona.shm_attach();
            int* pamiec = pamiec_dzielona.shm_get();

            MsgQueue kolejka_komunikatow(1234);
            kolejka_komunikatow.msg_attach();

            Sem semafor(1234);
            semafor.sem_attach();

            semafor.sem_op(0, 0); // czekanie na start symulacji

            semafor.sem_op(1, 0); // czekanie na godzinę Tp


            // Najpierw wyładunek
            while(pamiec[1] > 0) { // pamiec[1] - liczba osób na łodzi 1
                if(pamiec[3] > K - 1) { // pamiec[3] - liczba osób na pomoście 1
                    semafor.sem_op(1, 1); // podniesienie semafora i czekanie na miejsce na pomoście
                } 
                pamiec[1]--;
                pamiec[3]++;
                kolejka_komunikatow.msg_send(13); // sygnał na opuszczenie pokładu
            }

            // Załadunek

            while(pamiec[5] > 0 && pamiec[1] < N1) { // najpierw załadunek vipów, pamiec[5] - liczba czekających vipów
                pamiec[5]--;
                pamiec[1]++;
                kolejka_komunikatow.msg_send(9);
            }
        }

        if(getpid() == pid_sternik2) {
            SharedMem pamiec_dzielona(1234, 1024);
            pamiec_dzielona.shm_attach();
            int* pamiec = pamiec_dzielona.shm_get();

            MsgQueue kolejka_komunikatow(1234);
            kolejka_komunikatow.msg_attach();

            kolejka_komunikatow.msg_rcv(14); // sygnał godzina Tp

            // Wyładunek
            // pamiec[1] - liczba osób na łodzi 1, pamiec[3] - liczba osób na pomoście 1
            while(pamiec[2] > 0 && pamiec[4] < 10) { 
                pamiec[2]--;
                pamiec[4]++;
                kolejka_komunikatow.msg_send(16); // sygnał na opuszczenie pokładu
            }
        }
    }
}