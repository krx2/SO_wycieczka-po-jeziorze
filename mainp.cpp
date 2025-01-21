#include <iostream>
#include "classes.h"
#include <wait.h>

using namespace std;

int main() {

    pid_t mainp = getpid();
    key_t key;
    key = 1234;
    
    SharedMem memory(1234, 1024);
    memory.shm_create();
    int* shared_mem = memory.shm_get();

    shared_mem[0] = 0;
   
    pid_t pid_producent = fork();
    if (pid_producent < 0) {
        error("fork policjant");
    } else if (pid_producent == 0) {
        printf("Inicjowanie programu policjant\n");
        execl("./policjant", "./policjant", NULL);
        error("execl policjant");
    }



    wait(NULL);

    memory.shm_detach(shared_mem);
    memory.shm_delete();

    return 0;
}
