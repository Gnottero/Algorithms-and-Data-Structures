#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SIZE 31
#define CMD_SIZE 6


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
    r_fine
} comando_e;


ride *readDataFromFile(int *len);
// char *getString();
comando_e leggiComando(char commands[][MAX_SIZE], char ***strArray, int *argNum);
void printCommands(char commands[][MAX_SIZE]);
void execCommand(comando_e cmd, ride *rides, char **strArray, int len, int argNum);


int main() {
    int len, cmd, argNum;
    char commands[CMD_SIZE][MAX_SIZE] = {"date", "partenza", "capolinea", "ritardo", "ritardo_tot", "fine"}, **strArray;

    printf("Inserire il nome del file contente le corse: ");
    ride *rides = readDataFromFile(&len);

    if (rides != NULL) {
        printf("Inserire un comando tra i seguenti: \n");
        printCommands(commands);
    
        do {
            cmd = leggiComando(commands, &strArray, &argNum);
            execCommand(cmd, rides, strArray, len, argNum);
        } while (cmd != -1 && cmd != 5);

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


comando_e leggiComando(char commands[][MAX_SIZE], char ***strArray, int *argNum) {
    char *token, *input = malloc(MAX_SIZE * sizeof(char));
    int strLen;
    *strArray = malloc(2 * sizeof(void *));
    comando_e result = -1;

    printf("Seleziona un comando: ");
    fgets(input, MAX_SIZE, stdin);
    strLen = strlen(input);

    if (strLen == 1)
        return result;

    if (input[strLen - 1] == '\n')
        input[strLen - 1] = '\0';

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

    return result;
}


void execCommand(comando_e cmd, ride *rides, char **strArray, int len, int argNum) {
    int fyear, fmonth, fday, syear, smonth, sday, i = 0, delay = 0;
    static int const cmdArgsNum[CMD_SIZE] = {2, 1, 1, 2, 1, 0};


    if (cmd == -1) {
        printf("Il comando inserito non è un comando valido\n");
        return;
    }
    
    if (argNum != cmdArgsNum[cmd]) {
        printf("Il numero di argomenti richiesto per il comando è: %d. Gli argomenti forniti sono: %d. Input non valido\n", cmdArgsNum[cmd], argNum);
        return;
    }

    if (cmd == r_date || cmd == r_ritardo) {
        sscanf(strArray[0], "%d/%d/%d", &fyear, &fmonth, &fday);
        sscanf(strArray[1], "%d/%d/%d", &syear, &smonth, &sday);
    }

    switch (cmd) {
        case r_date:
            for ( ; i < len; ++i) {
                if (rides[i].date >= (fyear*372 + fmonth * 31 + fday) && rides[i].date <= (syear*372 + smonth * 31 + sday))
                    printf("%s %s %s\n", rides[i].rideCode, rides[i].startPos, rides[i].destPos);
            }
            break;
        case r_partenza:
            for ( ; i < len; ++i) {
                if (strcmp(rides[i].startPos, strArray[0]) == 0)
                    printf("%s %s %s %d\n", rides[i].rideCode, rides[i].startPos, rides[i].destPos, rides[i].delay);
            }
            break;
        case r_capolinea:
            for ( ; i < len; ++i) {
                if (strcmp(rides[i].destPos, strArray[0]) == 0)
                    printf("%s %s %s %d\n", rides[i].rideCode, rides[i].startPos, rides[i].destPos, rides[i].delay);
            }
            break;
        case r_ritardo:
            for ( ; i < len; ++i) {
                if (rides[i].date >= (fyear*372 + fmonth * 31 + fday) && rides[i].date <= (syear*372 + smonth * 31 + sday) && rides[i].delay > 0)
                    printf("%s %s %s %d\n", rides[i].rideCode, rides[i].startPos, rides[i].destPos, rides[i].delay);
            }
            break;
        case r_ritardo_tot:
            for ( ; i < len; ++i) {
                if (strcmp(rides[i].rideCode, strArray[0]) == 0 && rides[i].delay)
                    delay += rides[i].delay;
            }
            printf("Il ritardo complessivo delle corse identificate dal codice %s è stato di %d minuti\n", strArray[0], delay);
            break;
    }
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
        rides[i].date = year * 372 + month * 31 + day;
        rides[i].depTime = depHour * 3600 + depMin * 60 + depSec;
        rides[i].arrTime = arrHour * 3600 + arrMin * 60 + arrSec;
        ++i;
    }
    fclose(file);
    return rides;
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