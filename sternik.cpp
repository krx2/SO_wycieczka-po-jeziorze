#include <iostream>
#include "classes.h"

using namespace std;

#define K 10
#define N1 30
#define N2 25
#define T1 1
#define T2 2

volatile sig_atomic_t stop_requested = 0; // Flaga do zatrzymania pętli

pid_t pid_sternik1;
pid_t pid_sternik2;

void signal_handler(int sig) {
    if (sig == SIGUSR1 || sig == SIGUSR2 || sig == SIGINT || sig == SIGSTOP) {
        kill(pid_sternik1, SIGINT);
        kill(pid_sternik2, SIGINT);
        stop_requested = 1; // Ustawienie flagi końca działania
    }
}

void sternik(int nr) {
    // Proces potomny: sternik1
        SharedMem pamiec_dzielona(1234, 1024);
        pamiec_dzielona.shm_attach();
        int* pamiec = pamiec_dzielona.shm_get();

        MsgQueue kolejka_komunikatow(1234);
        kolejka_komunikatow.msg_attach();

        Sem semafor(1234);
        semafor.sem_attach();

        printf("Sternik1 działa!\n");

        kolejka_komunikatow.msg_rcv(20+nr); // Tp

        pamiec[1+nr] = 0;
        pamiec[3+nr] = 0;

        while(!stop_requested) {
            printf("Proces sternik %d obsługuje łódź %d\n", getpid(), nr);

            // Najpierw wyładunek
            printf("Kapitan: Rozpoczynam wyładunek!\n");

            while (pamiec[1+nr] > 0) { // pamiec[1+nr] - liczba osób na łodzi
                kolejka_komunikatow.msg_send(15+nr); // sygnał na opuszczenie pokładu
                kolejka_komunikatow.msg_rcv(17+nr); // czeka aż pasażer zejdzie
                printf("wyładunek: Łódź 1: %d/%d\tPomost 1: %d/%d\n", pamiec[1+nr], N1, pamiec[3+nr], K);
                //sleep(1);
            }

            // Załadunek
            printf("Rozpoczynam załadunek!\n");
            while (pamiec[5+nr] > 0 && pamiec[1+nr] < N1) { // najpierw załadunek vipów, pamiec[5+nr] - liczba czekających vipów
                kolejka_komunikatow.msg_send(9+nr);
                printf("załadunek: Łódź 1: %d/%d\tPomost 1: %d/%d\n", pamiec[1+nr], N1, pamiec[3+nr], K);
            }

            while (pamiec[1+nr] < N1) {
                kolejka_komunikatow.msg_send(11+nr); // załadunek pasażerów, proszę wejść na pomost
                printf("wysłano 10, czekam na 11\n");
                kolejka_komunikatow.msg_rcv(13+nr); // czeka aż pasażer wejdzie
                printf("załadunek: Łódź 1: %d/%d\tPomost 1: %d/%d\n", pamiec[1+nr], N1, pamiec[3+nr], K);
                //sleep(1);
            }

            // Rejs
            sleep(T1);
        }
        printf("[STERNIK%d] Następny rejs się nie odbędzie z powodu SIGUSR1\n", nr);
        pamiec_dzielona.shm_detach(pamiec);
        exit(0);
}

int main() {
    
    pid_t pid_sternik = getpid();

    SharedMem pamiec_dzielona(1234, 1024);
    pamiec_dzielona.shm_attach();
    int* pamiec = pamiec_dzielona.shm_get();

    MsgQueue kolejka_komunikatow(1234);
    kolejka_komunikatow.msg_attach();

    Sem semafor(1234);
    semafor.sem_attach();

    semafor.sem_op(0, -1); // start symulacji po zainicjowaniu procesów

    pid_t pid_sternik1 = fork();
    if (pid_sternik1 < 0) {
        error("fork sternik1");
    } else if (pid_sternik1 == 0) {
        sternik(0);
    }

    pid_t pid_sternik2 = fork();
    if (pid_sternik2 < 0) {
        error("fork sternik2");
    } else if (pid_sternik2 == 0) {
        sternik(1);
    }

    // Proces macierzysty czeka na zakończenie potomków lub dalsze sygnały
    pause();
    return 0;
}