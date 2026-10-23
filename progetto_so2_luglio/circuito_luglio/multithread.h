#ifndef MULTITHREAD_H
#define MULTITHREAD_H
#include "matrix.h"
#include "vector.h"
#include "complex.h"
#include <stdlib.h>
#include <math.h>
#include <pthread.h>




typedef struct {
    int thread_id;
    int num_threads;
    int dimensione;       // Usiamo direttamente 2^n per evitare ricalcoli
    matrix matrice;       // Passiamo la struct matrice direttamente
    vector *vettoreprec;   // Puntatore al vettore di input
    vector *vettoreresult; // Puntatore al vettore di output
} thread_data;


void* moltiplica_riga(void* arg);
void applica_circuito(matrix *porte, int k, int n, vector *vinit, vector *vfin, int num_threads);
#endif