#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SIZE 31
#define CMD_SIZE 12


typedef struct {
    char rideCode[MAX_SIZE], depPos[MAX_SIZE], destPos[MAX_SIZE];
    int date, depTime, arrTime, delay; 
} ride;


typedef enum {
    r_date,
    r_partenza,
    r_capolinea,
    r_ritardo,
    r_ritardo_tot,
    r_ordina_date,
    r_ordina_codice,
    r_ordina_partenza,
    r_ordina_arrivo,
    r_ricerca_lineare,
    r_ricerca_dicotomica,
    r_fine
} comando_e;


ride **readDataFromFile(int *len);

comando_e leggiComando(char commands[][MAX_SIZE], char ***strArray, int *argNum, char *outputPath);
void printCommands(char commands[][MAX_SIZE]);

void execCommand(comando_e cmd, ride **rides, char **strArray, int len, int argNum, char *outputPath);
void printWithConversion(ride *ride, FILE *file);
void sortByDate(ride **rides, int len);
void sortByCode(ride **rides, int len);
void sortByDep(ride **rides, int len);
void sortByDest(ride **rides, int len);
int *linearSearch(ride **rides, int len, char *name);
int *binarySearch(ride **rides, int len, char *name);

void getTrimmedLine(char *line);


int main() {
    int len, cmd, argNum;
    char commands[CMD_SIZE][MAX_SIZE] = {"date", "partenza", "capolinea", "ritardo", "ritardo_tot", "ordina_date", "ordina_codice", "ordina_partenza", "ordina_arrivo", "ricerca_lineare", "ricerca_dicotomica", "fine"}; 
    char **strArray, outputPath[MAX_SIZE];
    ride **orderedDate, **orderedCode, **orderedDep, **orderedDest;

    orderedDate = malloc(len * sizeof(void *));
    orderedCode = malloc(len * sizeof(void *));
    orderedDep = malloc(len * sizeof(void *));
    orderedDest = malloc(len * sizeof(void *));
    

    printf("Inserire il nome del file contente le corse: ");
    ride **rides = readDataFromFile(&len);

    if (rides != NULL) {
        printf("Inserire un comando tra i seguenti: \n");
        printCommands(commands);

        memcpy(orderedDate, rides, len * sizeof(void *));
        memcpy(orderedCode, rides, len * sizeof(void *));
        memcpy(orderedDep, rides, len * sizeof(void *));
        memcpy(orderedDest, rides, len * sizeof(void *));

        sortByDate(orderedDate, len);
        sortByCode(orderedCode, len);
        sortByDep(orderedDep, len);
        sortByDest(orderedDest, len);
    
        do {
            cmd = leggiComando(commands, &strArray, &argNum, outputPath);
            switch (cmd) {
                case r_ordina_date:
                    execCommand(cmd, orderedDate, strArray, len, argNum, outputPath);
                case r_ordina_codice:
                    execCommand(cmd, orderedCode, strArray, len, argNum, outputPath);
                case r_ordina_partenza:
                    execCommand(cmd, orderedDep, strArray, len, argNum, outputPath);
                case r_ordina_arrivo:
                    execCommand(cmd, orderedDest, strArray, len, argNum, outputPath);
                default:
                    execCommand(cmd, rides, strArray, len, argNum, outputPath);

            }
        } while (cmd != -1 && cmd != r_fine);

    } else {
        printf("Impossibile aprire il file indicato");
    }

    return 0;
}


void printCommands(char commands[][MAX_SIZE]) {
    for (int i = 0; i < CMD_SIZE; ++i) {
        printf("%s \n", commands[i]);
    }
}


comando_e leggiComando(char commands[][MAX_SIZE], char ***strArray, int *argNum, char *outputPath) {
    char *token, *input = malloc(MAX_SIZE * sizeof(char));
    int strLen;
    *strArray = malloc(2 * sizeof(void *));
    comando_e result = -1;

    printf("Seleziona un comando: ");
    getTrimmedLine(input);

    token = strtok(input, " ");

    for (int i = 0; i < CMD_SIZE; ++i) {
        if (!strcmp(token, commands[i])) {
            result = (comando_e) i;
            break;
        }
    }

    for (*argNum = 0; ; ++*argNum) {
        token = strtok(NULL, " ");
        if (token) {
            (*strArray)[(*argNum)] = token;
        } else
            break;
    }

    if (result != r_fine) {
        printf("Inserire nome del file di output (lasciare vuoto per stampare a video): ");
        getTrimmedLine(outputPath);
    }

    return result;
}


