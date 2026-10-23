#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "complex.h"
#include "vector.h" 
#include "matrix.h"
#include "parser.h"
#include "multithread.h"
#include "misurazione.h"

// Converte un indice intero nella sua rappresentazione binaria
// su n_qubit cifre (con zeri iniziali). buffer deve avere spazio
// per almeno n_qubit+1 caratteri (incluso il terminatore).
void indice_a_binario(int indice, int n_qubit, char *buffer) {
    for (int b = n_qubit - 1; b >= 0; b--) {
        buffer[n_qubit - 1 - b] = ((indice >> b) & 1) ? '1' : '0';
    }
    buffer[n_qubit] = '\0';
}

// funzione che stampa un numero complesso nello stesso formato usato in input,

void stampa_complesso(FILE *out, complesso c) {
    fprintf(out, "%.6f", c.reale);
    if (c.immaginario >= 0.0) {
        fprintf(out, "+i%.6f", c.immaginario);
    } else {
        fprintf(out, "-i%.6f", -c.immaginario);
    }
}

//funzione che stampa un intero vettore di numeri complessi nel formato
void stampa_vettore(FILE *out, vector *v) {
    fprintf(out, "[");
    for (int i = 0; i < v->dimensione; i++) {
        stampa_complesso(out, v->componenti[i]);
        if (i < v->dimensione - 1) fprintf(out, ", ");
    }
    fprintf(out, "]\n");
}

int main(int argc, char *argv[]){
    lineacomando linea;
    array_porte arr_porte;
    matrix porte;
    vector vinit;   

    // leggo la linea di comando  e ottengo il  vettore iniziale e il numero di thread 
    parse_linea_di_comando(argc, argv, &linea);
    parse_file_inizializzazione(linea.file_inizializzazione, &vinit);

    //  alloco la matrice 
    porte.dimensione = vinit.dimensione; 
    porte.elementi = malloc(porte.dimensione * sizeof(complesso*));
    for (int i = 0; i < porte.dimensione; i++) {
        porte.elementi[i] = malloc(porte.dimensione * sizeof(complesso));
    }

    // Inizializza l'array delle porte del circuito
    arr_porte.numero_porte = 0;
    arr_porte.ha_misurazione = 0;
    arr_porte.porte = malloc(100 * sizeof(matrix)); 

    //parsing porte
    parse_file_porte(linea.file_porte, &porte, &arr_porte);

    // Alloco il vettore da usare  per il risultato finale 
    vector vfinal;
    vfinal.num_componenti = vinit.num_componenti;
    vfinal.dimensione = vinit.dimensione;
    vfinal.componenti = malloc(vfinal.dimensione * sizeof(complesso));

    //applico il circuito e in caso poi se ha misurazione eseguo la misurazione
    applica_circuito(arr_porte.porte, arr_porte.numero_porte, vinit.dimensione, &vinit, &vfinal, linea.numero_threads);

    long *conteggi_misura = NULL;
    if (arr_porte.ha_misurazione > 0) {
        conteggi_misura = (long*) malloc(vfinal.dimensione * sizeof(long));
        misura_stato(&vfinal, arr_porte.ha_misurazione, linea.numero_threads, conteggi_misura);
    }

    // Calcoliamo il numero di qubit dalla dimensione dello stato (dimensione = 2^n),
    // necessario per stampare gli esiti della misurazione in notazione binaria
    int n_qubit = (int) round(log2(vfinal.dimensione));

    // Output richiesto dal progetto: va stampato su stdout
    if (arr_porte.ha_misurazione > 0) {
        // Caso con misurazione: stampiamo la distribuzione di probabilita' stimata,
        // un esito per riga nel formato "<binario> @ <frequenza>", per tutti
        // i 2^n_qubit possibili esiti (anche quelli mai osservati)
        char label[64];
        for (int i = 0; i < vfinal.dimensione; i++) {
            indice_a_binario(i, n_qubit, label);
            double frequenza = (double) conteggi_misura[i] / (double) arr_porte.ha_misurazione;
            printf("%s @ %.4f\n", label, frequenza);
        }
    } else {
        // Caso senza misurazione: stampiamo lo stato finale nello stesso
        // formato dello stato iniziale (lista di numeri complessi)
        stampa_vettore(stdout, &vfinal);
    }

    // Log aggiuntivo su file, utile per debug ma non sostitutivo dell'output
    // richiesto su stdout
    FILE *f_log = fopen("risultato_simulazione.txt", "w");
    if (f_log == NULL) {
        perror("Errore durante la creazione del file risultato_simulazione.txt");
    } else {
        fprintf(f_log, "==================================================\n");
        fprintf(f_log, "          RISULTATO DELLA SIMULAZIONE  \n");
        fprintf(f_log, "==================================================\n");
        fprintf(f_log, "Numero thread utilizzati: %d\n", linea.numero_threads);
        fprintf(f_log, "Dimensione dello stato (2^n): %d\n", vinit.dimensione);
        fprintf(f_log, "Numero di qubit: %d\n", n_qubit);
        fprintf(f_log, "Numero di porte applicate: %d\n", arr_porte.numero_porte);
        if (arr_porte.ha_misurazione > 0) {
            fprintf(f_log, "Ripetizioni misurazione: %d\n", arr_porte.ha_misurazione);
        }
        fprintf(f_log, "--------------------------------------------------\n");
        fprintf(f_log, "Stato finale del vettore (Ampiezze di probabilita):\n");

        // Stampiamo le componenti di vfinal (il risultato vero) nel log
        for (int i = 0; i < vfinal.dimensione; i++) {
            fprintf(f_log, "Stato [%d]: %.4f + i(%.4f)\n", 
                    i, vfinal.componenti[i].reale, vfinal.componenti[i].immaginario);
        }

        // Se e' stata richiesta una misurazione, aggiungiamo anche il riepilogo nel log
        if (conteggi_misura != NULL) {
            fprintf(f_log, "--------------------------------------------------\n");
            fprintf(f_log, "Risultati della misurazione (%d ripetizioni):\n", arr_porte.ha_misurazione);
            char label[64];
            for (int i = 0; i < vfinal.dimensione; i++) {
                indice_a_binario(i, n_qubit, label);
                double frequenza = (double) conteggi_misura[i] / (double) arr_porte.ha_misurazione;
                fprintf(f_log, "%s (indice %d): %ld volte (frequenza %.4f)\n",
                        label, i, conteggi_misura[i], frequenza);
            }
        }

        fprintf(f_log, "==================================================\n");

        fclose(f_log);
    }

    // Liberiamo la memoria profonda allocata dal parser per le matrici del circuito
    for (int p = 0; p < arr_porte.numero_porte; p++) {
        if (arr_porte.porte[p].elementi != NULL) {
            for (int i = 0; i < arr_porte.porte[p].dimensione; i++) {
                free(arr_porte.porte[p].elementi[i]);
            }
            free(arr_porte.porte[p].elementi);
        }
    }
    free(arr_porte.porte);
    
    // Pulizia matrice di supporto globale
    for (int i = 0; i < porte.dimensione; i++) {
        free(porte.elementi[i]);
    }
    free(porte.elementi);
    
    // Pulizia vettori
    free(vinit.componenti);
    free(vfinal.componenti);

    // Pulizia conteggi della misurazione, se allocati
    if (conteggi_misura != NULL) {
        free(conteggi_misura);
    }

    return 0;
}