#include <iostream>
#include "classes.h"

using namespace std;

volatile sig_atomic_t stop_requested = 0; // Flaga do zatrzymania pętli

pid_t pid_sternik1;
pid_t pid_sternik2;

int* pamiec = nullptr;

void signal_handler(int sig) {
    if (sig == SIGUSR1) {
        if(stop_requested > 0) {
            stop_requested = 3; // Ustawienie flagi końca działania
        } else stop_requested = 1;
        printf("\033[34m[STERNIK1]\033[0m: Otrzymano sygnał SIGUSR1\n");
    } else if (sig == SIGUSR2) {
        if(stop_requested > 0) {
            stop_requested = 3; // Ustawienie flagi końca działania
        } else stop_requested = 2;
        printf("\033[34m[STERNIK2]\033[0m: Otrzymano sygnał SIGUSR2\n");
    } else {
        if (pamiec != nullptr) {
            if (shmdt(pamiec) == -1) {
                perror("shmdt error sternik");
            }
        }
        pamiec = nullptr;
        exit(EXIT_SUCCESS);
    }
}

void signal_handler_parent(int sig) {
    if (sig == SIGINT) {
        if (pid_sternik1 > 0) {
            kill(pid_sternik1, SIGINT);
            waitpid(pid_sternik1, NULL, 0);
        }

        if (pid_sternik2 > 0) {
            kill(pid_sternik2, SIGINT);
            waitpid(pid_sternik2, NULL, 0);
        }

        exit(EXIT_SUCCESS);
    }
}

