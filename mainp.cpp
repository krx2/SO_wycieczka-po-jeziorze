#include <iostream>
#include "classes.h"

using namespace std;

int* shared_mem = nullptr;

int shmid;
int semid;
int msgid;

pid_t mainp;
pid_t pid_policjant;
pid_t pid_kasjer;
pid_t pid_pasazer;
pid_t pid_sternik;
pid_t pid_sternik1;
pid_t pid_sternik2;

void signal_handler(int sig) {
    // Bezpieczne odłączenie pamięci współdzielonej
    if (shared_mem != NULL) {
        if (shmdt(shared_mem) == -1) perror("shmdt error");
    }

    kill(pid_pasazer, SIGUSR2);
    kill(pid_pasazer, SIGTERM);
    kill(pid_sternik, SIGINT);
    kill(pid_kasjer, SIGINT);
    kill(pid_policjant, SIGINT);

    // Usunięcie segmentu pamięci współdzielonej
    if (shmctl(shmid, IPC_RMID, NULL) == -1) perror("shmctl error");

    // Usunięcie kolejki komunikatów
    if (msgctl(msgid, IPC_RMID, NULL) == -1) perror("msgctl error");

    // Usunięcie zestawu semaforów
    if (semctl(semid, 0, IPC_RMID) == -1) perror("semctl IPC_RMID error");

    printf("\033[35m[MAINP]\033[0m: Pomyślnie zakończono działanie programu!\n");

    // Wyjście z programu
    exit(EXIT_SUCCESS);
}




int main() {

    signal(SIGINT, signal_handler);

    pid_t mainp = getpid();

    setpgid(0, 0);

    SharedMem memory(1234, 12);
    memory.shm_create();
    int* shared_mem = memory.shm_get();
    shmid = memory.id;

    for(int i = 0; i < 12; i++) {
        shared_mem[i] = 0;
    }

    MsgQueue queue(1234);
    queue.msg_create();
    msgid = queue.id;
    

    Sem semafor(1234);
    semafor.sem_create(15);
    semid = semafor.id;
    semafor.sem_set_value(0, 0); // semafor 0 - semafor startowy
    semafor.sem_set_value(1, 1); // semafor 1 - blokada pamiec[1] obecni pasażerowie
    semafor.sem_set_value(2, 1); // semafor 2 - blokada pamiec[2] obecni pasażerowie
    semafor.sem_set_value(3, K); // semafor 3 - pomost dla łodzi 1
    semafor.sem_set_value(4, K); // semafor 4 - pomost dla łodzi 2
    semafor.sem_set_value(5, 0); // semafor 5 - vip1 
    semafor.sem_set_value(6, 0); // semafor 6 - vip2 
    semafor.sem_set_value(7, 0); // semafor 7 - wyładunek1 
    semafor.sem_set_value(8, 0); // semafor 8 - wyładunek2
    semafor.sem_set_value(9, 0); // semafor 9 - załadunek1 
    semafor.sem_set_value(10, 0); // semafor 10 - załadunek2
    semafor.sem_set_value(11, 0); // semafor 11 - kolejka na molo dla łodzi 1
    semafor.sem_set_value(12, 0); // semafor 11 - kolejka na molo dla łodzi 2
    semafor.sem_set_value(13, 0); // semafor 13 - tworzenie pasażerów
    semafor.sem_set_value(14, 0); // semafor 14 - końcowy

    pid_policjant = fork(); // tworzenie policjanta
    if (pid_policjant < 0) {
        error("fork policjant");
    } else if (pid_policjant == 0) {
        printf("\033[35m[MAINP]\033[0m: Inicjowanie programu policjant\n");
        execl("./policjant", "./policjant", NULL);
        error("execl policjant");
    }

    pid_kasjer = fork(); // tworzenie kasjera
    if (pid_kasjer < 0) {
        error("fork kasjer");
    } else if (pid_kasjer == 0) {
        printf("\033[35m[MAINP]\033[0m: Inicjowanie programu kasjer\n");
        execl("./kasjer", "./kasjer", NULL);
        error("execl kasjer");
    }

    pid_pasazer = fork(); // tworzenie pasażera
    if (pid_pasazer < 0) {
        error("fork pasazer");
    } else if (pid_pasazer == 0) {
        printf("\033[35m[MAINP]\033[0m: Inicjowanie programu pasazer\n");
        execl("./pasazer", "./pasazer", NULL);
        error("execl pasazer");
    }

    
    pid_sternik = fork(); // tworzenie sternika
    if (pid_sternik < 0) {
        error("fork sternik");
    } else if (pid_sternik == 0) {
        printf("\033[35m[MAINP]\033[0m: Inicjowanie programu sternik\n");
        execl("./sternik", "./sternik", NULL);
        error("execl sternik");
    }

    semafor.sem_op(0, 4);

    shared_mem[7] = mainp;

    queue.msg_send(27);

    printf("\033[35m[MAINP]\033[0m: PID: %d\n", getpid());

    sleep(Tk - Tp);
    shared_mem[8] = 1;
    printf("\033[35m[CZAS]\033[0m: Godzina Tk\n");

    pause();

    return 0;
}
