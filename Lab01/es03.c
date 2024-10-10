#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SIZE 31
#define CMD_SIZE 8


typedef struct {
    char rideCode[MAX_SIZE], startPos[MAX_SIZE], destPos[MAX_SIZE];
    int date, depTime, arrTime, delay; 
} ride;


typedef enum {
    r_date,
    r_partenza,
    r_capolinea,
    r_ritardo,
    r_ritardo_tot,
    r_ordina,
    r_cerca,
    r_fine
} comando_e;


ride *readDataFromFile(int *len);
comando_e leggiComando(char commands[][MAX_SIZE], char ***strArray, int *argNum, char *outputPath);
void printCommands(char commands[][MAX_SIZE]);
void execCommand(comando_e cmd, ride *rides, char **strArray, int len, int argNum, char *outputPath);
void printWithConversion(ride *ride, FILE *file);
void getTrimmedLine(char *line);


int main() {
    int len, cmd, argNum;
    char commands[CMD_SIZE][MAX_SIZE] = {"date", "partenza", "capolinea", "ritardo", "ritardo_tot", "ordina", "cerca", "fine"}; 
    char **strArray, outputPath[MAX_SIZE];

    printf("Inserire il nome del file contente le corse: ");
    ride *rides = readDataFromFile(&len);

    if (rides != NULL) {
        printf("Inserire un comando tra i seguenti: \n");
        printCommands(commands);
    
        do {
            cmd = leggiComando(commands, &strArray, &argNum, outputPath);
            execCommand(cmd, rides, strArray, len, argNum, outputPath);
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


void execCommand(comando_e cmd, ride *rides, char **strArray, int len, int argNum, char *outputPath) {
    int fyear, fmonth, fday, syear, smonth, sday, i = 0, delay = 0, fdate, sdate;
    static int const cmdArgsNum[CMD_SIZE] = {2, 1, 1, 2, 1, 1, 2, 0};
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
                // printf("Ride Value: %d %d %d \n", rides[i].date, fyear*372 + fmonth * 31 + fday, syear*372 + smonth * 31 + sday);
                if (rides[i].date >= fdate && rides[i].date <= sdate)
                    printWithConversion(&rides[i], file);
            }
            break;
        case r_partenza:
            for ( ; i < len; ++i) {
                if (strcmp(rides[i].startPos, strArray[0]) == 0)
                    printWithConversion(&rides[i], file);
            }
            break;
        case r_capolinea:
            for ( ; i < len; ++i) {
                if (strcmp(rides[i].destPos, strArray[0]) == 0)
                    printWithConversion(&rides[i], file);
            }
            break;
        case r_ritardo:
            for ( ; i < len; ++i) {
                if (rides[i].date >= fdate && rides[i].date <= sdate && rides[i].delay > 0)
                    printWithConversion(&rides[i], file);
            }
            break;
        case r_ritardo_tot:
            for ( ; i < len; ++i) {
                if (strcmp(rides[i].rideCode, strArray[0]) == 0 && rides[i].delay)
                    delay += rides[i].delay;
            }
            fprintf(file ? file : stdout, "Il ritardo complessivo delle corse identificate dal codice %s è stato di %d minuti\n", strArray[0], delay); 
            break;
    }

    if (file)
        fclose(file);
}


ride *readDataFromFile(int *len) {
    FILE *file;
    char fileName[MAX_SIZE];
    int i = 0, year, month, day, depHour, depMin, depSec, arrHour, arrMin, arrSec;

    scanf("%s", fileName);
    getchar();

    file = fopen(fileName, "r");

    if (file == NULL) return NULL;

    fscanf(file, "%d", len);

    if (*len > 1000) return NULL;

    ride *rides = malloc((*len) * sizeof(ride));

    while (fscanf(file, "%s %s %s %d/%d/%d %d:%d:%d %d:%d:%d %d", rides[i].rideCode, rides[i].startPos, rides[i].destPos, &year, &month, &day, &depHour, &depMin, &depSec, &arrHour, &arrMin, &arrSec, &rides[i].delay) == 13) {
        rides[i].date = year * 372 + (month - 1) * 31 + day - 1;
        rides[i].depTime = depHour * 3600 + depMin * 60 + depSec;
        rides[i].arrTime = arrHour * 3600 + arrMin * 60 + arrSec;
        ++i;
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


void printWithConversion(ride *ride, FILE *file) {
    fprintf(file ? file : stdout, "%s %s %s %04d/%02d/%02d %02d:%02d:%02d %02d:%02d:%02d %d \n",
        ride -> rideCode,
        ride -> startPos,
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