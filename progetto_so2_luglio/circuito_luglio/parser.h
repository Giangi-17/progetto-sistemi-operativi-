#ifndef PARSER_H
#define PARSER_H
#include "matrix.h"
#include "vector.h"
#include "complex.h"

typedef struct {
    int numero_threads; // Corretto il carattere '§' residuo
    char *file_inizializzazione;
    char *file_porte;
} lineacomando;

typedef struct {
    matrix *porte;
    int numero_porte;
    int ha_misurazione; // Uniformato il nome del campo rispetto a prima
} array_porte;

void parse_linea_di_comando(int argc, char *argv[], lineacomando *linea);
complesso parse_complex(char **p);
void parse_file_inizializzazione(char *filename, vector *vinit);
void parse_file_porte(char *filename, matrix *porte, array_porte *arr_porte);   
#endif