void execCommand(comando_e cmd, ride **rides, char **strArray, int len, int argNum, char *outputPath) {
    int fyear, fmonth, fday, syear, smonth, sday, i = 0, delay = 0, fdate, sdate, *idx;
    static int const cmdArgsNum[CMD_SIZE] = {2, 1, 1, 2, 1, 0, 0, 0, 0, 1, 1, 0};
    FILE *file = NULL;

    if (cmd == -1) {
        printf("Il comando inserito non è un comando valido\n");
        return;
    }
    
    if (argNum != cmdArgsNum[cmd]) {
        printf("Il numero di argomenti richiesto per il comando è: %d. Gli argomenti forniti sono: %d. Input non valido\n", cmdArgsNum[cmd], argNum);
        return;
    }

    if (outputPath[0] != '\0') {
        file = fopen(outputPath, "w");
        if (file == NULL) {
            printf("Errore nella creazione del file di log \n");
            return;
        }
    }


    if (cmd == r_date || cmd == r_ritardo) {
        sscanf(strArray[0], "%d/%d/%d", &fyear, &fmonth, &fday);
        sscanf(strArray[1], "%d/%d/%d", &syear, &smonth, &sday);
        fdate = fyear * 372 + (fmonth - 1) * 31 + fday - 1;
        sdate = syear * 372 + (smonth - 1) * 31 + sday - 1;
    }

    switch (cmd) {
        case r_date:
            for ( ; i < len; ++i) {
                if (rides[i]->date >= fdate && rides[i]->date <= sdate)
                    printWithConversion(rides[i], file);
            }
            break;
        case r_partenza:
            for ( ; i < len; ++i) {
                if (strcmp(rides[i]->depPos, strArray[0]) == 0)
                    printWithConversion(rides[i], file);
            }
            break;
        case r_capolinea:
            for ( ; i < len; ++i) {
                if (strcmp(rides[i]->destPos, strArray[0]) == 0)
                    printWithConversion(rides[i], file);
            }
            break;
        case r_ritardo:
            for ( ; i < len; ++i) {
                if (rides[i]->date >= fdate && rides[i]->date <= sdate && rides[i]->delay > 0)
                    printWithConversion(rides[i], file);
            }
            break;
        case r_ritardo_tot:
            for ( ; i < len; ++i) {
                if (strcmp(rides[i]->rideCode, strArray[0]) == 0 && rides[i]->delay)
                    delay += rides[i]->delay;
            }
            fprintf(file ? file : stdout, "Il ritardo complessivo delle corse identificate dal codice %s è stato di %d minuti\n", strArray[0], delay); 
            break;
        case r_ordina_date : case r_ordina_codice : case r_ordina_partenza : case r_ordina_arrivo:
            sortByDate(rides, len);
            for ( ; i < len; ++i) {
                printWithConversion(rides[i], file);
            }
            break;
        case r_ricerca_lineare:
            idx = linearSearch(rides, len, strArray[0]);
            for ( ; idx[i] != -1; ++i) {
                printWithConversion(rides[idx[i]], file);
            }
            break;
        case r_ricerca_dicotomica:
            idx = binarySearch(rides, len, strArray[0]);
            for ( ; idx[i] != -1; ++i) {
                printWithConversion(rides[idx[i]], file);
            }
            break;
    }

    if (file)
        fclose(file);
}


ride **readDataFromFile(int *len) {
    FILE *file;
    char fileName[MAX_SIZE];
    int i = 0, flag = 0, year, month, day, depHour, depMin, depSec, arrHour, arrMin, arrSec;

    scanf("%s", fileName);
    getchar();

    file = fopen(fileName, "r");

    if (file == NULL) return NULL;

    fscanf(file, "%d", len);

    if (*len > 1000) return NULL;

    ride **rides = malloc((*len) * sizeof(void *));

    while (!flag) {
        ride *currentRide = malloc(sizeof(ride));
        if (fscanf(file, "%s %s %s %d/%d/%d %d:%d:%d %d:%d:%d %d", currentRide->rideCode, currentRide->depPos, currentRide->destPos, &year, &month, &day, &depHour, &depMin, &depSec, &arrHour, &arrMin, &arrSec, &currentRide->delay) == 13) {
            currentRide->date = year * 372 + (month - 1) * 31 + day - 1;
            currentRide->depTime = depHour * 3600 + depMin * 60 + depSec;
            currentRide->arrTime = arrHour * 3600 + arrMin * 60 + arrSec;
            rides[i++] = currentRide;
        } else {
            flag = 1;
        }
    }

    fclose(file);
    return rides;
}


