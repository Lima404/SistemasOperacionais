#include <sys/ipc.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <pthread.h>
#include <time.h>
#include "../src/dijkstra.h"

static int Maximo_threads = 5;

struct arg_struct {
    int mutex;
    int wrt;
    int shared;
    int contador_leitores;
} * args;

void *writer(void *args) {
    struct arg_struct *_args = args;
    P(_args->wrt);

    _args->shared = rand() % 100;
    printf("> Escritor: compartilhou = %d\n", _args->shared);
    V(_args->wrt);

    return NULL;
}

void *reader(void *args) {
    struct arg_struct *_args = args;
    P(_args->mutex);
    _args->contador_leitores++;

    if (_args->contador_leitores == 1)
        P(_args->wrt);

    printf("\t- Leitor: compartilhou = %d\n", _args->shared);
    _args->contador_leitores--;
    if (_args->contador_leitores == 0)
        V(_args->wrt);
    V(_args->mutex);

    return NULL;
}

int main() {
    key_t key0 = 0, key1 = 1;

    int mutex = sem_create(key0, 1);
    int wrt = sem_create(key1, 1);
    int contador_leitores = 0;
    int shared = 0;
    int i;
    args = malloc(sizeof(struct arg_struct) * 1);
    args->mutex = mutex;
    args->wrt = wrt;
    args->shared = shared;
    args->contador_leitores = contador_leitores;

    pthread_t rID[Maximo_threads];
    pthread_t wID[Maximo_threads];

    // Criando threads
    for (i = 0; i < Maximo_threads; i++) {
        pthread_create(&rID[i], NULL, reader, args);
        pthread_create(&wID[i], NULL, writer, args);
    }
    // Entrando nos threads
    for (i = 0; i < Maximo_threads; i++) {
        pthread_join(rID[i], NULL);
        pthread_join(wID[i], NULL);
    }
    // Deleta os sem's
    sem_delete(mutex);
    sem_delete(wrt);

    free(args);
    exit(0);
}