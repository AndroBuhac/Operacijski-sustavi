#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

int *ULAZ, *BROJ, *REZ;
int brSt, brDr;

pthread_t *dretve;

int max_broj() {
    int max = 0;
    for (int i = 0; i < brDr; i++) {
        if (BROJ[i] > max)
            max = BROJ[i];
    }
    return max;
}

void udji_u_kriticni_odsjecak(int i) {
    ULAZ[i] = 1;
    BROJ[i] = max_broj() + 1;
    ULAZ[i] = 0;

    for (int j = 0; j < brDr; j++) {
        while (ULAZ[j]) {} // čekaj da dretva j završi s postavljanjem svog BROJ
        while (BROJ[j] != 0 &&
              (BROJ[j] < BROJ[i] || (BROJ[j] == BROJ[i] && j < i))) {
            // čekaj jer dretva j ima prioritet
        }
    }
}

void izadji_iz_kriticnog_odsjecka(int i) {
    BROJ[i] = 0;
}

int sve_zauzeto() {
    for (int i = 0; i < brSt; i++) {
        if (REZ[i] == -1)
            return 0;
    }
    return 1;
}

void ispisi_stanje() {
    for (int i = 0; i < brSt; i++) {
        if (REZ[i] == -1)
            printf("-");
        else
            printf("%d", REZ[i] + 1); // ispis broj dretve
    }
    printf("\n");
}

void *rezervacija(void *arg) {
    int id = *((int *)arg);
    free(arg);

    while (1) {
        sleep(1);

        if (sve_zauzeto())
            break;

        int stol = rand() % brSt;
        printf("Dretva %d: odabirem stol %d\n", id + 1, stol + 1);
        fflush(stdout);

        udji_u_kriticni_odsjecak(id);

        if (REZ[stol] == -1) {
            REZ[stol] = id;
            printf("Dretva %d: rezerviram stol %d, stanje:\n", id + 1, stol + 1);
        } else {
            printf("Dretva %d: neuspjela rezervacija stola %d, stanje:\n", id + 1, stol + 1);
        }

        ispisi_stanje();
        izadji_iz_kriticnog_odsjecka(id);
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Upotreba: %s <broj_dretvi> <broj_stolova>\n", argv[0]);
        return 1;
    }

    brDr = atoi(argv[1]);
    brSt = atoi(argv[2]);

    ULAZ = calloc(brDr, sizeof(int));
    BROJ = calloc(brDr, sizeof(int));
    REZ  = malloc(brSt * sizeof(int));
    dretve = malloc(brDr * sizeof(pthread_t));

    for (int i = 0; i < brSt; i++)
        REZ[i] = -1;

    srand(time(NULL));

    for (int i = 0; i < brDr; i++) {
        int *id = malloc(sizeof(int));
        *id = i;
        pthread_create(&dretve[i], NULL, rezervacija, id);
    }

    for (int i = 0; i < brDr; i++) {
        pthread_join(dretve[i], NULL);
    }

    free(ULAZ);
    free(BROJ);
    free(REZ);
    free(dretve);

    return 0;
}
