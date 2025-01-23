#include <iostream>
#include "classes.h"

using namespace std;

int main() {
    
    pid_t pid_sternik = getpid();

    pid_t pid_sternik1 = fork();
    if (pid_sternik1 < 0) {
        error("fork policjant");
    }

    pid_t pid_sternik2;
    if(getpid() == pid_sternik) {
        pid_t pid_sternik1 = fork();
        if (pid_sternik1 < 0) {
            error("fork policjant");
        }
    }

    while(true) {
        if(getpid() == pid_sternik1) {

        }

        if(getpid() == pid_sternik2) {
            
        }
    }
}