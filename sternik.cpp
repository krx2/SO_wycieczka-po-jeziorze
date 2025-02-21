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

int* pamiec;

void signal_handler(int sig) {
    if (sig == SIGUSR1) {
        stop_requested += 1; // Ustawienie flagi końca działania
        printf("\033[34m[STERNIK1]\033[0m: Otrzymano sygnał SIGUSR1\n");
    } else if (sig == SIGUSR2) {
        stop_requested += 2; // Ustawienie flagi końca działania
        printf("\033[34m[STERNIK2]\033[0m: Otrzymano sygnał SIGUSR2\n");
    } else {
        if (pamiec != nullptr) {
        if (shmdt(pamiec) == -1) {
            perror("shmdt error sternik");
        }
    }
    exit(EXIT_SUCCESS);
    }
    stop_requested = 3; // Ustawienie flagi końca działania
    printf("\033[34m[STERNIK]\033[0m: Otrzymano sygnał SIGINT\n");
}

void sternik(int nr) {
    // Proces potomny: sternik

        signal(SIGUSR1, signal_handler);
        signal(SIGUSR2, signal_handler);

        SharedMem pamiec_dzielona(1234, 1024);
        pamiec_dzielona.shm_attach();
        pamiec = pamiec_dzielona.shm_get();

        MsgQueue kolejka_komunikatow(1234);
        kolejka_komunikatow.msg_attach();

        Sem semafor(1234);
        semafor.sem_attach();

        pamiec[3+nr] = getpid();
        printf("pid sternik%d: %d", 1+nr, pamiec[3+nr]);

        kolejka_komunikatow.msg_send(23+nr);

        int pojemnosc;
        int czas;

        if(nr == 0) {
            pojemnosc = N1;
            czas = T1;
        } else {
            pojemnosc = N2;
            czas = T2;
        }

        int pasazerowie = 0;
        

        printf("\033[34m[STERNIK%d]\033[0m: działa!\n", 1+nr);

        kolejka_komunikatow.msg_rcv(20+nr); // Tp czas początkowy

        semafor.sem_set_value(3+nr, K); // semafor dla pomostu

        pamiec[1+nr] = 0; // ile jest osób na łodzi

        while(stop_requested != 1+nr && stop_requested != 3) {

            semafor.sem_set_value(DIRECTION_SEM+nr, 1); // Pomost do załadunku

            semafor.sem_op(1+nr, -1); // blokuje pamięć
            pasazerowie = pamiec[1+nr]; // update liczby pasazerow
            semafor.sem_op(1+nr, 1); // odblokowywuje pamięć

            printf("\033[34m[STERNIK%d]\033[0m: Rozpoczynam załadunek VIPów.\n", nr+1);
            while (pasazerowie < pojemnosc) {
                //usleep(100);

                semafor.sem_op(VIP_QUEUE_SEM+nr, 1);

                if(semafor.sem_get_value(VIP_QUEUE_SEM+nr) > 2){
                    semafor.sem_set_value(VIP_QUEUE_SEM+nr, 0);
                    break;
                }

            semafor.sem_op(1+nr, -1); // blokuje pamięć
            pasazerowie = pamiec[1+nr]; // update liczby pasazerow
            semafor.sem_op(1+nr, 1); // odblokowywuje pamięć

            }

            semafor.sem_op(1+nr, -1); // blokuje pamięć
            pasazerowie = pamiec[1+nr]; // update liczby pasazerow
            semafor.sem_op(1+nr, 1); // odblokowywuje pamięć

            printf("\033[34m[STERNIK%d]\033[0m: Zakończono załadunek VIP: %d pasażerów na pokładzie.\n", nr+1, pasazerowie);

            printf("\033[34m[STERNIK%d]\033[0m: Rozpoczynam załadunek pasażerów\n", nr+1);
            while (pasazerowie < pojemnosc) {
                //usleep(100);
                
                semafor.sem_op(LOADING_SEM+nr, 1);

                if(semafor.sem_get_value(LOADING_SEM+nr) > 2) {
                    semafor.sem_set_value(LOADING_SEM+nr, 0);
                    semafor.sem_set_value(DIRECTION_SEM+nr, 2);
                    break;
                } //else printf("\033[34m[STERNIK%d]\033[0m: Załadunek: %d pasażerów na pokładzie.\n", nr+1, pasazerowie);

                semafor.sem_op(1+nr, -1); // blokuje pamięć
                pasazerowie = pamiec[1+nr]; // update liczby pasazerow
                semafor.sem_op(1+nr, 1); // odblokowywuje pamięć
            }

            while (semafor.sem_get_value(POMOST_SEM+nr)!=10) { // czekaj aż wszyscy zejdą z pomostu
                semafor.sem_op(LOADING_SEM+nr, 1);
                if(semafor.sem_get_value(LOADING_SEM+nr) > 1000) break;
            }

            sleep(1);

            semafor.sem_op(1+nr, -1); // blokuje pamięć
            pasazerowie = pamiec[1+nr]; // update liczby pasazerow
            semafor.sem_op(1+nr, 1); // odblokowywuje pamięć

            printf("\033[34m[STERNIK%d]\033[0m: Zakończono załadunek: %d pasażerów na pokładzie. Liczba pasażerów na pomoście: %d \n", nr+1, pasazerowie, 10 - semafor.sem_get_value(POMOST_SEM+nr));

            semafor.sem_set_value(LOADING_SEM+nr, 0);


            semafor.sem_op(1+nr, -1); // blokuje pamięć
            pasazerowie = pamiec[1+nr]; // update liczby pasazerow
            semafor.sem_op(1+nr, 1); // odblokowywuje pamięć

            if(pamiec[8] == 1) {
                break;
            }

            printf("\033[34m[STERNIK%d]\033[0m: Wyruszam w rejs. Liczba pasażerów: %d\n", nr+1, pasazerowie);

            // Rejs
            sleep(czas);

            printf("\033[34m[STERNIK%d]\033[0m: Rozpoczynam wyładunek.\n", nr+1);

            semafor.sem_set_value(DIRECTION_SEM+nr, 2);

            semafor.sem_set_value(UNLOADING_SEM+nr, pasazerowie);
            
            while (semafor.sem_get_value(POMOST_SEM+nr)!=10) { // czekaj aż wszyscy zejdą z pomostu
                continue;
            }

            sleep(1);

            semafor.sem_op(1+nr, -1); // blokuje pamięć
            pasazerowie = pamiec[1+nr]; // update liczby pasazerow
            semafor.sem_op(1+nr, 1); // odblokowywuje pamięć

            printf("\033[34m[STERNIK%d]\033[0m: Zakończono wyładunek: %d pasażerów na pokładzie. Liczba pasażerów na pomoście: %d \n", nr+1, pasazerowie, 10 - semafor.sem_get_value(POMOST_SEM+nr));
            
            printf("pamiec[8]: %d\n", pamiec[8]);
            if(pamiec[8] == 1) {
                break;
            }
        }
        printf("\033[34m[STERNIK%d]\033[0m: Następny rejs się nie odbędzie\n", nr+1);
        semafor.sem_set_value(UNLOADING_SEM+nr, pasazerowie);
        pamiec_dzielona.shm_detach(pamiec);
        exit(0);
}

int main() {

    signal(SIGUSR1, signal_handler);
    signal(SIGUSR2, signal_handler);
    
    pid_t pid_sternik = getpid();

    MsgQueue kolejka_komunikatow(1234);
    kolejka_komunikatow.msg_attach();

    Sem semafor(1234);
    semafor.sem_attach();

    semafor.sem_op(0, -1); // start symulacji po zainicjowaniu procesów

    pid_sternik1 = fork();
    if (pid_sternik1 < 0) {
        error("fork sternik1");
    } else if (pid_sternik1 == 0) {
        sternik(0);
    }

    pid_sternik2 = fork();
    if (pid_sternik2 < 0) {
        error("fork sternik2");
    } else if (pid_sternik2 == 0) {
        sternik(1);
    }

    // Proces macierzysty czeka na zakończenie potomków lub dalsze sygnały
    pause();
}