/*
 * Implementare in ANSI C un programma per la gestione di una catena alberghiera
 * e delle relative prenotazioni.
 *
 * Gli Hotel sono memorizzati in un VETTORE DINAMICO (Modulo 1).
 * Ogni Hotel contiene una CODA di prenotazioni (Modulo 2).
 *
 * Il programma esegue le seguenti operazioni:

 * 1. Caricare i dati degli hotel da "hotel.txt" in un vettore allocato dinamicamente.
 * 2. Analizzare il vettore e trovare l'hotel con il maggior numero di camere libere.

 * 3. Caricare le prenotazioni da "prenotazioni.txt" e inserirle nella coda del
 *    relativo hotel.
 * 4. Generare e popolare una Tabella Hash contenente TUTTE le prenotazioni del sistema.
 * 5. Ricercare una prenotazione specifica nella Tabella Hash tramite il suo ID.

 * 6. Salvare su un file di testo "output.txt" tutte le prenotazioni di uno specifico hotel,
 *    cercandolo nel vettore e scandendo la sua coda.
 ******************************************************************************/

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define FILE_HOTEL "hotel.txt"
#define FILE_PRENOTAZIONI "prenotazioni.txt"
#define FILE_OUTPUT "output.txt"
#define DIM_HASH 101 // Numero primo per la Tabella Hash
#define STR_LEN 30

// Codici di ritorno
#define OK (0)
#define ERRORE_FILE (-1)
#define ERRORE_MEMORIA (-2)
#define ERRORE_INPUT (-3)

// --- STRUTTURE DATI ---

typedef struct {
    char id_prenotazione[STR_LEN]; // Es: RES1234
    char cognome_cliente[STR_LEN];
    int giorni_soggiorno;
    char codice_hotel[STR_LEN];    // Codice dell'hotel di destinazione
} Prenotazione;

// Nodo per la coda delle prenotazioni (per l'hotel)
typedef struct nodo_pren {
    Prenotazione info;
    struct nodo_pren* next;
} NodoPrenotazione;

// Nodo separato per la Tabella Hash (per evitare conflitti coi puntatori next)
typedef struct nodo_hash {
    Prenotazione info;
    struct nodo_hash* next;
} NodoHash;

typedef struct {
    char codice_hotel[STR_LEN];    // Es: HTL_RM
    char citta[STR_LEN];
    int camere_totali;
    int camere_occupate;
    NodoPrenotazione* primo;  // Testa della coda prenotazioni
    NodoPrenotazione* ultimo; // Coda della coda prenotazioni
} Hotel;

typedef struct {
    NodoHash* array[DIM_HASH];
} TabellaHash;


// --- FUNZIONI HELPER ---

/* Inizializza i campi coda di un Hotel a NULL */
void inizializzaHotel(Hotel* h) {
    h->primo = NULL;
    h->ultimo = NULL;
    h->camere_occupate = 0;
}

/* Cerca un Hotel per codice nel vettore di hotel */
Hotel* cercaHotel(Hotel* array_hotel, unsigned num_hotel, const char* codice) {
    if (array_hotel == NULL) return NULL;
    for (unsigned i = 0; i < num_hotel; i++) {
        if (strcmp(array_hotel[i].codice_hotel, codice) == 0) {
            return &array_hotel[i];
        }
    }
    return NULL;
}

/* Funzione Hash per stringhe (djb2) */
unsigned int funzioneHash(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % DIM_HASH;
}

/* Inizializza la Tabella Hash */
void inizializzaTabellaHash(TabellaHash *th) {
    for (int i = 0; i < DIM_HASH; i++) {
        th->array[i] = NULL;
    }
}

/* Funzioni di stampa */
void stampaPrenotazione(Prenotazione p) {
    printf("  [ID: %s] Cliente: %s - Giorni: %d -> Hotel: %s\n",
           p.id_prenotazione, p.cognome_cliente, p.giorni_soggiorno, p.codice_hotel);
}

void stampaHotel(Hotel* array_hotel, unsigned num_hotel) {
    for(unsigned i = 0; i < num_hotel; i++) {
        printf("HOTEL %s a %s (Camere: %d occupate su %d)\n",
            array_hotel[i].codice_hotel, array_hotel[i].citta,
            array_hotel[i].camere_occupate, array_hotel[i].camere_totali);

        NodoPrenotazione* corr = array_hotel[i].primo;
        while(corr != NULL) {
            stampaPrenotazione(corr->info);
            corr = corr->next;
        }
    }
}



