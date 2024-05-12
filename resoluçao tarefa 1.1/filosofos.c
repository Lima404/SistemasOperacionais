#include <sys/types.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <time.h>

#include "../src/dijkstra.h"

static int PHILO_NUM = 5;
// Struct Principal
struct argumento_struct {
    int *mutex_chopstick;
    int *chopstick_use;
} * args;

// Struct do Filosofo
struct Filosofo_struct {
    int id;
    struct arg_struct *args;
} * filosofos;

void *Philosopher(void *philo_args) {
    struct Filosofo_struct *philosopher_args = philo_args;
    struct arg_struct *_args = philosopher_args->args;
    int id = philosopher_args->id;
    int c1, c2;
    usleep(rand() % 1000000);
    if (id % 2 == 0) {
        c1 = id;                    // Esquerda
        c2 = (id + 1) % PHILO_NUM;  // Direita
    } else {
        c1 = (id + 1) % PHILO_NUM;  // Direita
        c2 = id;                    // Esquerda
    }
    // Espera pelos chopsticks
    P(_args->mutex_chopstick[c1]);
    P(_args->mutex_chopstick[c2]);

    if (_args->chopstick_use[c1] != -1) printf("\x1B[33mALERT: chopstick %d being used by %d\n\x1B[0m", c1, _args->chopstick_use[c1]);
    if (_args->chopstick_use[c2] != -1) printf("\x1B[33mALERT: chopstick %d being used by %d\n\x1B[0m", c2, _args->chopstick_use[c2]);
    _args->chopstick_use[c1] = id;
    _args->chopstick_use[c2] = id;

    printf("\x1B[32m+ Filosofo %d pegou o chopstick [%d] e [%d]\x1B[0m\n", id, c1, c2);

    printf("\t> Filosofo %d comendo\n", id);
    usleep(rand() % 1000000);
    _args->chopstick_use[c1] = -1;
    _args->chopstick_use[c2] = -1;
    printf("\x1B[31m- Filosofo %d liberou o chopstick [%d] e [%d]\x1B[0m\n", id, c1, c2);
    // Sinal do chopsticks
    V(_args->mutex_chopstick[c1]);
    V(_args->mutex_chopstick[c2]);

    return NULL;
}

int main() {
    pthread_t pID[PHILO_NUM];
    int *chopstick_use = malloc(sizeof(int) * PHILO_NUM);
    int *mutex_chopstick = malloc(sizeof(int) * PHILO_NUM);
    int i;

    args = malloc(sizeof(struct arg_struct) * 1);
    filosofos = malloc(sizeof(struct Filosofo_struct) * PHILO_NUM);

    // Initialize chopsticks and sem's
    for (i = 0; i < PHILO_NUM; i++) {
        chopstick_use[i] = -1;
        mutex_chopstick[i] = sem_create(i, 1);
    }

    args->mutex_chopstick = mutex_chopstick;
    args->chopstick_use = chopstick_use;
    srand(time(NULL));
    // Creates threads
    for (i = 0; i < PHILO_NUM; i++) {
        filosofos[i].id = i;
        filosofos[i].args = args;
        pthread_create(&pID[i], NULL, Philosopher, &filosofos[i]);
    }

    // Espera pelos threads para finalisar
    for (i = 0; i < PHILO_NUM; i++) pthread_join(pID[i], NULL);
    // Deleta os sem's
    for (i = 0; i < PHILO_NUM; i++) sem_delete(mutex_chopstick[i]);

    free(chopstick_use);
    free(mutex_chopstick);
    free(filosofos);
    free(args);
    exit(0);
}