void getTrimmedLine(char *line) {
    fgets(line, MAX_SIZE, stdin);
    int strLen = strlen(line);

    if (line[strLen - 1] == '\n')
        line[strLen - 1] = '\0';
}


void sortByDate(ride **rides, int len) {
    int i, j, r = len - 1, flag = 1;
    ride *temp;

    for (i = 0; i < r && flag; ++i) {
        flag = 0;
        for (j = 0; j < r - i; ++j) {
            if (rides[j]->date > rides[j+1]->date || (rides[j]->date == rides[j+1]->date && rides[j]->depTime > rides[j+1]->depTime)) {
                flag = 1;
                temp = rides[j];
                rides[j] = rides[j+1];
                rides[j+1] = temp; 
            }
        }
    }
}


void sortByCode(ride **rides, int len) {
    int i, j, r = len - 1, flag = 1;
    ride *temp;

    for (i = 0; i < r && flag; ++i) {
        flag = 0;
        for (j = 0; j < r - i; ++j) {
            if (strcmp(rides[j]->rideCode, rides[j+1]->rideCode) > 0) {
                flag = 1;
                temp = rides[j];
                rides[j] = rides[j+1];
                rides[j+1] = temp; 
            }
        }
    }
}


void sortByDep(ride **rides, int len) {
    int i, j, r = len - 1, flag = 1;
    ride *temp;

    for (i = 0; i < r && flag; ++i) {
        flag = 0;
        for (j = 0; j < r - i; ++j) {
            if (strcmp(rides[j]->depPos, rides[j+1]->depPos) > 0) {
                flag = 1;
                temp = rides[j];
                rides[j] = rides[j+1];
                rides[j+1] = temp; 
            }
        }
    }
}


void sortByDest(ride **rides, int len) {
    int i, j, r = len - 1, flag = 1;
    ride *temp;

    for (i = 0; i < r && flag; ++i) {
        flag = 0;
        for (j = 0; j < r - i; ++j) {
            if (strcmp(rides[j]->destPos, rides[j+1]->destPos) > 0) {
                flag = 1;
                temp = rides[j];
                rides[j] = rides[j+1];
                rides[j+1] = temp; 
            }
        }
    }
}

int *linearSearch(ride **rides, int len, char *name) {
    int *idx = malloc((len + 1) * sizeof(int)), i, j;
    for (i = 0, j = 0; i < len; ++i) {
        if (strncmp(rides[i]->depPos, name, strlen(name)) == 0) {
            idx[j++] = i;
        } 
    }
    idx[j] = -1;
    return idx;
}


int *binarySearch(ride **rides, int len, char *name) {
    int *idx = malloc((len + 1) * sizeof(int));
    int i = 0, left = 0, right = len - 1, mid, cmpRes, sSize, j;
    sSize = strlen(name);

    while (left <= right) {
        mid = left + (right - left) / 2;
        cmpRes = strncmp(name, rides[mid]->depPos, sSize);
        if (cmpRes == 0) {
            idx[i++] = mid;
            break;
        } else if (cmpRes < 0) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }


    for (j = mid - 1; j > 0; --j) {
        if (strncmp(rides[j]->depPos, name, sSize) == 0) {
            idx[i++] = j;
        }
    }

    for (j = mid + 1; j < len; ++j) {
        if (strncmp(rides[j]->depPos, name, sSize) == 0) {
            idx[i++] = j;
        }
    }


    idx[i] = -1;
    return idx;
}



void printWithConversion(ride *ride, FILE *file) {
    fprintf(file ? file : stdout, "%s %s %s %04d/%02d/%02d %02d:%02d:%02d %02d:%02d:%02d %d \n",
        ride -> rideCode,
        ride -> depPos,
        ride -> destPos,
        
        (ride -> date)/372,
        (((ride -> date) % 372) / 31) + 1,
        ((ride -> date) % 31) + 1,

        (ride -> depTime)/3600,
        ((ride -> depTime) % 3600)/60,
        (ride -> depTime) % 60,
        
        (ride -> arrTime)/3600,
        ((ride -> arrTime) % 3600)/60,
        (ride -> arrTime) % 60,
        
        ride -> delay
    );
}

// char *getString() {
//     char *string = malloc(sizeof(char)), c;
//     int i, n = sizeof(char);
//     for (i = 0; (c = getchar()) != '\n'; i++) {
//         if (i + 1 >= n) {
//             string = realloc(string, (n * 2));
//         }
//         string[i] = c;
//     }
//     string[i] = '\0';
//     return string;
// }