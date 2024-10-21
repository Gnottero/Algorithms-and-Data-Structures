#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SIZE 31
#define CMD_SIZE 12

// [Begin] Typedefs

typedef struct {
    char code[MAX_SIZE], initPos[MAX_SIZE], finPos[MAX_SIZE];
    int date, initTime, finTime, delay;
} route;

typedef enum {

    // Informazioni
    r_date,
    r_partenza,
    r_capolinea,
    r_ritardo,
    r_ritardo_tot,

    // Ordinamento
    r_ordina_date,
    r_ordina_codice,
    r_ordina_partenza,
    r_ordina_arrivo,

    // Ricerca
    r_ricerca_lineare,
    r_ricerca_dicotomica,

    r_fine
} comando_e;

// [End] Typedefs

// [Begin] Prototypes

comando_e leggiComando(char **cmds, char ***argArray, char **input, char *output, int *argCount);

int parseDates(char **argArray, int *fdate, int *sdate);
int compareRides(const route *fride, const route *sride, int criteria);

void getStringInput(char *input);
void readFromFile(int *len, route ***routes);
void printCmds(char **cmds);
void execCommand(comando_e cmd, route **routes, char **argArray, const char *output, int len, int argCount);
void sortByCriteria(route **routes, int len, int value);
void printWithConversion(route *route, FILE *file);
void linearSearch(route **routes, const char *name, FILE *file, int len, int nameLen);
void binarySearch(route **routes, const char *name, FILE *file, int left, int right, int nameLen);
void freePtrArray(void **array, int len);

// [End] Prototypes

int main() {
    int len, argCount;
    char *input, output[MAX_SIZE];
    char *cmds[] = {"date", "partenza", "capolinea", "ritardo", "ritardo_tot", "ordina_date", "ordina_codice", "ordina_partenza", "ordina_arrivo", "ricerca_lineare", "ricerca_dicotomica", "fine"};
    char **argArray;
    route **routes;

    readFromFile(&len, &routes);

    if (routes != NULL) {
        int cmd;
        printf("Inserire un comando tra i seguenti: \n");
        printCmds(cmds);

        do {
            cmd = leggiComando(cmds, &argArray, &input, output, &argCount);
            execCommand(cmd, routes, argArray, output, len, argCount);
        } while (cmd != -1 && cmd != r_fine);
    } else {
        return -1;
    }

    freePtrArray((void **) routes, len);
    freePtrArray((void **) argArray, argCount);
    free(input);

    return 0;
}


void readFromFile(int *len, route ***routes) {
    char fpName[MAX_SIZE];
    int year, month, day, initHour, initMin, initSec, finHour, finMin, finSec;

    printf("Inserire il nome del file contenente le corse: ");
    getStringInput(fpName);

    FILE *fp = fopen(fpName, "r");
    if (fp == NULL) return;

    // Get the number of lines in the file. If greater than 1000 discard it
    fscanf(fp, "%d", len);

    if ((*len) > 1000) return;

    *routes = malloc((*len) * sizeof(void *));

    for (int i = 0; i < *len; ++i) {
        // Riprova a leggere solo se fscanf restituisce il numero corretto di campi
        route *temp = malloc(sizeof(route));
        if (fscanf(fp, "%s %s %s %d/%d/%d %d:%d:%d %d:%d:%d %d",
                   temp->code, temp->initPos, temp->finPos,
                   &year, &month, &day,
                   &initHour, &initMin, &initSec,
                   &finHour, &finMin, &finSec, &temp->delay) == 13) {
            temp->date = year * 372 + (month - 1) * 31 + (day - 1);
            temp->initTime = initHour * 3600 + initMin * 60 + initSec;
            temp->finTime = finHour * 3600 + finMin * 60 + finSec;
            (*routes)[i] = temp;
        } else {
            free(temp);
            break;
        }
    }

    fclose(fp);
}


