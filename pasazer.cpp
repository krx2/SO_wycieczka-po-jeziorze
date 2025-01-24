#include <iostream>
#include "classes.h"
#include <queue>

using namespace std;

class pasazer {
    public:
    int id;
    bool vip;
    int wiek;
    int wiek_dziecka;
    int miejsca;
    int status;
    int bilet;
    double portfel;
    pasazer(int p_id) {
        id = p_id;
        wiek = rand() % 65 +15;
        if(rand() % 4 == 3) {
            wiek_dziecka = rand() % 15 + 1;
            miejsca = 2;
        }
        else {
            wiek_dziecka = 0;
            miejsca = 1;
        }
        status = 1;
        bilet = 0;
        portfel = double(rand() % 100);
        printf("Pasażer o ID %d przyjechał nad jezioro\n", id);
    }

    void ustaw_do_kasy() {
        
        if(wiek_dziecka == 0) {
            printf("Pasażer o ID %d ustawił się do kasy\n", id);
        } else {
            printf("Pasażer z dzieckiem, ID %d ustawili się do kasy\n", id);
        }
    }

    bool platnosc(int kwota) {
        if(portfel - kwota >= 0) {
            portfel -= kwota;
            status = 2;
            return true;
        } else {
            status = 0;
            return false;
        }
    }

    bool zdecyduj() {
        if(rand() % 2 == 0) return true;
        return false;
    }

};

void* pasazerowie(void* arg) {
    int id = *static_cast<int*>(arg);
    pasazer klient(id);

    SharedMem memory(1234, 1024);
    memory.shm_attach();
    int* pamiec = memory.shm_get();

    MsgQueue kolejka_komunikatow(1234);
    kolejka_komunikatow.msg_attach();
    
    kolejka_komunikatow.msg_rcv(1);
    klient.ustaw_do_kasy();
    pamiec[0] = id;
    kolejka_komunikatow.msg_send(2);
    // Klient czeka na informacje o zniżkach
    kolejka_komunikatow.msg_rcv(3);
    // Klient wybiera bilet
    if(pamiec[0] == 1) { // Jak jest VIP to dowolny
        klient.vip = true;
        if(klient.zdecyduj()) {
            pamiec[0] = 1;
        } else {
            pamiec[0] = 2;
        }
    } else { // Jak nie ma VIP to mogą być ograniczenia
        if(klient.wiek > 70 || klient.wiek_dziecka > 0) { // Jak > 70 lat lub z dzieckiem to bilet 2
            pamiec[0] = 2;
        } else { // Inaczej decyduje
            if(klient.zdecyduj()) {
            pamiec[0] = 1;
            } else {
                pamiec[0] = 2;
            }
        }
    }
    klient.bilet = pamiec[0];
    // Przekazuje informacje o biliecie kasjerowi
    kolejka_komunikatow.msg_send(4);
    kolejka_komunikatow.msg_rcv(5); // Kasjer pyta o dzieci
    pamiec[0] = klient.wiek_dziecka;
    kolejka_komunikatow.msg_send(6); // Mówi kasjerowi o dziecku
    kolejka_komunikatow.msg_rcv(7); // Kasjer mówi ile ma zapłacić
    if(klient.platnosc(double(pamiec[0] / 100))) { // kwota w liczbie groszy
        kolejka_komunikatow.msg_send(8);
    } else {
        pamiec[0] = 0; // Płatność nie przeszła
        kolejka_komunikatow.msg_send(8); // Mówi kasjerowi że nie ma tyle
        klient.status = 0;
        klient.bilet = 0;
        printf("Pasażer o ID: %d opuszcza jezioro z powodu braku pieniędzy: %lf\n", klient.id, klient.portfel);
        memory.shm_detach(pamiec);
        return nullptr; // Klient opuszcza jezioro
    }
    printf("Transakcja udana: %d\n", klient.id);
    // Po transakcji klient idzie na molo i czeka na sygnał od kapitana
    if(klient.bilet == 1) { // dla łodzi 1
        if(klient.vip) {
            pamiec[5]++; // liczba vipów czekająca na wejście
            kolejka_komunikatow.msg_rcv(9); // komunikat załadunek dla vipów
        } else {
            kolejka_komunikatow.msg_rcv(10); // komunikat załadunek
        }
    } else { // dla łodzi 2
        if(klient.vip) {
            pamiec[6]++; // liczba vipów czekająca na wejście
            kolejka_komunikatow.msg_rcv(11); // komunikat załadunek dla vipów
        } else {
            kolejka_komunikatow.msg_rcv(12); // komunikat załadunek
        }
    }
    memory.shm_detach(pamiec);
    return nullptr;
}


int main() {

    
    SharedMem pamiec_dzielona(1234, 1024);
    pamiec_dzielona.shm_attach();
    int* pamiec = pamiec_dzielona.shm_get();

    MsgQueue kolejka_komunikatow(1234);
    kolejka_komunikatow.msg_attach();

    Sem semafor(1234);
    semafor.sem_attach();

    semafor.sem_op(0, 0); // czekanie na start symulacji
    
    printf("Pasażer działa\n");

    for(int i = 0; i < 100; i++) {
        pthread_t thread;
        if (pthread_create(&thread, NULL, pasazerowie, &i) != 0) {
            error("Błąd tworzenia wątku");
        }
    }


    pamiec_dzielona.shm_detach(pamiec);

}