#include <iostream>
#include "classes.h"
#include <queue>

using namespace std;

queue<int> kolejka_do_kasy;
queue<int> kolejka_molo;
queue<int> kolejka_molo_vip;

class pasazer {
    int id;
    bool vip;
    int wiek;
    int wiek_dziecka;
    int obj;
    int pozycja;
    int bilet;
    double portfel;
    public: 
    pasazer(int p_id) {
        id = p_id;
        vip = false;
        wiek = rand() % 65 +15;
        if(rand() % 4 == 3) {
            wiek_dziecka = rand() % 15 + 1;
            obj = 2;
        }
        else {
            wiek_dziecka = 0;
            obj = 1;
        }
        pozycja = 0;
        bilet = 0;
        portfel = double(rand() % 50);
        cout << "Pasażer o ID " << id << " przyjecjał nad jezioro" << endl;
    }

    bool czy_dziecko() {
        if(wiek_dziecka > 0) return true;
        return false;
    }

    void ustaw_do_kasy() {
        kolejka_do_kasy.push(id);
        pozycja = 1;
        cout << "Pasażer o ID " << id << " ustawił się w kolejce" << endl;
        if(wiek_dziecka > 0) {
            kolejka_do_kasy.push(id);
            cout << "Dziecko pasażera o ID " << id << "ustawiło sie w kolejce" << endl;
        }
    }
};

void* pasazerowie(void* arg) {
    int id = *static_cast<int*>(arg);
    pasazer klient(id);
    klient.ustaw_do_kasy();

    return nullptr;
}

struct argumenty {
    int id;
    queue<int> kolejka;
};

int main() {
    key_t key;
    key = ftok("mainp", 1);

    SharedMemory pamiec_dzielona(key, 1024, 0666);
    MessageQueue kolejka_komunikatow(key, 0666);

    void* shmID = pamiec_dzielona.attach();
    int* pamiec = static_cast<int*>(shmID);

    

    for(int i = 0; i < 100; i++) {
        pthread_t thread;
        if (pthread_create(&thread, NULL, pasazerowie, &i) != 0) {
        throw runtime_error("Błąd tworzenia wątku");
        }
    }




}