comando_e leggiComando(char **cmds, char ***argArray, char **input, char *output, int *argCount) {
    *input = malloc(MAX_SIZE * sizeof(char));
    *argArray = malloc(2 * sizeof(void *));
    comando_e result = -1;

    printf("Seleziona un comando: ");
    getStringInput(*input);

    char *token = strtok(*input, " ");

    for (int i = 0; i < CMD_SIZE; ++i) {
        if (!strcmp(token, cmds[i])) {
            result = (comando_e) i;
            break;
        }
    }

    for (*argCount = 0; (token = strtok(NULL, " ")); ++*argCount) {
        (*argArray)[(*argCount)] = token;
    }

    if (result != r_fine && result != -1) {
        printf("Inserire il nome del file di output (lasciare vuoto per stampare a schermo): ");
        getStringInput(output);
    }

    return result;
}


void execCommand(const comando_e cmd, route **routes, char **argArray, const char *output, const int len, const int argCount) {
    int i, delay = 0, fdate, sdate, nameLen;
    static int const argsNum[CMD_SIZE] = {2, 1, 1, 2, 1, 0, 0, 0, 0, 1, 1, 0};
    FILE *file = NULL;

    if (cmd != -1 && argCount != argsNum[cmd]) {
        printf("Il numero di argomenti richiesto per il comando è: %d. Gli argomenti forniti sono: %d. Input non valido\n", argsNum[cmd], argCount);
        return;
    }

    if (output[0] != '\0' && cmd != r_fine) {
        file = fopen(output, "w");
        if (file == NULL) {
            printf("Errore nella creazione del file di log \n");
            return;
        }
    }

    if (cmd == r_date || cmd == r_ritardo) {
        if (!parseDates(argArray, &fdate, &sdate)) {
            printf("Errore nel parsing delle date \n");
            if (file != NULL) {
                fclose(file);
            }
            return;
        }
    }

    if (cmd == r_ricerca_lineare || cmd == r_ricerca_dicotomica) {
        nameLen = strlen(argArray[0]);
    }

    switch (cmd) {
        case r_date:
            for (i = 0; i < len; ++i) {
                if (routes[i] -> date >= fdate && routes[i] -> date <= sdate) {
                    printWithConversion(routes[i], file);
                }
            }
            break;

        case r_partenza:
            for (i = 0; i < len; ++i) {
                if (strcmp(routes[i] -> initPos, argArray[0]) == 0) {
                    printWithConversion(routes[i], file);
                }
            }
            break;

        case r_capolinea:
            for (i = 0; i < len; ++i) {
                if (strcmp(routes[i] -> finPos, argArray[0]) == 0) {
                    printWithConversion(routes[i], file);
                }
            }
            break;

        case r_ritardo:
            for (i = 0; i < len; ++i) {
                if (routes[i] -> date >= fdate && routes[i] -> date <= sdate && routes[i] -> delay) {
                    printWithConversion(routes[i], file);
                }
            }
            break;

        case r_ritardo_tot:
            for (i = 0; i < len; ++i) {
                if (strcmp(routes[i] -> code, argArray[0]) == 0 && routes[i] -> delay) {
                    delay += routes[i] -> delay;
                }
            }
            fprintf(file ? file : stdout, "Il ritardo complessivo delle corse identificate dal codice %s è stato di %d minuti \n", argArray[0], delay);
            break;

        case r_ordina_date : case r_ordina_codice : case r_ordina_partenza : case r_ordina_arrivo:
            sortByCriteria(routes, len, cmd);
            for (i = 0; i < len; ++i) {
                printWithConversion(routes[i], file);
            }
            break;

        case r_ricerca_lineare:
            linearSearch(routes, argArray[0], file, len, nameLen);
            break;

        case r_ricerca_dicotomica:
            sortByCriteria(routes, len, r_ordina_partenza);
            binarySearch(routes, argArray[0], file, 0, len - 1, nameLen);

        default:
            break;
    }

    if (file)
        fclose(file);
}


