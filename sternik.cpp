#include <iostream>
#include "classes.h"

using namespace std;

#define K 10
#define N1 30
#define N2 25
#define T1 1
#define T2 2

int main() {
    
    pid_t pid_sternik = getpid();

    SharedMem pamiec_dzielona(1234, 1024);
    pamiec_dzielona.shm_attach();
    int* pamiec = pamiec_dzielona.shm_get();

    MsgQueue kolejka_komunikatow(1234);
    kolejka_komunikatow.msg_attach();

    kolejka_komunikatow.msg_rcv(33);
    pamiec[1] = getpid();
    kolejka_komunikatow.msg_send(34); // daje swój pid policjantowi

    pid_t pid_sternik1 = fork();
    if (pid_sternik1 < 0) {
        error("fork sternik1");
    }
    
    if(pid_sternik1 == 0) {
        SharedMem pamiec_dzielona(1234, 1024);
        pamiec_dzielona.shm_attach();
        int* pamiec = pamiec_dzielona.shm_get();

        MsgQueue kolejka_komunikatow(1234);
        kolejka_komunikatow.msg_attach();

        Sem semafor(1234);
        semafor.sem_attach();

        printf("Sternik1 działa!\n");

        kolejka_komunikatow.msg_rcv(35);
        pamiec[1] = getpid();
        kolejka_komunikatow.msg_send(36); // daje swój pid policjantowi

        semafor.sem_op(0, 0); // czekanie na start symulacji

        semafor.sem_op(1, 0); // czekanie na godzinę Tp

        while(semafor.sem_get_value(SIGUSR1) != SIGUSR1) {

            
            // Najpierw wyładunek
            printf("Kapitan: Rozpoczynam wyładunek!\n");
            while(pamiec[1] > 0) { // pamiec[1] - liczba osób na łodzi 1
                kolejka_komunikatow.msg_send(12); // sygnał na opuszczenie pokładu
                printf("Łódź 1: %d/%d\tPomost 1: %d/%d\n", pamiec[1], N1, pamiec[3], K);
            }

            // Załadunek
            printf("Rozpoczynam załadunek!\n");
            while(pamiec[5] > 0 && pamiec[1] < N1) { // najpierw załadunek vipów, pamiec[5] - liczba czekających vipów
                kolejka_komunikatow.msg_send(9);
            }

            while(pamiec[1] < N1) {
                kolejka_komunikatow.msg_send(10); // załadunek pasażerów, proszę wejść na pomost
                printf("Łódź 1: %d/%d\tPomost 1: %d/%d\n", pamiec[1], N1, pamiec[3], K);
                if(pamiec[3] > 0) { // jeśli ktoś jest na pomoście to niech wejdzie na statek
                    kolejka_komunikatow.msg_send(11);
                }
            }

            //Rejs
            sleep(T1);
            raise(SIGKILL);
        }
        printf("[STERNIK1] Następny rejs się nie odbędzie z powodu SIGUSR1\n");
        pamiec_dzielona.shm_detach(pamiec);
    }

    pid_t pid_sternik2 = fork();
    if (pid_sternik2 < 0) {
        error("fork sternik2");
    }

    if(pid_sternik2 == 0) {
        SharedMem pamiec_dzielona(1234, 1024);
        pamiec_dzielona.shm_attach();
        int* pamiec = pamiec_dzielona.shm_get();

        MsgQueue kolejka_komunikatow(1234);
        kolejka_komunikatow.msg_attach();

        kolejka_komunikatow.msg_rcv(37);
        pamiec[1] = getpid();
        kolejka_komunikatow.msg_send(38); // daje swój pid policjantowi

         printf("Sternik2 działa!\n");
    }

    pause();

}