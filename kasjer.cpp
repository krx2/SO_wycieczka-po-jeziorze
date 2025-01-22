#include <iostream>
#include "classes.h"

using namespace std;

# define T1N 15.0
# define T1U 7.50
# define T2N 10.0
# define T2U 5.0

int oblicz_kwote(int bilet, int dziecko, bool vip) {
    int kwota;

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

    return kwota;
}

int main() {

    SharedMem memory(1234, 1024);
    memory.shm_attach();
    int* pamiec = memory.shm_get();

    MsgQueue kolejka_komunikatow(1234);
    kolejka_komunikatow.msg_attach();

    printf("Kasjer działa\n");

    int obsluzeni[10000] = {0};
    int klient;
    int bilet;
    bool vip;
    int dziecko;
    double kwota;

    double utarg = 0;

    while (true) {
        kolejka_komunikatow.msg_send(1); // "Następny klient!"
        kolejka_komunikatow.msg_rcv(2); // Czeka na ID klienta
        klient = pamiec[1];
        if(obsluzeni[klient] > 0) {
            pamiec[1] = 1; // Mówi klientowi, że ma zniżkę
            printf("Kasjer: Gratulacje, klient %d jest VIP-em\n", klient);
        } else {
            pamiec[1] = 0;
        }
        kolejka_komunikatow.msg_send(3);
        kolejka_komunikatow.msg_rcv(4); // Pyta klienta jaki bilet
        bilet = pamiec[1];
        kolejka_komunikatow.msg_send(5); // Pyta klienta czy ma dzieci
        kolejka_komunikatow.msg_rcv(6); // Czeka na odpowiedź klienta
        dziecko = pamiec[1];
        pamiec[1] = oblicz_kwote(bilet, dziecko, vip) * 100; // w liczbie groszy bo int
        kolejka_komunikatow.msg_send(7); // Mówi klientowi ile ma zapłacić
        kolejka_komunikatow.msg_rcv(8); // Czeka na płatność
        if(pamiec[1] != 0) obsluzeni[klient] ++; // Wpisuje klienta do obsłużonych
        utarg += pamiec[1] / 100.0; // dodaje utarg
        continue; // "Nastepny klient!"

    }
    
    memory.shm_detach(pamiec);

}