#include <iostream>
#include "classes.h"

using namespace std;

int main() {
    key_t key;
    key = 1234;

    SharedMem memory(1234, 1024);
    memory.shm_attach();
    int* pamiec = memory.shm_get();

    while(true) {
        cout << "Czas: " << pamiec[0]/60 << ":" << pamiec[0] % 60 << endl;
        pamiec[0]++;
        usleep(2000);
    }

}