void sternik(int nr) {
    // Proces potomny: sternik

        signal(SIGUSR1, signal_handler);
        signal(SIGUSR2, signal_handler);
        signal(SIGINT, signal_handler);

        SharedMem pamiec_dzielona(1234, 12);
        pamiec_dzielona.shm_attach();
        pamiec = pamiec_dzielona.shm_get();

        MsgQueue kolejka_komunikatow(1234);
        kolejka_komunikatow.msg_attach();

        Sem semafor(1234);
        semafor.sem_attach();

        pamiec[3+nr] = getpid();

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

        pamiec[10+nr] = 1; // kierunek pomostu 1 - do wchodzenia, 2 - do wychodzenia
        
        semafor.sem_set_value(POMOST_SEM+nr, K); // semafor dla pomostu
        semafor.sem_set_value(POMOST_ZAL_SEM+nr, 0); // semafor dla załadunku

        pamiec[1+nr] = 0; // ile jest osób na łodzi

        printf("\033[34m[STERNIK%d]\033[0m: działa, PID: %d\n", 1+nr, getpid());

        kolejka_komunikatow.msg_rcv(20+nr); // Tp czas początkowy



        while(pamiec[8] != 1 && stop_requested != 1+nr && stop_requested != 3) {

            if(pamiec[8] == 1 || stop_requested == 1+nr || stop_requested == 3) break;

            semafor.sem_op(1+nr, -1); // blokuje pamięć
            pasazerowie = pamiec[1+nr]; // update liczby pasazerow
            semafor.sem_op(1+nr, 1); // odblokowywuje pamięć

            printf("\033[34m[STERNIK%d]\033[0m: Rozpoczynam załadunek VIPów.\n", nr+1);
            while (pasazerowie < pojemnosc) {

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

            semafor.sem_op(1+nr, -1); // blokuje pamięć
            pasazerowie = pamiec[1+nr]; // update liczby pasazerow
            semafor.sem_op(1+nr, 1); // odblokowywuje pamięć

            if(pasazerowie < pojemnosc) {
                printf("\033[34m[STERNIK%d]\033[0m: Rozpoczynam załadunek pasażerów.\n", nr+1);

                semafor.sem_op(1+nr, -1); // blokuje pamięć
                pamiec[10+nr] = 1;
                pasazerowie = pamiec[1+nr]; // update liczby pasazerow
                semafor.sem_op(1+nr, 1); // odblokowywuje pamięć

                

                if(pojemnosc % 2 == 1) {
                    semafor.sem_set_value(POMOST_ZAL_SEM+nr, pojemnosc - pasazerowie + 1); // daje możliwość wejścia na pokład
                } else {
                    semafor.sem_set_value(POMOST_ZAL_SEM+nr, pojemnosc - pasazerowie); // daje możliwość wejścia na pokład
                }

                while(pasazerowie != pojemnosc) { // czekaj aż wszyscy wsiądą

                    if(pamiec[8] == 1 || stop_requested == 1+nr || stop_requested == 3) {
                        semafor.sem_set_value(POMOST_ZAL_SEM+nr, 0);
                        break;
                    }

                    if(pojemnosc % 2 == 1 && pasazerowie == pojemnosc - 1 && semafor.sem_get_value(POMOST_ZAL_SEM+nr) == 0) {
                        break;
                    }
                    
                    semafor.sem_op(1+nr, -1); // blokuje pamięć
                    pasazerowie = pamiec[1+nr]; // update liczby pasazerow
                    semafor.sem_op(1+nr, 1); // odblokowywuje pamięć

                }

                semafor.sem_set_value(POMOST_ZAL_SEM+nr, 0);

                printf("\033[34m[STERNIK%d]\033[0m: Czekam na opróżnienie pomostu...\n", nr+1);
                while(semafor.sem_get_value(POMOST_SEM+nr) != K) { // oczekiwanie na opróżnienie pomostu
                    continue;
                }

                semafor.sem_op(1+nr, -1); // blokuje pamięć
                pasazerowie = pamiec[1+nr]; // update liczby pasazerow
                semafor.sem_op(1+nr, 1); // odblokowywuje pamięć

                printf("\033[34m[STERNIK%d]\033[0m: Zakończono załadunek: %d pasażerów na pokładzie. Liczba pasażerów na pomoście: %d \n", nr+1, pasazerowie, K - semafor.sem_get_value(POMOST_SEM+nr));

            }

           

            semafor.sem_op(1+nr, -1); // blokuje pamięć
            pasazerowie = pamiec[1+nr]; // update liczby pasazerow
            semafor.sem_op(1+nr, 1); // odblokowywuje pamięć

            if(pamiec[8] == 1 || stop_requested == 1+nr || stop_requested == 3) break;

            semafor.sem_op(1+nr, -1); // blokuje pamięć
            pamiec[10+nr] = 2;
            semafor.sem_op(1+nr, 1); // odblokowywuje pamięć

            printf("\033[34m[STERNIK%d]\033[0m: Wyruszam w rejs. Liczba pasażerów: %d\n", nr+1, pasazerowie);

            // Rejs
            //sleep(czas);

            printf("\033[34m[STERNIK%d]\033[0m: Rozpoczynam wyładunek.\n", nr+1);

            

            semafor.sem_op(1+nr, -1); // blokuje pamięć
            pasazerowie = pamiec[1+nr]; // update liczby pasazerow
            semafor.sem_op(1+nr, 1); // odblokowywuje pamięć

            semafor.sem_set_value(POMOST_WYL_SEM+nr, pasazerowie); // umożliwia pasażerom zejście z pokładu

            while(pasazerowie != 0 || semafor.sem_get_value(POMOST_SEM+nr) != K) {
                semafor.sem_op(1+nr, -1); // blokuje pamięć
                pasazerowie = pamiec[1+nr]; // update liczby pasazerow
                semafor.sem_op(1+nr, 1); // odblokowywuje pamięć
            }

            semafor.sem_set_value(POMOST_WYL_SEM+nr, 0);
            
            printf("\033[34m[STERNIK%d]\033[0m: Zakończono wyładunek: %d pasażerów na pokładzie. Liczba pasażerów na pomoście: %d \n", nr+1, pasazerowie, K - semafor.sem_get_value(POMOST_SEM+nr));
        }
        printf("\033[34m[STERNIK%d]\033[0m: Następny rejs się nie odbędzie\n", nr+1);

        semafor.sem_set_value(VIP_QUEUE_SEM+nr, 0);
        semafor.sem_set_value(LOADING_SEM+nr, 0);

        semafor.sem_op(1+nr, -1); // blokuje pamięć
        pasazerowie = pamiec[1+nr]; // update liczby pasazerow
        semafor.sem_op(1+nr, 1); // odblokowywuje pamięć

        semafor.sem_set_value(POMOST_WYL_SEM+nr, pasazerowie);
        //usleep(100);
        semafor.sem_op(14, 1);

        pamiec_dzielona.shm_detach(pamiec);
        exit(0);
}

int main() {

    signal(SIGINT, signal_handler_parent);
    
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