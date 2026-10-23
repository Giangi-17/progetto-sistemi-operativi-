#ifndef MATRIX_H
#define MATRIX_H
#include "complex.h"
#include "vector.h"


typedef struct{
    char nome[36];
    int num;
    int dimensione;
    complesso **elementi;

}  matrix;

matrix crea_matrix(int n_componenti);
complesso get_elemento(matrix m, int i , int j);
matrix prodotto_scalare(matrix m, complesso c);

#endif