#include <iostream>
#include "classes.h"

using namespace std;

int oblicz_kwote(int bilet, int dziecko, bool vip) {
    int kwota = 0;

    if(dziecko > 0) {
        if(bilet == 1) {
            kwota += T1N + T1U;
        } else {
            kwota += T2N + T2U;
        }
    } else {
        if(bilet == 1) kwota += T1N; else kwota += T2N;
    }
    if(vip) kwota /=2;

    return kwota * 100;
}

volatile sig_atomic_t stop_requested = 0; // Flaga do zatrzymania pętli

int* pamiec = nullptr;
int utarg = 0;

void signal_handler(int sig) {
    printf("\033[33m[KASJER]\033[0m: Zakończono pracę. Utarg: %d\n", utarg/100);
    if (pamiec != nullptr) {
        if (shmdt(pamiec) == -1) {
            perror("shmdt error");
        }
    }
    exit(EXIT_SUCCESS);
}


int main() {

    SharedMem memory(1234, 1024);
    memory.shm_attach();
    pamiec = memory.shm_get();

    MsgQueue kolejka_komunikatow(1234);
    kolejka_komunikatow.msg_attach();

    Sem semafor(1234);
    semafor.sem_attach();

    pamiec[5] = getpid();
    kolejka_komunikatow.msg_send(25);

    semafor.sem_op(0, -1); // start symulacji po zainicjowaniu procesów

    printf("\033[33m[KASJER]\033[0m: działa\n");

    int obsluzeni[10000] = {0};
    int klient;
    int bilet;
    bool vip;
    int dziecko;
    int kwota;

    signal(SIGINT, signal_handler);

    while (!stop_requested) {
        kolejka_komunikatow.msg_send(1); // "Następny klient!"
        kolejka_komunikatow.msg_rcv(2); // Czeka na ID klienta
        klient = pamiec[0];
        if(obsluzeni[klient] > 0) {
            pamiec[0] = 1; // Mówi klientowi, że ma zniżkę
            //printf("Kasjer: Gratulacje, klient %d jest VIP-em\n", klient);
        } else {
            pamiec[0] = 0;
        }
        kolejka_komunikatow.msg_send(3);
        kolejka_komunikatow.msg_rcv(4); // Pyta klienta jaki bilet
        bilet = pamiec[0];
        kolejka_komunikatow.msg_send(5); // Pyta klienta czy ma dzieci
        kolejka_komunikatow.msg_rcv(6); // Czeka na odpowiedź klienta
        dziecko = pamiec[0];
        kwota = oblicz_kwote(bilet, dziecko, vip);
        pamiec[0] = kwota; // w liczbie groszy bo int
        kolejka_komunikatow.msg_send(7); // Mówi klientowi ile ma zapłacić
        kolejka_komunikatow.msg_rcv(8); // Czeka na płatność
        if(pamiec[0] != 0) obsluzeni[klient] ++; // Wpisuje klienta do obsłużonych
        utarg += pamiec[0]; // dodaje utarg
        obsluzeni[klient]++;
        // "Nastepny klient!"
        if(klient == 2200) break;

    }
    
    pause();
    

}