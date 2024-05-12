#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include "../src/dijkstra.h"
#include <sys/sem.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>

static int PRODUTORES_NUM = 5;
static int TAMANHO_BUFFER = 5;
// STRUCT PRINCIPAL
struct argumento_struct {
    int mutex;
    int full;
    int empty;
    int *buffer;
    int in;
    int out;
} * args;

// Produtor struct
struct produtores_struct {
    int id;
    struct argumento_struct *arg;
} * produtores;

void print_buffer(int *buffer) {
    printf("\n\t\x1B[32m== BUFFER ==\n\x1B[0m");
    for (int i = 0; i < TAMANHO_BUFFER; i++) printf("| %d ", buffer[i]);
    printf("|\n\n");
}

void *Producer(void *args) {
    struct produtores_struct *produtores_args = args;
    struct argumento_struct *_args = produtores_args->arg;
    int id = produtores_args->id;
    int produto;

    usleep(rand() % 1000000);
    P(_args->empty); // Espera o args ficar vazio
    P(_args->mutex); // Fecha o mutex
    printf("\x1B[33m> Producer %d has come into action!\n\x1B[0m", id);
    produto = rand() % 100;

    if (_args->buffer[_args->in] != -1) {
        printf("\t\x1B[33m==== ALERTA DO PRODUTOR %d ====\n\x1B[0m", id);
        printf("\t> Posição %d está ocupado com o valor %d\n\n", _args->in, _args->buffer[_args->in]);
    }
    printf("\t\x1B[33m> O Produtor %d relembra dos valores %d no POS %d\n\x1B[0m", id, produto, _args->in);
    _args->buffer[_args->in] = produto;
    // Incrementa a posição do buffer
    _args->in = (_args->in + 1) % TAMANHO_BUFFER;
    V(_args->mutex); // Desbloqueia o mutex
    V(_args->full); // Notifica os cosumidores que o buffer não está mais vazio
    return NULL;
}

void *Consumidor(void *args) {
    struct argumento_struct *_args = args;
    usleep(rand() % 1000000);
    int i = 0;

    while (i != PRODUTORES_NUM) {
        P(_args->full); // Espera o produtor estar cheio
        P(_args->mutex); // Fecha o mutex
        printf("- Consumidor entrou em ação!\n");
        print_buffer(_args->buffer);
        if (_args->buffer[_args->out] == -1) {
            printf("\t\x1B[31m==== COSUMIDOR ALERT ====\n\x1B[0m");
            printf("\t> A posição %d Está vazia\n\n", _args->in);
        }

        printf("\t- O valor do consumidor: %d\n", _args->buffer[_args->out]);

        // Limpando o buffer
        _args->buffer[_args->out] = -1;
        _args->out = (_args->out + 1) % TAMANHO_BUFFER;
        V(_args->mutex); // Desbloqueia mutex
        V(_args->empty); // Notifica o produtor
        i++;
    }

    return NULL;
}

int main() {
    key_t key0 = 0, key1 = 1, key2 = 2;
    int mutex = sem_create(key0, 1);
    int full = sem_create(key1, 0);
    int empty = sem_create(key2, 5);
    int i, in = 0, out = 0;
    int *buffer;

    buffer = malloc(TAMANHO_BUFFER * sizeof(int));
    for (i = 0; i < TAMANHO_BUFFER; i++) buffer[i] = -1;
    args = malloc(sizeof(struct argumento_struct) * 1);
    produtores = malloc(sizeof(struct produtores_struct) * PRODUTORES_NUM);
    args->mutex = mutex;
    args->full = full;
    args->empty = empty;
    args->buffer = buffer;
    args->in = in;
    args->out = out;

    pthread_t pID[PRODUTORES_NUM];
    pthread_t cID;
    srand(time(NULL));
    
    // Creates threads
    for (i = 0; i < PRODUTORES_NUM; i++) {
        produtores[i].id = i;
        produtores[i].arg = args;
        pthread_create(&pID[i], NULL, Producer, &produtores[i]);
    }

    pthread_create(&cID, NULL, Consumidor, args);
    // Espera pelos threads para finalizar o programa
    for (i = 0; i < PRODUTORES_NUM; i++) pthread_join(pID[i], NULL);
    pthread_join(cID, NULL);
    // Deleta os sem's
    sem_delete(mutex);
    sem_delete(full);
    sem_delete(empty);
    free(buffer);
    free(args);
    free(produtores);
    exit(0);
}