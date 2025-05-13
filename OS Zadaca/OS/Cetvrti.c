#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>

#define N 3     // Broj mjesta
#define M 18    // Broj posjetitelja

// Imena semafora
#define SEM_MJESTA "/sem_mjesta"
#define SEM_SIDJI  "/sem_sidji"
#define SEM_MUTEX  "/sem_mutex"

void posjetitelj(int id, int* voznja_u_tijeku, int* preostalo_posjetitelja) {
    sem_t* mjesta = sem_open(SEM_MJESTA, 0);
    sem_t* sidji = sem_open(SEM_SIDJI, 0);
    sem_t* mutex = sem_open(SEM_MUTEX, 0);

    while (1) {
        sem_wait(mutex);
        if (*voznja_u_tijeku == 0) {
            if (sem_trywait(mjesta) == 0) {
                printf("Posjetitelj %d je sjeo.\n", id);
                sem_post(mutex);
                sem_wait(sidji);  // čeka kraj vožnje
                printf("Posjetitelj %d silazi.\n", id);

                // Smanji broj preostalih posjetitelja
                sem_wait(mutex);
                (*preostalo_posjetitelja)--;
                sem_post(mutex);

                exit(0);
            }
        }
        sem_post(mutex);
        usleep(100000); // 0.1 sekunda
    }
}

void vrtuljak(int* voznja_u_tijeku, int* preostalo_posjetitelja) {
    sem_t* mjesta = sem_open(SEM_MJESTA, 0);
    sem_t* sidji = sem_open(SEM_SIDJI, 0);
    sem_t* mutex = sem_open(SEM_MUTEX, 0);

    while (1) {
        sem_wait(mutex);
        int gotovi = (*preostalo_posjetitelja == 0);
        sem_post(mutex);
        if (gotovi) break;

        int slobodno;
        do {
            sem_getvalue(mjesta, &slobodno);
            sleep(1);

            sem_wait(mutex);
            gotovi = (*preostalo_posjetitelja == 0);
            sem_post(mutex);
            if (gotovi) break;
        } while (slobodno > 0);

        if (gotovi) break;

        // Pokreni vožnju
        sem_wait(mutex);
        *voznja_u_tijeku = 1;
        sem_post(mutex);

        printf("\n[VOŽNJA] Vrtuljak se pokreće!\n");
        sleep(3);
        printf("[VOŽNJA] Vrtuljak se zaustavlja.\n");

        // Zaustavi vožnju
        sem_wait(mutex);
        *voznja_u_tijeku = 0;
        sem_post(mutex);

        // Pusti svih N posjetitelja da siđu i oslobode mjesto
        for (int i = 0; i < N; i++) {
            sem_post(sidji);
            sem_post(mjesta);
        }

        sleep(1);
    }

    printf("[INFO] Vrtuljak završava s radom.\n");
    exit(0);
}

int main() {
    // Dijeljene varijable
    int* voznja_u_tijeku = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
                                 MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *voznja_u_tijeku = 0;

    int* preostalo_posjetitelja = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
                                        MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *preostalo_posjetitelja = M;

    // Očisti stare semafore
    sem_unlink(SEM_MJESTA);
    sem_unlink(SEM_SIDJI);
    sem_unlink(SEM_MUTEX);

    // Inicijalizacija semafora
    sem_t* mjesta = sem_open(SEM_MJESTA, O_CREAT, 0644, N);
    sem_t* sidji  = sem_open(SEM_SIDJI,  O_CREAT, 0644, 0);
    sem_t* mutex  = sem_open(SEM_MUTEX, O_CREAT, 0644, 1);

    // Pokreni proces vrtuljka
    if (fork() == 0) {
        vrtuljak(voznja_u_tijeku, preostalo_posjetitelja);
    }

    // Pokreni M posjetitelja
    for (int i = 0; i < M; i++) {
        if (fork() == 0) {
            posjetitelj(i + 1, voznja_u_tijeku, preostalo_posjetitelja);
        }
        usleep(150000); // mali razmak
    }

    // Čekaj sve child procese (M + 1: posjetitelji + vrtuljak)
    for (int i = 0; i < M + 1; i++) {
        wait(NULL);
    }

    // Čišćenje
    sem_unlink(SEM_MJESTA);
    sem_unlink(SEM_SIDJI);
    sem_unlink(SEM_MUTEX);
    munmap(voznja_u_tijeku, sizeof(int));
    munmap(preostalo_posjetitelja, sizeof(int));

    return 0;
}