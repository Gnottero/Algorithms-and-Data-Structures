#include <stdio.h>
#include <stdlib.h>

void malloc2dP(int ***mat, int row, int col, FILE *file);
void separa(int ***mat, int **black, int **white, int row, int col);
void printCells(int **cells, int nCells);
void freeMat(int ***mat, int row);

int main() {
    int **mat, row, col, *black, *white;
    FILE *file = fopen("mat.txt", "r");
    if (file == NULL) {
        return -1;
    }
    fscanf(file, "%d %d", &row, &col);

    malloc2dP(&mat, row, col, file);
    separa(&mat, &black, &white, row, col);

    printf("Posizioni nere: \n");
    printCells(&black, ((row*col)/2) + 1);

    printf("Posizioni bianche: \n");
    printCells(&white, (row*col)/2);

    freeMat(&mat, row);
    free(black);
    free(white);

    return 0;
}

void malloc2dP(int ***mat, int row, int col, FILE *file) {
    int i, j;
    *mat = (int **) malloc(row * sizeof(int *));
    for (i = 0; i < row; ++i) {
        (*mat)[i] = (int *) malloc(col * sizeof(int));
    }

    for (i = 0; i < row; ++i) {
        for (j = 0; j < col; ++j) {
            fscanf(file, "%d", &((*mat)[i][j]));
            printf("%d ", (*mat)[i][j]);
        }
        printf("\n");
    }
}

void separa(int ***mat, int **black, int **white, int row, int col) {
    int nBlack = 0, nWhite = 0;

    // Allocating one more slot since the matrix can can be odd on both dimensions, leading to a situation in which there are more cells of the first color (black in this case)
    *black = (int *) malloc((((row * col)/2) + 1) * sizeof(int));
    *white = (int *) malloc(((row * col)/2) * sizeof(int));
    
    for (int i = 0; i < row; ++i) {
        for (int j = 0; j < col; ++j) {
            if ((i * col + j) % 2) {
                (*white)[nWhite++] = (*mat)[i][j];
            } else {
                (*black)[nBlack++] = (*mat)[i][j];
            }
        }
    }
}

void printCells(int **cells, int nCells) {
    for (int i = 0; i < nCells; ++i) {
        printf("%d ", (*cells)[i]);
    }
    printf("\n");
}

void freeMat(int ***mat, int row) {
    for (int i = 0; i < row; ++i) {
        free((*mat)[i]);
    }
    free(*mat);
}