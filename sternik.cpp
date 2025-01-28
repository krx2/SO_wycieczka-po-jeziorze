#include <iostream>
#include "classes.h"

using namespace std;

#define K 10
#define N1 30
#define N2 25
#define T1 1
#define T2 2

#define VIP_QUEUE_SEM 5
#define LOADING_SEM 9
#define UNLOADING_SEM 7
#define DIRECTION_SEM 11
#define POMOST_SEM 3


volatile sig_atomic_t stop_requested = 0; // Flaga do zatrzymania pętli

pid_t pid_sternik1;
pid_t pid_sternik2;

void signal_handler(int sig) {
    if (sig == SIGUSR1 || sig == SIGUSR2) {
        stop_requested = 1; // Ustawienie flagi końca działania
    } else {
        kill(pid_sternik1, SIGINT);
        kill(pid_sternik2, SIGINT);
        stop_requested = 1; // Ustawienie flagi końca działania
    }
    stop_requested = 1; // Ustawienie flagi końca działania
}

void sternik(int nr) {
    // Proces potomny: sternik
        SharedMem pamiec_dzielona(1234, 1024);
        pamiec_dzielona.shm_attach();
        int* pamiec = pamiec_dzielona.shm_get();

        MsgQueue kolejka_komunikatow(1234);
        kolejka_komunikatow.msg_attach();

        Sem semafor(1234);
        semafor.sem_attach();

        int pojemnosc;

        if(nr == 0) {
            pojemnosc = N1;
        } else pojemnosc = N2;

        printf("[STERNIK%d]: działa!\n", 1+nr);

        kolejka_komunikatow.msg_rcv(20+nr); // Tp czas początkowy

        semafor.sem_set_value(3+nr, K); // semafor dla pomostu

        pamiec[1+nr] = 0; // ile jest osób na łodzi

        while(!stop_requested) {
            printf("[STERNIK%d]: Rozpoczynam wyładunek.\n", nr+1);

            while (pamiec[1 + nr] > 0) {
                semafor.sem_op(UNLOADING_SEM+nr, 1);
                sleep(1);

                if(semafor.sem_get_value(UNLOADING_SEM+nr) > 2) {
                    semafor.sem_set_value(UNLOADING_SEM+nr, 0);
                    break;
                }
                printf("[STERNIK%d]: Wyładunek: %d pasażerów na pokładzie.\n", nr+1, pamiec[1 + nr]);
            }

            semafor.sem_set_value(DIRECTION_SEM+nr, 1); // Pomost do załadunku

            printf("[STERNIK%d]: Rozpoczynam załadunek VIPów.\n", nr+1);
            while (pamiec[1 + nr] < pojemnosc) {
                semafor.sem_op(VIP_QUEUE_SEM+nr, 1);
                sleep(1);
                if(semafor.sem_get_value(VIP_QUEUE_SEM+nr) > 2){
                    semafor.sem_set_value(VIP_QUEUE_SEM+nr, 0);
                    break;
                }
            }
            printf("[STERNIK%d]: Załadunek VIP: %d pasażerów na pokładzie.\n", nr+1, pamiec[1 + nr]);

        while (pamiec[1 + nr] < pojemnosc) {
                semafor.sem_op(LOADING_SEM+nr, 1);
                sleep(1);
                if(K - semafor.sem_get_value(POMOST_SEM+nr) + pamiec[1+nr] >= pojemnosc) {
                    semafor.sem_set_value(DIRECTION_SEM+nr, 2); // nie wpuszczamy więcej na pomost, pomost do wyładunku
                }

                if(semafor.sem_get_value(LOADING_SEM+nr) > 2) {
                    semafor.sem_set_value(LOADING_SEM+nr, 0);
                    break;
                } else printf("[STERNIK%d]: Załadunek: %d pasażerów na pokładzie.\n", nr+1, pamiec[1 + nr]);
            }

        if(pamiec[1+nr] == 0) {
            printf("[STERNIK%d]: Brak pasażerów, kończę działanie.\n", nr+1);
            exit(0);
        }
        printf("[STERNIK%d]: Wyruszam w rejs. Liczba pasażerów: %d\n", nr+1, pamiec[1+nr]);
        sleep(3);

            // Rejs
            sleep(T1);
        }
        printf("[STERNIK%d]: Następny rejs się nie odbędzie z powodu SIGUSR1\n", nr+1);
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