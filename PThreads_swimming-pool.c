#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// inicializacia premennych a mutexov
int poradie = 0;
int pocet_navstevnikov = 0;
int pocitadlo_zeny = 0;
int pocitadlo_deti = 0;
int pocitadlo_muzi = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cakaj_na_sprchu = PTHREAD_COND_INITIALIZER;
pthread_cond_t cakaj_na_bazen = PTHREAD_COND_INITIALIZER;

// funkcia na simuláciu kupania (2s)
void kupanie(char nav)
{
    sleep(2);
}

// funkcia na simulaciu sprchovania (1s)
void sprchovanie(char nav)
{
    sleep(1);
}

// funkcia na simulaciu odpocinku (3s)
void odpocivanie(char nav)
{
    sleep(3);
}

// funkcia pre kazdeho navstevnika
void *navstevnik(void* n)
{
    char nav = (char)(intptr_t)n;
    int fronta[3];  // poradie

    // 3 navstevy bazena
    for (int i = 0; i < 3; i++)
    {
        odpocivanie(nav);  // odpocinok pred kazdym vstupom do bazena
        pthread_mutex_lock(&mutex);

        // cakaj na volnu sprchu podla toho, ci ide o zenu, muza alebo dieta
        while ((nav == 'Z' && pocitadlo_zeny >= 6) || (nav == 'D' && pocitadlo_deti >= 12) || (nav == 'M' && pocitadlo_muzi >= 6)) {
            pthread_cond_wait(&cakaj_na_sprchu, &mutex);
        }

        // aktualizujem pocet sprchujucich sa podla toho, o koho ide
        if (nav == 'Z') pocitadlo_zeny++;
        if (nav == 'D') pocitadlo_deti++;
        if (nav == 'M') pocitadlo_muzi++;
        pthread_mutex_unlock(&mutex);

        sprchovanie(nav);
        pthread_mutex_lock(&mutex);

        // cakaj, kym sa uvolni miesto v bazene (max. 20 navstevnikov)
        while (pocet_navstevnikov >= 20) {
            pthread_cond_wait(&cakaj_na_bazen, &mutex);
        }
        fronta[i] = ++poradie;
        printf(" %c vstupil(a) ako %d. v poradi\n", nav, fronta[i]);
        pocet_navstevnikov++;
        pthread_mutex_unlock(&mutex);

        kupanie(nav);

        pthread_mutex_lock(&mutex);
        pocet_navstevnikov--;
        pthread_cond_signal(&cakaj_na_bazen);  // signal, ze uz je miesto v bazene
        pthread_mutex_unlock(&mutex);

        sprchovanie(nav);

        pthread_mutex_lock(&mutex);

        // aktualizujeme pocty sprchujucich sa podla toho o koho ide
        if (nav == 'Z') pocitadlo_zeny--;
        if (nav == 'D') pocitadlo_deti--;
        if (nav == 'M') pocitadlo_muzi--;

        pthread_cond_broadcast(&cakaj_na_sprchu);  // sprcha je volna
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main(void)
{
    int i;
    pthread_t zeny[10];
    pthread_t muzi[10];
    pthread_t deti[20];

    // vytvorenie vlakien
    for (i = 0; i < 10; ++i) pthread_create(&zeny[i], NULL, navstevnik, (void*)'Z');
    for (i = 0; i < 10; ++i) pthread_create(&muzi[i], NULL, navstevnik, (void*)'M');
    for (i = 0; i < 20; ++i) pthread_create(&deti[i], NULL, navstevnik, (void*)'D');

    // cakanie
    for (i = 0; i < 10; ++i) pthread_join(zeny[i], NULL);
    for (i = 0; i < 10; ++i) pthread_join(muzi[i], NULL);
    for (i = 0; i < 20; ++i) pthread_join(deti[i], NULL);

    exit(EXIT_SUCCESS);
}
