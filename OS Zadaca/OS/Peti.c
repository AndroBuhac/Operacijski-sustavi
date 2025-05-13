#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define N 5

// Stanja filozofa: 0 = misli, 1 = gladan, 2 = jede
enum {MISLI, GLADAN, JEDE};

pthread_mutex_t monitor = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t red[N];
int filozof[N];     // stanja filozofa: O, o, X
int vilica[N];      // 1 = dostupna, 0 = zauzeta

void ispiši_stanje() {
    printf("Stanje filozofa: ");
    for (int i = 0; i < N; i++) {
        if (filozof[i] == MISLI) printf("O ");
        else if (filozof[i] == GLADAN) printf("o ");
        else if (filozof[i] == JEDE) printf("X ");
    }
    printf("\n");
    fflush(stdout);
}

void misliti() {
    sleep(3); // simulacija razmišljanja
}

void jesti(int n) {
    pthread_mutex_lock(&monitor);

    filozof[n] = GLADAN;
    ispiši_stanje();

    while (vilica[n] == 0 || vilica[(n + 1) % N] == 0) {
        pthread_cond_wait(&red[n], &monitor);
    }

    vilica[n] = 0;
    vilica[(n + 1) % N] = 0;
    filozof[n] = JEDE;
    ispiši_stanje();

    pthread_mutex_unlock(&monitor);

    sleep(2); // njam njam
}

void pusti_vilice(int n) {
    pthread_mutex_lock(&monitor);

    filozof[n] = MISLI;
    vilica[n] = 1;
    vilica[(n + 1) % N] = 1;

    pthread_cond_signal(&red[(n + N - 1) % N]);
    pthread_cond_signal(&red[(n + 1) % N]);

    ispiši_stanje();

    pthread_mutex_unlock(&monitor);
}

void* filozof_dretva(void* arg) {
    int i = *(int*)arg;
    free(arg);

    while (1) {
        misliti();
        jesti(i);
        pusti_vilice(i);
    }

    return NULL;
}

int main() {
    pthread_t dretve[N];

    for (int i = 0; i < N; i++) {
        filozof[i] = MISLI;
        vilica[i] = 1;
        pthread_cond_init(&red[i], NULL);
    }

    for (int i = 0; i < N; i++) {
        int* id = malloc(sizeof(int));
        *id = i;
        pthread_create(&dretve[i], NULL, filozof_dretva, id);
    }

    for (int i = 0; i < N; i++) {
        pthread_join(dretve[i], NULL);
    }

    return 0;
}
