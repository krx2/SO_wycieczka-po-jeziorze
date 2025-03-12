#include <iostream>
#include "classes.h"

using namespace std;

volatile sig_atomic_t liczba_pasazerow = 0;

pid_t pid_policjant;

void signal_handler(int sig) {

    if(sig == SIGUSR1) {
        Sem semafor(1234);
        semafor.sem_attach();

        semafor.sem_op(14, -2); // czekanie na zakończenie rejsów

        printf("\033[32m[PASAŻER]\033[0m: Pozostali pasażerowie czekający na molo: %d Opuszczają jezioro\n", liczba_pasazerow);

        kill(pid_policjant, SIGUSR1);

    } else if(sig == SIGUSR2) {
        printf("\033[32m[PASAŻER]\033[0m: Pozostali pasażerowie czekający na molo: %d Opuszczają jezioro\n", liczba_pasazerow);
    }
}

class pasazer {
    public:
    int id;
    bool vip;
    int wiek;
    int wiek_dziecka;
    int miejsca;
    int status;
    int bilet;
    int portfel;
    pasazer(int p_id) {
        id = p_id;
        wiek = rand() % 65 + 15;
        if(rand() % 4 == 0) {
            wiek_dziecka = rand() % 15 + 1;
            miejsca = 2;
        }
        else {
            wiek_dziecka = 0;
            miejsca = 1;
        }
        status = 1;
        bilet = -1;
        portfel = rand() % 150 + 1;
        portfel *= 100;
        //printf("Pasażer o ID %d przyjechał nad jezioro\n", id);
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

int molo_pomost_statek(Sem& semafor, int* pamiec, pasazer& klient);
int bilety(MsgQueue& kolejka_komunikatow, int* pamiec, pasazer& klient);

void* pasazerowie(void* arg) {
    int id = *static_cast<int*>(arg);
    pasazer klient(id);

    SharedMem memory(1234, 12);
    memory.shm_attach();
    int* pamiec = memory.shm_get();

    MsgQueue kolejka_komunikatow(1234);
    kolejka_komunikatow.msg_attach();

    Sem semafor(1234);
    semafor.sem_attach();

    liczba_pasazerow++;

    if(semafor.sem_get_value(13) == 0) { // niech się stworzy następny
        semafor.sem_op(13, 1);
    }

    
    
    
    // Po transakcji klient idzie na molo i czeka na sygnał od kapitana

    while (pamiec[8] != 1) {
        if(bilety(kolejka_komunikatow, pamiec, klient) == -1) {
            //printf("\033[32m[PASAŻER]\033[0m: Transakcja nieudana. Klient opuszcza jezioro. ID: %d\n", klient.id);
            break;
        }
        //printf("\033[32m[PASAŻER]\033[0m: %d ustawia się w kolejce na łódź %d\n", klient.id, klient.bilet+1);
        bool nieuzyty_bilet = true;
        while (nieuzyty_bilet) {
            if(molo_pomost_statek(semafor, pamiec, klient) == 0) {
                nieuzyty_bilet = false;
            }
        }
    }
    
    
    liczba_pasazerow--;
    memory.shm_detach(pamiec);
    return nullptr;
}

int bilety(MsgQueue& kolejka_komunikatow, int* pamiec, pasazer& klient) {
    kolejka_komunikatow.msg_rcv(1);
    //klient.ustaw_do_kasy();
    pamiec[0] = klient.id;
    kolejka_komunikatow.msg_send(2);
    // Klient czeka na informacje o zniżkach
    kolejka_komunikatow.msg_rcv(3);
    // Klient wybiera bilet
    if(pamiec[0] == 1) { // Jak jest VIP to dowolny
        klient.vip = true;
        if(klient.zdecyduj()==false) {
            klient.bilet = 0;
        } else {
            klient.bilet = 1;
        }
    } else { // Jak nie ma VIP to mogą być ograniczenia
        if(klient.wiek > 70 || klient.wiek_dziecka > 0) { // Jak > 70 lat lub z dzieckiem to bilet 2
            klient.bilet = 1;
        } else { // Inaczej decyduje
            if(klient.zdecyduj()==false) {
            klient.bilet = 0;
            } else {
                klient.bilet = 1;
            }
        }
    }
    pamiec[0] = klient.bilet;
    // Przekazuje informacje o biliecie kasjerowi
    kolejka_komunikatow.msg_send(4);
    kolejka_komunikatow.msg_rcv(5); // Kasjer pyta o dzieci
    pamiec[0] = klient.wiek_dziecka;
    kolejka_komunikatow.msg_send(6); // Mówi kasjerowi o dziecku
    kolejka_komunikatow.msg_rcv(7); // Kasjer mówi ile ma zapłacić
    if(klient.platnosc(pamiec[0])) { // kwota w liczbie groszy
        kolejka_komunikatow.msg_send(8);
    } else {
        pamiec[0] = 0; // Płatność nie przeszła
        kolejka_komunikatow.msg_send(8); // Mówi kasjerowi że nie ma tyle
        klient.status = 0;
        klient.bilet = -1;
        //printf("Pasażer o ID: %d opuszcza jezioro z powodu braku pieniędzy: %d\n", klient.id, klient.portfel);
        return -1; // Klient opuszcza jezioro
    }
    //printf("\033[32m[PASAŻER]\033[0m: Transakcja udana. ID: %d Bilet: %d\n", klient.id, klient.bilet);
    return 0;
}

int molo_pomost_statek(Sem& semafor, int* pamiec, pasazer& klient) {
    int nr = klient.bilet;
    int pojemnosc_lodzi;
    if(nr == 0) {
        pojemnosc_lodzi = N1;
    } else pojemnosc_lodzi = N2;
    
    if(klient.vip) {
        semafor.sem_op(VIP_QUEUE_SEM+nr, -1); // czeka na podniesienie semafora

        semafor.sem_op(1+nr, -1); // blokuje pamięć
        if(pamiec[1+nr] + klient.miejsca <= pojemnosc_lodzi) {
            pamiec[1+nr] += klient.miejsca; // wchodzi na łódź
            printf("\033[32m[PASAŻER]\033[0m: VIP %d Wszedł na łódź %d\n", klient.id, nr+1);
        } else {
            //printf("\033[32m[PASAŻER]\033[0m: VIP nie może wejść na łódź %d\n", 1+nr);
            semafor.sem_op(1+nr, 1); // odblokowywuje pamięć
            return 1;
        }
        semafor.sem_op(1+nr, 1); // odblokowywuje pamięć
    } else { // jeśli pomost do wejścia to może wejść

        printf("\033[32m[PASAŻER]\033[0m: %d Czeka w kolejce na załadunek %d\n", klient.id, nr+1);
        
        semafor.sem_op(POMOST_ZAL_SEM+nr, -1*klient.miejsca); // czeka w kolejce na wejście

        if(pamiec[10+nr] != 1) {
            printf("\033[32m[PASAŻER]\033[0m: %d Próbuje wejść na zamnknięty pomost %d\n", klient.id, nr+1);
            return 1;
        }

        printf("\033[32m[PASAŻER]\033[0m: %d Czeka na wejście na pomost %d\n", klient.id, nr+1);

        semafor.sem_op(POMOST_SEM+nr, -1*klient.miejsca); // wchodzi na pomost

        semafor.sem_op(1+nr, -1); // blokuje pamięć
        if(pamiec[1+nr] + klient.miejsca <= pojemnosc_lodzi) {
            pamiec[1+nr] += klient.miejsca; // wchodzi na łódź

            printf("\033[32m[PASAŻER]\033[0m: %d Wszedł na łódź %d\n", klient.id, nr+1);
            semafor.sem_op(1+nr, 1); // odblokowywuje pamięć

        } else {
            printf("\033[32m[PASAŻER]\033[0m: %d nie może wejść na łódź %d\n", klient.id, nr+1);
            semafor.sem_op(1+nr, 1); // odblokowywuje pamięć
            semafor.sem_op(POMOST_SEM+nr, klient.miejsca); // zwalnia miejsce na pomoście
            return 1;
        }
        
        semafor.sem_op(POMOST_SEM+nr, klient.miejsca); // zwalnia miejsce na pomoście

    }

    semafor.sem_op(POMOST_WYL_SEM+nr, -1*klient.miejsca); // schodzi z łodzi po rejsie

    semafor.sem_op(POMOST_SEM+nr, -1*klient.miejsca); // wchodzi na pomost

    semafor.sem_op(1+nr, -1); // blokuje pamięć
    pamiec[1+nr] -= klient.miejsca; // schodzi z łodzi
    semafor.sem_op(POMOST_SEM+nr, klient.miejsca); // zwalnia miejsce na pomoście
    printf("\033[32m[PASAŻER]\033[0m: %d zszedł ze statku %d\n", klient.id, nr+1);
    semafor.sem_op(1+nr, 1); // odblokowywuje pamięć

    

    return 0;
}

int main() {

    signal(SIGUSR1, signal_handler);
    signal(SIGUSR2, signal_handler);

    Sem semafor(1234);
    semafor.sem_attach();

    SharedMem memory(1234, 12);
    memory.shm_attach();
    int* pamiec = memory.shm_get();

    semafor.sem_op(0, -1); // czekanie na start symulacji
    
    printf("\033[32m[PASAŻER]\033[0m: działa, PID: %d\n", getpid());

    for(int i = 1; i < MAX_PASSENGERS; i++) {
        pthread_t thread;
        if (pthread_create(&thread, NULL, pasazerowie, &i) != 0) {
            error("\033[32m[PASAŻER]\033[0m: Błąd tworzenia wątku");
        }
        if(i != MAX_PASSENGERS - 1) semafor.sem_op(13, -1);
    }

    while(pamiec[8] != 1) {
        continue;
    }

    pid_policjant = pamiec[6];

    memory.shm_detach(pamiec);

    raise(SIGUSR1);

    pause();

}