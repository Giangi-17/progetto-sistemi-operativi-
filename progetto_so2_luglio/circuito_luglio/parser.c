#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <unistd.h>
#include "complex.h"
#include "vector.h"
#include "matrix.h"
#include "parser.h"




// 1. PARSING DELLA LINEA DI COMANDO : ottengo i parametri di input riconoscendo ddeterminati flag 
void parse_linea_di_comando(int argc, char *argv[], lineacomando *linea) {
    int opt;
    while ((opt = getopt(argc, argv, "t:i:p:")) != -1) {
        switch (opt) {
            case 't':
                linea->numero_threads = atoi(optarg);
                break;
            case 'i':
                linea->file_inizializzazione = optarg;
                break;
            case 'p':
                linea->file_porte = optarg;
                break;
            default:
                fprintf(stderr, "Uso: %s -t <numero_threads> -i <file_inizializzazione> -p <file_porte>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
}


// 2. PARSING DI UN NUMERO COMPLESSO: questa funzione legge un numero complesso da un file in input e sposta i puntatore in avanti fino al numero successivo
complesso parse_complex(char **p) {
    complesso c;
    c.reale = 0.0;
    c.immaginario = 0.0;

    // Salta spazi e l'eventuale parentesi tonda aperta '(' prima del numero reale
    while (isspace((unsigned char)**p) || **p == '(') {
        (*p)++;
    }
    
    c.reale = strtod(*p, p);
    
    while (isspace((unsigned char)**p)) (*p)++;
    if (**p != '+' && **p != '-') {
        // Prima di uscire, consuma l'eventuale parentesi chiusa ')' o virgola rimasta attaccata
        while (**p == ')' || **p == ',' || isspace((unsigned char)**p)) {
            (*p)++;
        }
        return c; 
    }

    char sign = **p;
    (*p)++;
    while (isspace((unsigned char)**p)) (*p)++;

    if (**p == 'i') {
        (*p)++;
    }
    while (isspace((unsigned char)**p)) (*p)++;
    double im = strtod(*p, p);

    if (sign == '-') {
        im = -im;
    }
    c.immaginario = im;
    while (**p == ')' || **p == ',' || isspace((unsigned char)**p)) {
        (*p)++;
    }

    return c;
}

// 3. PARSING DEL FILE DI INIZIALIZZAZIONE: in questo file dobbiamo trovare il numero di qubit e i valori iniziali del vettore
void parse_file_inizializzazione(char *filename, vector *vinit) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Errore nell'apertura del file di inizializzazione: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    int n_qubits = 0;
    char line[256];
    
    // Spostata la graffa di apertura per includere il ciclo di lettura
    while (fgets(line, sizeof(line), file)) {
        // se leggiamo #qubits significa che  possiamo leggere il numero di qubit e creare il vettore iniziale con la dimensione corretta
        if (strncmp(line, "#qubits", 7) == 0) {
            sscanf(line + 7, "%d", &n_qubits); // Corretto il nome della variabile n_qubit
            *vinit = crea_vector(n_qubits);   // Assumo che inizializzi vinit->dimensione = 2^n
        }
        // se leggiamo #init significa che possiamo leggere i valori iniziali del vettore e salvarli in vinit->componenti
        else if (strncmp(line, "#init", 5) == 0) {
            char *p = strchr(line, '[');
            if (!p) continue;

            p++; 

            for (int i = 0; i < vinit->dimensione; i++) {
                vinit->componenti[i] = parse_complex(&p);

                while (isspace((unsigned char)*p)) p++;

                if (*p == ',') p++;

                while (isspace((unsigned char)*p)) p++;
            }
        }
    }
    fclose(file); 
}


// Funzione di supporto: alloca e copia in profondità una matrice  ovvero non copio solo il puntatore 
static void copia_matrice_profonda(matrix *dst, matrix *src) {
    dst->dimensione = src->dimensione;
    strncpy(dst->nome, src->nome, sizeof(dst->nome) - 1);
    dst->nome[sizeof(dst->nome) - 1] = '\0';

    dst->elementi = malloc(dst->dimensione * sizeof(complesso*));
    for (int i = 0; i < dst->dimensione; i++) {
        dst->elementi[i] = malloc(dst->dimensione * sizeof(complesso));
        for (int j = 0; j < dst->dimensione; j++) {
            dst->elementi[i][j] = src->elementi[i][j];
        }
    }
}


// 4. PARSING DEL FILE DELLE PORTE E CIRCUITO: nel seguente file troviamo i gate su cui dobbiamo far passare i qubit e il circuito
void parse_file_porte(char *filename, matrix *porte, array_porte *arr_porte) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Errore nell'apertura del file delle porte: %s\n", filename);
        exit(EXIT_FAILURE);
    }   

    char line[512]; 
    
    while (fgets(line, sizeof(line), file)) {
        //   abbiamo trovato un  #define quindi possiamo leggere il nome della porta e i valori della matrice associata
        if (strncmp(line, "#define", 7) == 0) {
            int offset_nome =7;
            
            sscanf(line + offset_nome, "%63s", porte->nome);

            //cerco la parentesi quadra che indica l'inizio della matrice
            char *p = strchr(line, '[');
            while (!p) {
                if (!fgets(line, sizeof(line), file)) break;
                p = strchr(line, '[');
            }
            if (!p) continue;
            p++; 

            // Ciclo di estrazione degli elementi della matrice
            for (int i = 0; i < porte->dimensione; i++) {
                for (int j = 0; j < porte->dimensione; j++) {
                    
                    //controllo che il puntatore non punti  su caratteri di spaziatura o parentesi
                    while (*p == ' ' || *p == '\t' || *p == ',' || *p == ')' || *p == '(') {
                        p++;
                    }

                    //controllo che non siamo arrivati a fine riga in caso contraria  leggo quella successiva 
                    while (*p == '\0' || *p == '\n' || *p == '\r') {
                        if (!fgets(line, sizeof(line), file)) {
                            fprintf(stderr, "Errore: Fine del file inaspettata durante la lettura della matrice %s\n", porte->nome);
                            fclose(file);
                            exit(EXIT_FAILURE);
                        }
                        p = line; // Resetto il puntatore all'inizio della nuova riga
                        while (*p == ' ' || *p == '\t' || *p == ',' || *p == '(' || *p == ')') {
                            p++;
                        }
                    }

                    // faccio il parsing del numero complesso
                    porte->elementi[i][j] = parse_complex(&p); 
                    
                    //printf("Matrice %s, cella [%d][%d] letta come: RE=%.5f IM=%.5f\n", 
                           porte->nome, i, j, porte->elementi[i][j].reale, porte->elementi[i][j].immaginario;
                }
            }

            // Salvo la matrice letta nell'array cumulativo (DEEP COPY)
            // porte->elementi puntano alla memoria allocata per la matrice corrente, quindi
            // qui ne facciamo una copia completamente indipendente.
            matrix nuova_porta;
            copia_matrice_profonda(&nuova_porta, porte);

            arr_porte->porte[arr_porte->numero_porte] = nuova_porta;
            arr_porte->numero_porte++;
        }
        // se invece troviamo un #circ significa che dobbiamo costruire il circuito 
        else if (strncmp(line, "#circ", 5) == 0) {
            char *ptr = line + 5; 
            char nome_porta[64];
            int offset = 0;
            //ci prepariamo un array temporaneo per contenere le matrici del circuito espanso di lunghezza massima 100
            matrix *circuito_temporaneo = malloc(100 * sizeof(matrix)); 
            int conteggio_porte_circuito = 0;

            while (sscanf(ptr, "%63s%n", nome_porta, &offset) == 1) {
                ptr += offset; 
                //se abbiamo trovato measure  allora leggiamo il numero di ripetizioni
                if (strcmp(nome_porta, "measure") == 0) {
                    sscanf(ptr, "%d", &arr_porte->ha_misurazione); 
                    break; 
                }

                int trovata = 0;
                for (int j = 0; j < arr_porte->numero_porte; j++) {
                    if (strcmp(arr_porte->porte[j].nome, nome_porta) == 0) {
                        // se leggo il nome della porta allora copio la matrice corrispondente in circuito_temporaneo attraverso la funzione copia_matrice_profonda
                        copia_matrice_profonda(&circuito_temporaneo[conteggio_porte_circuito],
                                                &arr_porte->porte[j]);
                        conteggio_porte_circuito++;
                        trovata = 1;
                        break; 
                    }
                }

                if (!trovata) {
                    fprintf(stderr, "Errore: La matrice '%s' nel #circ non è stata definita prima!\n", nome_porta);
                    // Libera quanto già allocato in circuito_temporaneo prima di uscire
                    for (int k = 0; k < conteggio_porte_circuito; k++) {
                        for (int r = 0; r < circuito_temporaneo[k].dimensione; r++) {
                            free(circuito_temporaneo[k].elementi[r]);
                        }
                        free(circuito_temporaneo[k].elementi);
                    }
                    free(circuito_temporaneo);
                    fclose(file);
                    exit(EXIT_FAILURE);
                }
            }

            // Libero la memoria allocata, e sostituiamo array delle porte con il circuito ordinato
            for (int j = 0; j < arr_porte->numero_porte; j++) {
                for (int r = 0; r < arr_porte->porte[j].dimensione; r++) {
                    free(arr_porte->porte[j].elementi[r]);
                }
                free(arr_porte->porte[j].elementi);
            }
            free(arr_porte->porte); 

            arr_porte->porte = circuito_temporaneo;
            arr_porte->numero_porte = conteggio_porte_circuito;
        }
    }
    fclose(file);
}