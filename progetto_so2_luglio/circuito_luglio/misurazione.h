#ifndef MISURAZIONE_H
#define MISURAZIONE_H
#include "complex.h"
#include "vector.h"



// Esegue 'ripetizioni' misurazioni dello stato vfinal usando num_threads thread.
// conteggi_finali deve essere un array gia' allocato dal chiamante, di lunghezza
// vfinal->dimensione: alla fine conteggi_finali[i] contiene quante volte e'
// stato osservato lo stato i.
void misura_stato(vector *vfinal, int ripetizioni, int num_threads, long *conteggi_finali);

#endif