/*
 * Carica gli hotel dal file "hotel.txt" in un vettore allocato dinamicamente.
 * Formato file:
 * - 1 riga con il numero N di hotel
 * - N righe con i dati: CodiceHotel Citta CamereTotali
 */
Hotel* caricaHotel(const char* nome_file, unsigned* num_hotel) {
    Hotel* array_hotel = NULL;
    *num_hotel = 0;

    FILE* fp= fopen(nome_file,"r");
    if (fp==NULL) return NULL;
    if (fscanf(fp,"%u",num_hotel)!=1) {
        fclose(fp);
        return NULL;
    }

    if (*num_hotel==0) {
        fclose(fp);
        return NULL;
    }

    array_hotel=malloc((*num_hotel)*sizeof(Hotel));
    if (array_hotel==NULL) {
        fclose(fp);
        return NULL;
    }

    for (int i = 0;i<*num_hotel;i++) {
        fscanf(fp,"%s%s%d",
            array_hotel[i].codice_hotel,
            array_hotel[i].citta,
            &array_hotel[i].camere_totali);
        inizializzaHotel(&array_hotel[i]);
    }

    fclose(fp);
    return array_hotel;
}

/*
 * Analizza il vettore degli hotel e restituisce un puntatore all'hotel
 * che ha il maggior numero di camere libere (camere_totali - camere_occupate).
 * Se il vettore è vuoto, restituisce NULL.
 */
Hotel* trovaHotelPiuLibero(Hotel* array_hotel, unsigned num_hotel) {
    if (array_hotel == NULL || num_hotel == 0) return NULL;

    int camer_libere = array_hotel[0].camere_totali-array_hotel[0].camere_occupate;
    Hotel *maxHotel = &array_hotel[0];

    for (int i =1;i<num_hotel;i++) {
        int cam_corr = array_hotel[i].camere_totali-array_hotel[i].camere_occupate;
        if (cam_corr>camer_libere) {
            camer_libere = cam_corr;
            maxHotel = &array_hotel[i];
        }
    }

    return maxHotel;
}

/*
 * Legge le prenotazioni dal file "prenotazioni.txt" e le accoda all'hotel corretto.
 * Formato file (Nessun contatore iniziale, leggere fino a EOF):
 * IdPrenotazione CognomeCliente GiorniSoggiorno CodiceHotel
 */
int caricaEAccodaPrenotazioni(const char* nome_file, Hotel* array_hotel, unsigned num_hotel) {
    FILE* fp=fopen(nome_file,"r");
    if (fp==NULL) return ERRORE_FILE;
    Prenotazione p;

    while (fscanf(fp,"%s%s%d%s",
        p.id_prenotazione,
        p.cognome_cliente,
        &p.giorni_soggiorno,
        p.codice_hotel)==4) {

        Hotel *hotel_corr = cercaHotel(array_hotel,num_hotel,p.codice_hotel);
        if (hotel_corr!=NULL) {
            NodoPrenotazione *nuovo = malloc(sizeof(NodoPrenotazione));
            if (nuovo==NULL) {
                fclose(fp);
                return ERRORE_MEMORIA;
            }

            nuovo->info=p;
            nuovo->next=NULL;

            if (hotel_corr->primo==NULL){
                hotel_corr->primo=nuovo;
            }
            else {
                hotel_corr->ultimo->next=nuovo;
            }
            hotel_corr->ultimo=nuovo;
            hotel_corr->camere_occupate++;
        }
    }
    fclose(fp);
    return OK;
}

/*
 * Popola la Tabella Hash inserendovi TUTTE le prenotazioni presenti nelle code
 * di tutti gli hotel del vettore.
 * Chiave di Hash: id_prenotazione.
 * Gestione collisioni: Inserimento in TESTA alla lista concatenata presente nel bucket.

 */
