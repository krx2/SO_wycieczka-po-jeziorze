#include <iostream>
#include "classes.h"

using namespace std;

int main() {
    key_t key;
    key = ftok("mainp", 1);

    SharedMemory pamiec_dzielona(key, 1024, 0666);
    MessageQueue kolejka_komunikatow(key, 0666);

    void* shmID = pamiec_dzielona.attach();
    int* pamiec = static_cast<int*>(shmID);

}