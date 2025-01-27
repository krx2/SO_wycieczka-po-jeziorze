#include <iostream>
#include "classes.h"


#define K 10
#define N1 30
#define N2 25
#define T1 1
#define T2 2

using namespace std;


void signal_handler(int sig) {
    killpg(getpgrp(), SIGINT);
}


int main() {

    signal(SIGINT, signal_handler);

    pid_t mainp = getpid();

    setpgid(0, 0);

    SharedMem memory(1234, 1024);
    memory.shm_create();
    int* shared_mem = memory.shm_get();

    for(int i = 0; i < 1024; i++) {
        shared_mem[i] = 0;
    }

    SharedMem pamiec_pomost(1234, K);
    pamiec_pomost.shm_create();
    int* pomost = pamiec_pomost.shm_get();

    MsgQueue queue(1234);
    queue.msg_create();
    

    Sem semafor(1234);
    semafor.sem_create(100);
    semafor.sem_set_value(0, 1); // semafor 0 - semafor startowy
    semafor.sem_set_value(1, 1); // semafor 1 - pamiec[1]
    semafor.sem_set_value(1, 1); // semafor 2 - pamiec[2]
    semafor.sem_set_value(3, 1); // semafor 3 - pomost dla łodzi 1 pamiec[3]
    semafor.sem_set_value(4, 1); // semafor 4 - pomost dla łodzi 2 pamiec[4]
    semafor.sem_set_value(5, 1); // semafor 5 - vip1 pamiec[5]
    semafor.sem_set_value(6, 1); // semafor 6 - vip2 pamiec[6]

    semafor.sem_set_value(0, 4);

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

    
    pid_t pid_sternik = fork(); // tworzenie sternika
    if (pid_sternik < 0) {
        error("fork sternik");
    } else if (pid_sternik == 0) {
        printf("Inicjowanie programu sternik\n");
        execl("./sternik", "./sternik", NULL);
        error("execl sternik");
    }


    for(int i = 0; i < 4; i++) {
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
        } else if(child_pid == pid_sternik) {
            if(WIFEXITED(status)) 
                printf("Proces sternik (%d) zakonczony z kodem %d\n", pid_sternik, WEXITSTATUS(status));
        }
    }

    memory.shm_detach(shared_mem);
    memory.shm_delete();
    queue.msg_ctl();
    semafor.sem_remove();

    return 0;
}
