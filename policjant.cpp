#include <iostream>
#include "classes.h"

using namespace std;

int main() {

    SharedMem memory(1234, 1024);
    memory.shm_attach();
    int* pamiec = memory.shm_get();

    printf("Policjant działa\n");

    while(true) {
        cout << "Czas: " << pamiec[0]/60 << ":" << pamiec[0] % 60 << endl;
        pamiec[0]++;
        usleep(200);
        if(pamiec[0]/60 == 1) {
            break;
        }
    }

    memory.shm_detach(pamiec);
}