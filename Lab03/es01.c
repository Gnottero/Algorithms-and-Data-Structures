#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#define MIN_VEC 10

int majority(int *a, int N);
int *getVector(int *len);

int main() {
    int len, *vec, maj;
    printf("Inserisci i valori del vettore: \n");
    vec = getVector(&len);
    
    maj = majority(vec, len);

    if (maj > 0) {
        printf("Il maggiorante è: %d\n", maj);
    } else {
        printf("Non è stato trovato un maggiorante");
    }
    
    return 0;
}

int *getVector(int *len) {
    int *vec = malloc(MIN_VEC * sizeof(int)), j = MIN_VEC;
    *len = 0;
    while (fscanf(stdin, "%d", &(vec[*len]) ) == 1) {
        if ((*len) + 1 >= j) {
            vec = realloc(vec, j * 2);
            j *= 2;
        }
        ++*len;
    }
    vec[*len] = (*len) / 2;
    return vec;
}

int majority(int *a, int N) {
    int found = 0, i;
    for (i = 0; i < N; ++i) {
        if (a[0] == a[i]) {
            ++found;
        }
    }

    if (found > a[N]) {
        return a[0];
    }

    if (N == 1) {
        return -1;
    }

    majority(++a, N-1);
}

