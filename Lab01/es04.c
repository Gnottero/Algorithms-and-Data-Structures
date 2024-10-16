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
void sortByCriteria(ride **rides, ride **toSort, int len, int value);
void linearSearch(int len, int nameLen, ride **rides, char *name, FILE *file);
void binarySearch(int left, int right, int nameLen, ride **rides, char *name, FILE *file);
void getTrimmedLine(char *line);


int main() {
    int len, cmd, argNum;
    char commands[CMD_SIZE][MAX_SIZE] = {"date", "partenza", "capolinea", "ritardo", "ritardo_tot", "ordina_date", "ordina_codice", "ordina_partenza", "ordina_arrivo", "ricerca_lineare", "ricerca_dicotomica", "fine"}; 
    char **strArray, outputPath[MAX_SIZE];
    ride **orderedDate, **orderedCode, **orderedDep, **orderedDest;



    printf("Inserire il nome del file contente le corse: ");
    ride **rides = readDataFromFile(&len);

    if (rides != NULL) {
        printf("Inserire un comando tra i seguenti: \n");
        printCommands(commands);
    
        orderedDate = malloc(len * sizeof(void *));
        orderedCode = malloc(len * sizeof(void *));
        orderedDep = malloc(len * sizeof(void *));
        orderedDest = malloc(len * sizeof(void *));

        sortByCriteria(rides, orderedDate, len, r_ordina_date);
        sortByCriteria(rides, orderedCode, len, r_ordina_codice);
        sortByCriteria(rides, orderedDep, len, r_ordina_partenza);
        sortByCriteria(rides, orderedDest, len, r_ordina_arrivo);
    
        do {
            cmd = leggiComando(commands, &strArray, &argNum, outputPath);
            switch (cmd) {
                case r_ordina_date:
                    execCommand(cmd, orderedDate, strArray, len, argNum, outputPath);
                    break;
                case r_ordina_codice:
                    execCommand(cmd, orderedCode, strArray, len, argNum, outputPath);
                    break;
                case r_ordina_partenza:
                case r_ricerca_dicotomica:
                    execCommand(cmd, orderedDep, strArray, len, argNum, outputPath);
                    break;
                case r_ordina_arrivo:
                    execCommand(cmd, orderedDest, strArray, len, argNum, outputPath);
                    break;
                default:
                    execCommand(cmd, rides, strArray, len, argNum, outputPath);
                    break;
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
    int fyear, fmonth, fday, syear, smonth, sday, i, delay = 0, fdate, sdate, nameLen;
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

    if (cmd == r_ricerca_lineare || cmd == r_ricerca_dicotomica) {
        nameLen = strlen(strArray[0]);
    }

    for (i = 0; i < len && cmd >= r_date && cmd <= r_ordina_arrivo; ++i) {
        switch (cmd) {
            case r_date:
                if (rides[i] -> date >= fdate && rides[i] -> date <= sdate) {
                    printWithConversion(rides[i], file);
                }
                break;
            case r_partenza:
                if (strcmp(rides[i] -> depPos, strArray[0]) == 0) {
                    printWithConversion(rides[i], file);
                }
                break;
            case r_capolinea:
                if (strcmp(rides[i] -> destPos, strArray[0]) == 0) {
                    printWithConversion(rides[i], file);
                }
                break;
            case r_ritardo:
                if (rides[i]->date >= fdate && rides[i]->date <= sdate && rides[i]->delay > 0) {
                    printWithConversion(rides[i], file);
                }
                break;
            case r_ritardo_tot:
                if (strcmp(rides[i] -> rideCode, strArray[0]) == 0 && rides[i] -> delay) {
                    delay += rides[i]->delay;
                }
                break;
            case r_ordina_date : case r_ordina_codice : case r_ordina_partenza : case r_ordina_arrivo:
                printWithConversion(rides[i], file);
                break;
        }
    }

    if (cmd == r_ritardo_tot) {
        fprintf(file ? file : stdout, "Il ritardo complessivo delle corse identificate dal codice %s è stato di %d minuti\n", strArray[0], delay);
    } else if (cmd == r_ricerca_lineare) {
        linearSearch(len, nameLen, rides, strArray[0], file);
    } else if (cmd == r_ricerca_dicotomica) {
        binarySearch(0, len - 1, nameLen, rides, strArray[0], file);
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



int compareRides(ride *fride, ride *sride, int criteria) {
    switch (criteria) {
        case r_ordina_date:
            return (fride -> date > sride -> date || (fride -> date == sride -> date && fride -> depTime > sride -> depTime));
        case r_ordina_codice:
            return (strcmp(fride -> rideCode, sride -> rideCode) > 0);
        case r_ordina_partenza:
            return (strcmp(fride -> depPos, sride -> depPos) > 0);
        case r_ordina_arrivo:
            return (strcmp(fride -> destPos, sride -> destPos) > 0);
    }
}



void sortByCriteria(ride **rides, ride **toSort, int len, int value) {
    int i, j, r = len - 1, flag = 1;
    ride *temp;
    memcpy(toSort, rides, len * sizeof(void *));
    for (i = 0; i < r && flag; ++i) {
        flag = 0;
        for (j = 0; j < r - i; ++j) {
            if (compareRides(toSort[j], toSort[j+1], value)) {
                flag = 1;
                temp = toSort[j];
                toSort[j] = toSort[j+1];
                toSort[j+1] = temp;
            }
        }
    }
}



void linearSearch(int len, int nameLen, ride **rides, char *name, FILE *file) {
    for (int i = 0; i < len; ++i) {
        if (strncmp(rides[i] -> depPos, name, nameLen) == 0) {
            printWithConversion(rides[i], file);
        } 
    }
}



void binarySearch(int left, int right, int nameLen, ride **rides, char *name, FILE *file) {
    int m = (left + right)/2;
    if (left > right) {
        return;
    }

    if (strncmp(rides[m] -> depPos, name, nameLen) == 0) {
        printWithConversion(rides[m], file);
    }

    if (strncmp(rides[m] -> depPos, name, nameLen) > 0) {
        binarySearch(left, m - 1, nameLen, rides, name, file);
    }
    binarySearch(m + 1, right, nameLen, rides, name, file);
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