int creaTabellaHash(TabellaHash* th, Hotel* array_hotel, unsigned num_hotel) {
    if (th == NULL || array_hotel == NULL || num_hotel == 0) return ERRORE_INPUT;

    for (int i = 0;i<num_hotel;i++) {
        NodoPrenotazione *nodo_corr=array_hotel[i].primo;
        while (nodo_corr!=NULL) {
            NodoHash *nuovo = malloc(sizeof(NodoHash));
            if (nuovo==NULL) return ERRORE_MEMORIA;
            int IDX = funzioneHash(nodo_corr->info.id_prenotazione);
            nuovo->info=nodo_corr->info;
            nuovo->next = th->array[IDX];
            th->array[IDX] = nuovo;

            nodo_corr=nodo_corr->next;
        }
    }


    return OK;
}

/*
 * Data la chiave 'id_prenotazione', cerca la prenotazione corrispondente
 * all'interno della Tabella Hash e ne restituisce il puntatore.
 * Se non la trova, restituisce NULL.
 */
Prenotazione* cercaInHash(TabellaHash* th, const char* id_prenotazione) {
    if (th == NULL || id_prenotazione == NULL) return NULL;

    int IDX = funzioneHash(id_prenotazione);
    NodoHash *nodo_corr = th->array[IDX];
    while (nodo_corr!=NULL) {
        if (strcmp(nodo_corr->info.id_prenotazione,id_prenotazione)==0) {
            return &nodo_corr->info;
        }
        nodo_corr=nodo_corr->next;
    }


    return NULL;
}

/*
 * Dato il 'codice_hotel', cerca l'hotel nel vettore (tramite cercaHotel).
 * Se l'hotel esiste, scansiona tutta la sua coda di prenotazioni e salva
 * i dettagli su "output.txt" con il formato:
 * "ID: [id_prenotazione] - Cliente: [cognome] - Soggiorno: [giorni] notti"
 */
int salvaPrenotazioniHotel(Hotel* array_hotel, unsigned num_hotel, const char* codice_hotel, const char* nome_file_out) {
    if (array_hotel == NULL) return ERRORE_INPUT;

    Hotel *hotel_corr=cercaHotel(array_hotel,num_hotel,codice_hotel);
    if (hotel_corr==NULL) return ERRORE_INPUT;

    FILE* fp=fopen(nome_file_out,"w");
    if (fp==NULL) return ERRORE_FILE;

    NodoPrenotazione *nodo_corr=hotel_corr->primo;
    while (nodo_corr!=NULL) {
        fprintf(fp,"ID: [%s] - Cliente: [%s] - Soggiorno: [%d] notti\n",
            nodo_corr->info.id_prenotazione,
            nodo_corr->info.cognome_cliente,
            nodo_corr->info.giorni_soggiorno);
        nodo_corr=nodo_corr->next;
    }

    fclose(fp);
    return OK;
}


// --- MAIN --

int main() {
    Hotel* array_hotel = NULL;
    unsigned num_hotel = 0;
    TabellaHash tabHash;

    inizializzaTabellaHash(&tabHash);

    printf("====== INIZIO PROGRAMMA ======\n");

    // TEST
    array_hotel = caricaHotel(FILE_HOTEL, &num_hotel);
    printf("\nCaricati %u hotel.\n", num_hotel);

    // TEST
    caricaEAccodaPrenotazioni(FILE_PRENOTAZIONI, array_hotel, num_hotel);
    stampaHotel(array_hotel, num_hotel);

    // TEST
    Hotel* h_libero = trovaHotelPiuLibero(array_hotel, num_hotel);
    if(h_libero) {
        int libere = h_libero->camere_totali - h_libero->camere_occupate;
        printf("\nHotel con piu' camere libere: %s (%d libere)\n", h_libero->codice_hotel, libere);
    }

    // TEST
    creaTabellaHash(&tabHash, array_hotel, num_hotel);
    printf("\nTabella Hash generata con successo.\n");

    // TEST
    Prenotazione* trovata = cercaInHash(&tabHash, "RES999");
    if(trovata) {
    printf("\nRicerca Hash: Trovata -> ");
    stampaPrenotazione(*trovata);
    } else {
    printf("\nRicerca Hash: Prenotazione RES999 non trovata.\n");
    }

    // TEST
    salvaPrenotazioniHotel(array_hotel, num_hotel, "HTL_MI", FILE_OUTPUT);
    printf("\nSalvataggio su output.txt per HTL_MI completato.\n");

    printf("\n====== FINE. ======\n");

    return 0;
}