int compareRides(const route *fride, const route *sride, const int criteria) {
    switch (criteria) {
        case r_ordina_date:
            if (fride -> date != sride -> date)
                return fride -> date > sride -> date;
            return fride -> initTime > sride -> initTime;
        case r_ordina_codice:
            return strcmp(fride -> code, sride -> code) > 0;
        case r_ordina_partenza:
            return strcmp(fride -> initPos, sride-> initPos) > 0;
        case r_ordina_arrivo:
            return strcmp(fride -> finPos, sride -> finPos) > 0;
        default: ;
    }
    return 0;
}


void sortByCriteria(route **routes, int len, int value) {
    int flag = 1;
    const int r = len - 1;
    for (int i = 0; i < r && flag; ++i) {
        flag = 0;
        for (int j = 0; j < r - i; ++j) {
            if (compareRides(routes[j], routes[j+1], value)) {
                flag = 1;
                route *temp = routes[j];
                routes[j] = routes[j+1];
                routes[j+1] = temp;
            }
        }
    }
}


void linearSearch(route **routes, const char *name, FILE *file, const int len, const int nameLen) {
    for (int i = 0; i < len; ++i) {
        if (strncmp(routes[i] -> initPos, name, nameLen) == 0) {
            printWithConversion(routes[i], file);
        }
    }
}


void binarySearch(route **routes, const char *name, FILE *file, int left, int right, int nameLen) {
    int mid = 0, i;

    // Find the first occurrence of the partial
    while (left <= right) {
        mid = left + (right - left) / 2;
        const int cmp = strncmp(routes[mid]->initPos, name, nameLen);

        if (cmp == 0) {
            printWithConversion(routes[mid], file);
            break;
        }
        if (cmp < 0) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    // Since the first occurrence might not be the first in the array, expand to the left and to the right until the value of the strcmp is not zero
    for (i = mid - 1; i >= 0; --i) {
        if (strncmp(routes[i] -> initPos, name, nameLen) == 0) {
            printWithConversion(routes[i], file);
        }
    }

    for (i = mid + 1; i <= right; ++i) {
        if (strncmp(routes[i] -> initPos, name, nameLen) == 0) {
            printWithConversion(routes[i], file);
        }
    }
}


int parseDates(char **argArray, int *fdate, int *sdate) {
    int fyear, fmonth, fday, syear, smonth, sday;

    if (sscanf(argArray[0], "%d/%d/%d", &fyear, &fmonth, &fday) != 3) {
        return 0;
    }
    if (sscanf(argArray[1], "%d/%d/%d", &syear, &smonth, &sday) != 3) {
        return 0;
    }

    *fdate = fyear * 372 + (fmonth - 1) * 31 + fday - 1;
    *sdate = syear * 372 + (smonth - 1) * 31 + sday - 1;
    return 1;
}


void printCmds(char **cmds) {
    for (int i = 0; i < CMD_SIZE; ++i) {
        printf("%s\n", cmds[i]);
    }
}


void printWithConversion(route *route, FILE *file) {
    fprintf(file ? file : stdout, "%s %s %s %04d/%02d/%02d %02d:%02d:%02d %02d:%02d:%02d %d \n",
        route -> code,
        route -> initPos,
        route -> finPos,

        (route -> date)/372,
        (((route -> date) % 372) / 31) + 1,
        ((route -> date) % 31) + 1,

        (route -> initTime)/3600,
        ((route -> initTime) % 3600)/60,
        (route -> initTime) % 60,

        (route -> finTime)/3600,
        ((route -> finTime) % 3600)/60,
        (route -> finTime) % 60,

        route -> delay
    );
}


void freePtrArray(void **array, int len) {
    for (int i = 0; i < len; ++i) {
        free(array[i]);
    }
    free(array);
}


void getStringInput(char *input) {
    fgets(input, MAX_SIZE, stdin);
    int strLen = strlen(input);

    if (input[strLen - 1] == '\n')
        input[strLen - 1] = '\0';
}