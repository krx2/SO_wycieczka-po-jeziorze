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

    setpgid(0, 0); // Ustawia PID procesu jako PGID (Group ID)


    SharedMem pamiec_dzielona(1234, 1024);
    pamiec_dzielona.shm_attach();
    int* pamiec = pamiec_dzielona.shm_get();

    MsgQueue kolejka_komunikatow(1234);
    kolejka_komunikatow.msg_attach();

    kolejka_komunikatow.msg_rcv(33);
    pamiec[0] = getpid();
    kolejka_komunikatow.msg_send(34); // daje swój pid policjantowi

    pid_t pid_sternik1 = fork();
    if (pid_sternik1 < 0) {
        error("fork sternik1");
    } else if (pid_sternik1 == 0) {
        // Proces potomny: sternik1
        SharedMem pamiec_dzielona(1234, 1024);
        pamiec_dzielona.shm_attach();
        int* pamiec = pamiec_dzielona.shm_get();

        MsgQueue kolejka_komunikatow(1234);
        kolejka_komunikatow.msg_attach();

        Sem semafor(1234);
        semafor.sem_attach();

        printf("Sternik1 działa!\n");

        kolejka_komunikatow.msg_rcv(35);
        pamiec[0] = getpid();
        kolejka_komunikatow.msg_send(36); // daje swój pid policjantowi

        kolejka_komunikatow.msg_rcv(21); // Tp

        pamiec[1] = 0;
        pamiec[3] = 0;

        for (int i = 0; i < 3; i++) {
            printf("Proces sternik %d obsługuje łódź 1\n", getpid());

            // Najpierw wyładunek
            printf("Kapitan: Rozpoczynam wyładunek!\n");

            while (pamiec[1] > 0) { // pamiec[1] - liczba osób na łodzi 1
                kolejka_komunikatow.msg_send(12); // sygnał na opuszczenie pokładu
                kolejka_komunikatow.msg_rcv(13); // czeka aż pasażer zejdzie
                printf("wyładunek: Łódź 1: %d/%d\tPomost 1: %d/%d\n", pamiec[1], N1, pamiec[3], K);
                //sleep(1);
            }

            // Załadunek
            printf("Rozpoczynam załadunek!\n");
            while (pamiec[5] > 0 && pamiec[1] < N1) { // najpierw załadunek vipów, pamiec[5] - liczba czekających vipów
                kolejka_komunikatow.msg_send(9);
                printf("załadunek: Łódź 1: %d/%d\tPomost 1: %d/%d\n", pamiec[1], N1, pamiec[3], K);
            }

            while (pamiec[1] < N1) {
                kolejka_komunikatow.msg_send(10); // załadunek pasażerów, proszę wejść na pomost
                printf("wysłano 10, czekam na 11\n");
                kolejka_komunikatow.msg_rcv(11); // czeka aż pasażer wejdzie
                printf("załadunek: Łódź 1: %d/%d\tPomost 1: %d/%d\n", pamiec[1], N1, pamiec[3], K);
                //sleep(1);
            }

            // Rejs
            sleep(T1);
        }
        printf("[STERNIK1] Następny rejs się nie odbędzie z powodu SIGUSR1\n");
        pamiec_dzielona.shm_detach(pamiec);
        exit(0); // Upewnij się, że proces sternika 1 kończy się poprawnie
    }

    pid_t pid_sternik2 = fork();
    if (pid_sternik2 < 0) {
        error("fork sternik2");
    } else if (pid_sternik2 == 0) {
        
        // Proces potomny: sternik2
        SharedMem pamiec_dzielona(1234, 1024);
        pamiec_dzielona.shm_attach();
        int* pamiec = pamiec_dzielona.shm_get();

        MsgQueue kolejka_komunikatow(1234);
        kolejka_komunikatow.msg_attach();

        Sem semafor(1234);
        semafor.sem_attach();

        printf("Sternik2 działa!\n");

        kolejka_komunikatow.msg_rcv(37);
        pamiec[0] = getpid();
        kolejka_komunikatow.msg_send(38); // daje swój pid policjantowi

        kolejka_komunikatow.msg_rcv(22); // Tp

        pamiec[2] = 0;
        pamiec[4] = 0;
        /*
        while (semafor.sem_get_value(SIGUSR2) != SIGUSR2) {
            printf("Proces sternik %d obsługuje łódź 2\n", getpid());

            // Najpierw wyładunek
            printf("Kapitan: Rozpoczynam wyładunek dla łodzi 2!\n");

            while (pamiec[2] > 0) { // pamiec[2] - liczba osób na łodzi 2
                kolejka_komunikatow.msg_send(14); // sygnał na opuszczenie pokładu
                kolejka_komunikatow.msg_rcv(15); // czeka aż pasażer zejdzie
                printf("wyładunek: Łódź 2: %d/%d\tPomost 2: %d/%d\n", pamiec[2], N2, pamiec[4], K);
                sleep(1);
            }

            // Załadunek
            printf("Rozpoczynam załadunek dla łodzi 2!\n");
            while (pamiec[6] > 0 && pamiec[2] < N2) { // najpierw załadunek vipów, pamiec[6] - liczba czekających vipów
                kolejka_komunikatow.msg_send(16);
            }

            while (pamiec[2] < N2) {
                kolejka_komunikatow.msg_send(17); // załadunek pasażerów, proszę wejść na pomost
                kolejka_komunikatow.msg_rcv(18); // czeka aż pasażer wejdzie
                printf("załadunek: Łódź 2: %d/%d\tPomost 2: %d/%d\n", pamiec[2], N2, pamiec[4], K);
                sleep(1);
            }

            // Rejs
            sleep(T2);
        }
        printf("[STERNIK2] Następny rejs się nie odbędzie z powodu SIGUSR2\n");
        pamiec_dzielona.shm_detach(pamiec);
        */
        exit(0); // Upewnij się, że proces sternika 2 kończy się poprawnie
    }

    // Proces macierzysty czeka na zakończenie potomków lub dalsze sygnały
    pause();
    return 0;
}