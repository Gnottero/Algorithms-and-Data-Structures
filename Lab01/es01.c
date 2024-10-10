#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#define MAX_SIZE 100

// Function Prototypes
char *cercaRegexp(char *src, char *regexp);
char *getString();
int checkRegexp(char *src, char *regexp);

int main() {

    int i;
    char *src, *regexp, *result;

    printf("Inserisci la stringa su cui eseguire la ricerca: ");
    src = getString();

    printf("Inserisci la regexp da cercare all'interno della stringa: ");
    regexp = getString();

    printf("Eseguo la ricerca di %s sulla stringa %s\n", regexp, src);

    result = cercaRegexp(src, regexp);

    if (result == NULL) {
        printf("Non è stata trovata alcuna occorrenza della regex fornita");
    } else {
        printf("La regexp ha trovato: ");
        for (i = 0; isalpha(result[i]); ++i) {
            printf("%c", result[i]);
        }
    }

    return 0;
}



char *getString() {
    char *string = malloc(sizeof(char)), c;
    int i, n = sizeof(char);
    for (i = 0; (c = getchar()) != '\n'; i++) {
        if (i + 1 >= n) {
            string = realloc(string, (n * 2));
        }
        string[i] = c;
    }
    string[i] = '\0';
    return string;
}



char *cercaRegexp(char *src, char *regexp) {
    if (!*src) return NULL;

    int regexpLen;
    char *ogRegexp = regexp, *ogSrc = src;
    

    while (*regexp) {
        regexpLen = checkRegexp(src, regexp);
    
        if (regexpLen) {
            ++src;
            regexp += regexpLen;
        } else return cercaRegexp(ogSrc + 1, ogRegexp);
    }

    return *regexp ? NULL : ogSrc;
}



int checkRegexp(char *src, char *regexp) {
    switch (*regexp) {
        case '.': return !!isalpha(*src);
        case '\\': return (isupper(regexp[1]) && isupper(*src) || islower(regexp[1]) && islower(*src)) * 2;
        case '[': {
            int found = 0, i, excluded = regexp[1] == '^';
            for (i = 1 + excluded; regexp[i] != ']'; ++i) {
                if (regexp[i] == *src) {
                    if (excluded) {
                        return 0;
                    } else {
                        found = 1;
                    }
                }
            }
            return (i + 1) * (excluded ? 1 : found);
        }
        default: return (*src == *regexp);
    }
}