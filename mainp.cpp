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

    
    pid_t pid_policjant = fork();
    if (pid_policjant < 0) {
        error("fork policjant");
    } else if (pid_policjant == 0) {
        printf("Inicjowanie programu policjant\n");
        execl("./policjant", "./policjant", NULL);
        error("execl policjant");
    }
    

    pid_t pid_kasjer = fork();
    if (pid_kasjer < 0) {
        error("fork kasjer");
    } else if (pid_kasjer == 0) {
        printf("Inicjowanie programu kasjer\n");
        execl("./kasjer", "./kasjer", NULL);
        error("execl kasjer");
    }

    pid_t pid_pasazer = fork();
    if (pid_pasazer < 0) {
        error("fork pasazer");
    } else if (pid_pasazer == 0) {
        printf("Inicjowanie programu pasazer\n");
        execl("./pasazer", "./pasazer", NULL);
        error("execl pasazer");
    }





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

    return 0;
}
