#include <iostream>
#include "classes.h"

using namespace std;

int main() {

    key_t key;
    key = ftok("mainp", 1);

    SharedMemory SharedMemory(key, 1024, IPC_CREAT | 0666);
    void* shmID = SharedMemory.attach();

    MessageQueue(key, IPC_CREAT | 0666);

    int* pamiec = static_cast<int*>(shmID);
    pamiec[0] = 0;

    try {
        pid_t czas = NowyProces("./czas");
    }
    catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
    }

    try {
        pid_t czas = NowyProces("./sternik");
    }
    catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
    }

    try {
        pid_t czas = NowyProces("./kasjer");
    }
    catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
    }

    try {
        pid_t czas = NowyProces("./pasazer");
    }
    catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
    }

    try {
        pid_t czas = NowyProces("./policjant");
    }
    catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
    }
    

    return 0;
}
