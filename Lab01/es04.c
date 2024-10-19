#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SIZE 31
#define CMD_SIZE 12

// [Begin] Typedefs

typedef struct {
    char rideCode[MAX_SIZE], initPos[MAX_SIZE], finalPos[MAX_SIZE];
    int date, initTime, finalTime, delay;
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

route **readFromFile(int *len);
void printCmds(char **cmds);
void getStringInput(char *input);
comando_e leggiComando(char *output, char **cmds, char ***argArray, int *argNum);
int parseDates(char **argArray, int *fdate, int *sdate);

void execCommand(comando_e cmd, route **routes, route **dateOrd, route **codeOrd, route **initOrd, route **finalOrd, char **argArray, char *output, int len, int argNum);

int compareRides(route *fride, route *sride, int criteria);
void sortByCriteria(route **routes, int len, int value);

void printWithConversion(route *route, FILE *file);

void linearSearch(route **routes, char *name, FILE *file, int len, int nameLen);
void binarySearch(route **routes, char *name, FILE *file, int left, int right, int nameLen);

void createSortedCopy(route **routes, route ***nRoutes, int len, int criteria);

void freeVec(void **routes, int len);
// [End] Prototypes


int main() {
    int len, cmd, argNum;
    char *commands[] = {"date", "partenza", "capolinea", "ritardo", "ritardo_tot", "ordina_date", "ordina_codice", "ordina_partenza", "ordina_arrivo", "ricerca_lineare", "ricerca_dicotomica", "fine"};
    char **argArray, output[MAX_SIZE];

    route **routes = readFromFile(&len);
    route **dateOrd, **codeOrd, **initOrd, **finalOrd;

    createSortedCopy(routes, &dateOrd, len, r_ordina_date);
    createSortedCopy(routes, &codeOrd, len, r_ordina_codice);
    createSortedCopy(routes, &initOrd, len, r_ordina_partenza);
    createSortedCopy(routes, &finalOrd, len, r_ordina_arrivo);

    if (routes != NULL) {
        printf("Inserire un comando tra i seguenti: \n");
        printCmds(commands);

        do {
            cmd = leggiComando(output, commands, &argArray, &argNum);
            execCommand(cmd, routes, dateOrd, codeOrd, initOrd, finalOrd, argArray, output, len, argNum);
        } while (cmd != -1 && cmd != r_fine);
    } else {
        printf("Impossibile aprire il file richiesto");
        return -1;
    }

    freeVec((void **) routes, len);
    freeVec((void **) argArray, argNum);

    return 0;
}


route **readFromFile(int *len) {
    FILE *file;
    char fileName[MAX_SIZE];
    int i = 0, flag = 0, year, month, day, initHour, initMin, initSec, finalHour, finalMin, finalSec; 

    // Get the name of the file
    printf("Inserire il nome del file contenente le corse: ");
    getStringInput(fileName);

    file = fopen(fileName, "r");
    if (file == NULL) return NULL;

    // Get the number of lines stored in the file. The return is added to limit the maximum amount of lines (as requested by the exercise)
    fscanf(file, "%d", len);
    if ((*len) > 1000) return NULL;

    route **routes = malloc((*len) * sizeof(void *));

    for ( ; i < (*len); ++i) {
        route *cRoute = malloc(sizeof(route));
        if (fscanf(file, "%s %s %s %d/%d/%d %d:%d:%d %d:%d:%d %d", cRoute -> rideCode, cRoute -> initPos, cRoute -> finalPos, &year, &month, &day, &initHour, &initMin, &initSec, &finalHour, &finalMin, &finalSec, &cRoute -> delay) == 13) {
            cRoute -> date = year * 372 + (month - 1) * 31 + day - 1;
            cRoute -> initTime = initHour * 3600 + initMin * 60 + initSec;
            cRoute -> finalTime = finalHour * 3600 + finalMin * 60 + finalSec;
            routes[i] = cRoute;
        } else {
            i = *len;
        }
    }

    fclose(file);
    return routes;
}


comando_e leggiComando(char *output, char **cmds, char ***argArray, int *argNum) {
    char *token, *input = malloc(MAX_SIZE * sizeof(char));
    int len;
    *argArray = malloc(2 * sizeof(void *));
    comando_e result = -1;

    printf("Seleziona un comando: ");
    getStringInput(input);

    token = strtok(input, " ");

    for (int i = 0; i < CMD_SIZE; ++i) {
        if (!strcmp(token, cmds[i])) {
            result = (comando_e) i;
            break;
        }
    }

    for (*argNum = 0; token = strtok(NULL, " "); ++*argNum) {
        (*argArray)[(*argNum)] = token;
    }

    if (result != r_fine && result != -1) {
        printf("Inserire il nome del file di output (lasciare vuoto per stampare a schermo): ");
        getStringInput(output);
    }

    return result;
}


void execCommand(comando_e cmd, route **routes, route **dateOrd, route **codeOrd, route **initOrd, route **finalOrd, char **argArray, char *output, int len, int argNum) {
    int i = 0, delay = 0, fdate, sdate, nameLen;
    static int const argsNum[CMD_SIZE] = {2, 1, 1, 2, 1, 0, 0, 0, 0, 1, 1, 0};
    FILE *file = NULL;

    if (cmd != -1 && argNum != argsNum[cmd]) {
        printf("Il numero di argomenti richiesto per il comando è: %d. Gli argomenti forniti sono: %d. Input non valido\n", argsNum[cmd], argNum);
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
            for ( ; i < len; ++i) {
                if (routes[i] -> date >= fdate && routes[i] -> date <= sdate) {
                    printWithConversion(routes[i], file);
                }
            }
            break;
        
        case r_partenza:
            for ( ; i < len; ++i) {
                if (strcmp(routes[i] -> initPos, argArray[0]) == 0) {
                    printWithConversion(routes[i], file);
                }
            }
            break;
        
        case r_capolinea:
            for ( ; i < len; ++i) {
                if (strcmp(routes[i] -> finalPos, argArray[0]) == 0) {
                    printWithConversion(routes[i], file);
                }
            }
            break;

        case r_ritardo:
            for ( ; i < len; ++i) {
                if (routes[i] -> date >= fdate && routes[i] -> date <= sdate && routes[i] -> delay) {
                    printWithConversion(routes[i], file);
                }
            }
            break;

        case r_ritardo_tot:
            for ( ; i < len; ++i) {
                if (strcmp(routes[i] -> rideCode, argArray[0]) == 0 && routes[i] -> delay) {
                    delay += routes[i] -> delay;
                }
            }
            fprintf(file ? file : stdout, "Il ritardo complessivo delle corse identificate dal codice %s è stato di %d minuti \n", argArray[0], delay);
            break;

        case r_ordina_date :
            for ( ; i < len; ++i) {
                printWithConversion(dateOrd[i], file);
            }
            break;

        case r_ordina_codice: 
            for ( ; i < len; ++i) {
                printWithConversion(codeOrd[i], file);
            }
            break;

        case r_ordina_partenza:
            for ( ; i < len; ++i) {
                printWithConversion(initOrd[i], file);
            }
            break;
        
        case r_ordina_arrivo:
            for ( ; i < len; ++i) {
                printWithConversion(finalOrd[i], file);
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


int compareRides(route *fride, route *sride, int criteria) {
    switch (criteria) {
        case r_ordina_date:
            if (fride -> date != sride -> date)
                return fride -> date > sride -> date;
            return fride -> initTime > sride -> initTime;
        case r_ordina_codice:
            return strcmp(fride -> rideCode, sride -> rideCode) > 0;
        case r_ordina_partenza:
            return strcmp(fride -> initPos, sride-> initPos) > 0;
        case r_ordina_arrivo:
            return strcmp(fride -> finalPos, sride -> finalPos) > 0;
    }
    return 0;
}


void createSortedCopy(route **routes, route ***nRoutes, int len, int criteria) {
    *nRoutes = malloc(len * sizeof(void *));
    memcpy(*nRoutes, routes, len * sizeof(void *));
    sortByCriteria(*nRoutes, len, criteria);
}


void sortByCriteria(route **routes, int len, int value) {
    int i, j, r = len - 1, flag = 1;
    route *temp;
    for (i = 0; i < r && flag; ++i) {
        flag = 0;
        for (j = 0; j < r - i; ++j) {
            if (compareRides(routes[j], routes[j+1], value)) {
                flag = 1;
                temp = routes[j];
                routes[j] = routes[j+1];
                routes[j+1] = temp;
            }
        }
    }
}


void linearSearch(route **routes, char *name, FILE *file, int len, int nameLen) {
    for (int i = 0; i < len; ++i) {
        if (strncmp(routes[i] -> initPos, name, nameLen) == 0) {
            printWithConversion(routes[i], file);
        } 
    }
}


void binarySearch(route **routes, char *name, FILE *file, int left, int right, int nameLen) {
    int mid, cmp, i;
    
    // Find the first occurrence of the partial
    while (left <= right) {
        mid = left + (right - left) / 2;
        cmp = strncmp(routes[mid] -> initPos, name, nameLen);
        
        if (cmp == 0) {
            printWithConversion(routes[mid], file);
            break;
        } else if (cmp < 0) {
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


void getStringInput(char *input) {
    fgets(input, MAX_SIZE, stdin);
    int strLen = strlen(input);

    if (input[strLen - 1] == '\n')
        input[strLen - 1] = '\0';
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
        route -> rideCode,
        route -> initPos,
        route -> finalPos,
        
        (route -> date)/372,
        (((route -> date) % 372) / 31) + 1,
        ((route -> date) % 31) + 1,

        (route -> initTime)/3600,
        ((route -> initTime) % 3600)/60,
        (route -> initTime) % 60,
        
        (route -> finalTime)/3600,
        ((route -> finalTime) % 3600)/60,
        (route -> finalTime) % 60,
        
        route -> delay
    );
}

void freeVec(void **vec, int len) {
    for (int i = 0; i < len; ++i) {
        free(vec[i]);
    }
    free(vec);
}
