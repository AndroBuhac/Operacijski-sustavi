#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

int *PRAVO, *ZASTAVICA;
int pravoId, zastavicaId;

void udi_u_kriticni_odsjecak(int i, int j) {
    ZASTAVICA[i] = 1;
    while (ZASTAVICA[j] != 0) {
        if (*PRAVO == j) {
            ZASTAVICA[i] = 0;
            while (*PRAVO == j) {
                // busy wait
            }
            ZASTAVICA[i] = 1;
        }
    }
}

void izadji_iz_kriticnog_odsjecka(int i, int j) {
    *PRAVO = j;
    ZASTAVICA[i] = 0;
}

void proc(int i, int j) {
    for (int k = 1; k <= 5; k++) {
        udi_u_kriticni_odsjecak(i, j);
        for (int m = 1; m <= 5; m++) {
            printf("i: %d, k: %d, m: %d\n", i, k, m);
            sleep(1);
        }
        izadji_iz_kriticnog_odsjecka(i, j);
    }
}

void brisi() {
    shmdt(PRAVO);
    shmdt(ZASTAVICA);
    shmctl(pravoId, IPC_RMID, NULL);
    shmctl(zastavicaId, IPC_RMID, NULL);
    exit(0);
}

int main() {
    pravoId = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0660);
    zastavicaId = shmget(IPC_PRIVATE, sizeof(int) * 2, IPC_CREAT | 0660);
    PRAVO = (int *)shmat(pravoId, NULL, 0);
    ZASTAVICA = (int *)shmat(zastavicaId, NULL, 0);

    *PRAVO = 0;
    ZASTAVICA[0] = 0;
    ZASTAVICA[1] = 0;

    signal(SIGINT, brisi);

    pid_t pid1 = fork();
    if (pid1 == 0) {
        proc(1, 0);
        exit(0);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        proc(0, 1);
        exit(0);
    }

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    brisi();

    return 0;
}