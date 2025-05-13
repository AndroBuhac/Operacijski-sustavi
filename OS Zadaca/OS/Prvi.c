#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>

// Globalne varijable koje se koriste za praćenje trenutnog broja i stanja programa
volatile sig_atomic_t current_number = 0; // Trenutni broj koji se obrađuje
volatile sig_atomic_t running = 1;       // Zastavica koja označava da program radi

// Obrada signala SIGUSR1 - ispisuje trenutni broj koji se obrađuje
void sigusr1_handler(int sig) {
    printf("Trenutni broj u obradi: %d\n", current_number);
}

// Obrada signala SIGTERM - zapisuje trenutni broj u datoteku `status.txt` i prekida rad programa
void sigterm_handler(int sig) {
    FILE *status_file = fopen("status.txt", "w");
    if (status_file == NULL) {
        perror("Ne mogu otvoriti status.txt");
        exit(EXIT_FAILURE);
    }
    fprintf(status_file, "%d\n", current_number); // Sprema trenutni broj
    fclose(status_file);
    running = 0; // Zaustavlja glavni radni loop
}

// Obrada signala SIGINT - samo prekida rad programa bez zapisivanja statusa
void sigint_handler(int sig) {
    running = 0;
}

// Funkcija za čitanje broja iz datoteke `status.txt`
int procitaj_broj_iz_statusa() {
    FILE *status_file = fopen("status.txt", "r");
    if (status_file == NULL) {
        perror("Ne mogu otvoriti status.txt");
        exit(EXIT_FAILURE);
    }
    int broj;
    fscanf(status_file, "%d", &broj); // Čita broj iz datoteke
    fclose(status_file);
    return broj;
}

// Funkcija za zapisivanje broja u datoteku `status.txt`
void zapisi_u_status(int broj) {
    FILE *status_file = fopen("status.txt", "w");
    if (status_file == NULL) {
        perror("Ne mogu otvoriti status.txt");
        exit(EXIT_FAILURE);
    }
    fprintf(status_file, "%d\n", broj); // Sprema broj u datoteku
    fclose(status_file);
}

// Funkcija za dohvaćanje posljednjeg broja iz datoteke `obrada.txt`
int zadnji_broj_iz_obrade() {
    FILE *obrada_file = fopen("obrada.txt", "r");
    if (obrada_file == NULL) {
        perror("Ne mogu otvoriti obrada.txt");
        exit(EXIT_FAILURE);
    }
    int broj = 0, temp;
    while (fscanf(obrada_file, "%d", &temp) != EOF) {
        broj = temp; // Sprema posljednji pročitani broj
    }
    fclose(obrada_file);
    return broj;
}

// Funkcija za dodavanje broja u datoteku `obrada.txt`
void dodaj_u_obrada(int broj) {
    FILE *obrada_file = fopen("obrada.txt", "a");
    if (obrada_file == NULL) {
        perror("Ne mogu otvoriti obrada.txt");
        exit(EXIT_FAILURE);
    }
    fprintf(obrada_file, "%d\n", broj); // Dodaje broj na kraj datoteke
    fclose(obrada_file);
}

// Glavna funkcija programa
int main() {
    // Postavljanje rukovatelja signala
    signal(SIGUSR1, sigusr1_handler);
    signal(SIGTERM, sigterm_handler);
    signal(SIGINT, sigint_handler);

    // Čitanje statusa iz datoteke `status.txt`
    int status = procitaj_broj_iz_statusa();
    if (status == 0) { // Ako je status 0, započinje obradu od korijena zadnjeg broja
        int zadnji_broj = zadnji_broj_iz_obrade();
        current_number = zadnji_broj > 0 ? (int)sqrt(zadnji_broj) + 1 : 1; // Korijenuje zadnji broj i počinje od sljedećeg
    } else {
        current_number = status; // Nastavlja od spremljenog statusa
    }

    zapisi_u_status(0); // Označava da je obrada u tijeku (status 0)

    // Glavna petlja obrade
    while (running) {
        int kvadrat = current_number * current_number; // Računa kvadrat trenutnog broja
        dodaj_u_obrada(kvadrat); // Sprema kvadrat u datoteku `obrada.txt`
        printf("Obrađujem broj: %d, kvadrat: %d\n", current_number, kvadrat);
        current_number++; // Povećava trenutni broj
        sleep(5); // Simulira dugotrajnu obradu (pauza od 5 sekundi)
    }

    return 0; // Završava program
}