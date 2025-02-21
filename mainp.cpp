#include <iostream>
#include "classes.h"


#define K 10
#define N1 30
#define N2 25
#define T1 1
#define T2 2
#define Tp 1
#define Tk 10

using namespace std;

int* shared_mem;

int shmid;
int semid;
int msgid;

void signal_handler(int sig) {
    // Bezpieczne odłączenie pamięci współdzielonej
    if (shared_mem != NULL) {
        if (shmdt(shared_mem) == -1) perror("shmdt error");
    }

    // Usunięcie segmentu pamięci współdzielonej
    if (shmctl(shmid, IPC_RMID, NULL) == -1) perror("shmctl error");

    // Usunięcie kolejki komunikatów
    if (msgctl(msgid, IPC_RMID, NULL) == -1) perror("msgctl error");

    // Usunięcie zestawu semaforów
    if (semctl(semid, 0, IPC_RMID) == -1) perror("semctl IPC_RMID error");

    // Zabijanie grupy procesów
    if (killpg(getpgrp(), SIGINT) == -1) perror("killpg error");

    printf("[MAINP]: Pomyślnie zakończono działanie programu!\n");

    // Wyjście z programu
    exit(EXIT_SUCCESS);
}




int main() {

    signal(SIGINT, signal_handler);

    pid_t mainp = getpid();

    setpgid(0, 0);

    SharedMem memory(1234, 1024);
    memory.shm_create();
    int* shared_mem = memory.shm_get();
    shmid = memory.id;

    for(int i = 0; i < 1024; i++) {
        shared_mem[i] = 0;
    }

    MsgQueue queue(1234);
    queue.msg_create();
    msgid = queue.id;
    

    Sem semafor(1234);
    semafor.sem_create(100);
    semid = semafor.id;
    semafor.sem_set_value(0, 4); // semafor 0 - semafor startowy
    semafor.sem_set_value(1, 1); // semafor 1 - pamiec[1] obecni pasażerowie
    semafor.sem_set_value(2, 1); // semafor 2 - pamiec[2] obecni pasażerowie
    semafor.sem_set_value(3, K); // semafor 3 - pomost dla łodzi 1
    semafor.sem_set_value(4, K); // semafor 4 - pomost dla łodzi 2
    semafor.sem_set_value(5, 0); // semafor 5 - vip1 
    semafor.sem_set_value(6, 0); // semafor 6 - vip2 
    semafor.sem_set_value(7, 0); // semafor 7 - wyładunek1 
    semafor.sem_set_value(8, 0); // semafor 8 - wyładunek2
    semafor.sem_set_value(9, 0); // semafor 9 - załadunek1 
    semafor.sem_set_value(10, 0); // semafor 10 - załadunek2
    semafor.sem_set_value(11, 2); // semafor kierunek pomostu1
    semafor.sem_set_value(12, 2); // semafor kierunek pomostu2
    semafor.sem_set_value(13, 0); // semafor tworzenie pasażerów

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

    shared_mem[7] = mainp;

    queue.msg_send(27);

    sleep(Tk - Tp);
    shared_mem[8] = 1;
    printf("[Czas]: Godzina Tk, %d\n", shared_mem[8]);


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
