#include <iostream>
#include "classes.h"
#include <wait.h>

using namespace std;

int main() {

    pid_t mainp = getpid();
    
    SharedMem memory(1234, 1024);
    memory.shm_create();
    int* shared_mem = memory.shm_get();

    for(int i = 0; i < 1024; i++) {
        shared_mem[i] = 0;
    }

    MsgQueue queue(1234);
    queue.msg_create();

    Sem semafor(1234);
    semafor.sem_create(100);
    semafor.sem_set_value(0, 1); // semafor 0 - semafor startowy
    semafor.sem_set_value(1, 1); // semafor 1 - rozpoczynający rejsy
    semafor.sem_set_value(3, 1); // semafor 3 - pomost dla łodzi 1
    semafor.sem_set_value(4, 1); // semafor 4 - pomost dla łodzi 2

    
    pid_t pid_policjant = fork(); // tworzenie policjanta
    if (pid_policjant < 0) {
        error("fork policjant");
    } else if (pid_policjant == 0) {
        printf("Inicjowanie programu policjant\n");
        execl("./policjant", "./policjant", NULL);
        error("execl policjant");
    }
    

    pid_t pid_kasjer = fork(); // tworzenie kasjera
    if (pid_kasjer < 0) {
        error("fork kasjer");
    } else if (pid_kasjer == 0) {
        printf("Inicjowanie programu kasjer\n");
        execl("./kasjer", "./kasjer", NULL);
        error("execl kasjer");
    }

    pid_t pid_pasazer = fork(); // tworzenie pasażera
    if (pid_pasazer < 0) {
        error("fork pasazer");
    } else if (pid_pasazer == 0) {
        printf("Inicjowanie programu pasazer\n");
        execl("./pasazer", "./pasazer", NULL);
        error("execl pasazer");
    }


    semafor.sem_op(0, -1); // start symulacji po zainicjowaniu procesów


    for(int i = 0; i < 2; i++) {
        int status;
        pid_t child_pid = wait(&status);  // Czekaj na dowolny zakonczony proces

        if (child_pid == -1) {
            error("wait error");
        }

        if(child_pid == pid_policjant) {
            if(WIFEXITED(status)) 
                printf("Proces policjant (%d) zakonczony z kodem %d\n", pid_policjant, WEXITSTATUS(status));
        } else if(child_pid == pid_kasjer) {
            if(WIFEXITED(status)) 
                printf("Proces kasjer (%d) zakonczony z kodem %d\n", pid_kasjer, WEXITSTATUS(status));
        } else if(child_pid == pid_pasazer) {
            if(WIFEXITED(status)) 
                printf("Proces pasazer (%d) zakonczony z kodem %d\n", pid_pasazer, WEXITSTATUS(status));
        }
    }

    memory.shm_detach(shared_mem);
    memory.shm_delete();
    queue.msg_ctl();
    semafor.sem_remove();

    return 0;
}
