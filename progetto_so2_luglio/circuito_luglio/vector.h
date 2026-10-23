#ifndef VECTOR_H
#define VECTOR_H
#include "complex.h"

typedef struct {
    int num_componenti;//numero di qubit

    int dimensione;//stato dei 2^n qubit

    complesso *componenti;//le componenti del vettore che sono complessi
}vector;
vector crea_vector(int n_componenti);


#endif