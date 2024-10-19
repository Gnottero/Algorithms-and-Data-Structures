#include <stdio.h>

int gcd(int a, int b);
void swapValues(int *a, int *b);


int main() {
    int a, b;

    // Get the values    
    printf("Inserisci i valori a e b di cui calcolare il gcd: ");
    scanf("%d %d", &a, &b);

    printf("Il gcd tra %d e %d è %d \n", a, b, gcd(a, b));

    return 0;
}


void swapValues(int *a, int *b) {
    int aux = *a;
    *a = *b;
    *b = aux;
}


int gcd(int a, int b) {
    int rema, remb;

    if (b == 1) {
        return 1;
    } else if (a == b) {
        return b;
    } else if (b > a) {
        swapValues(&a, &b);
    }

    printf("I valori di a e b sono: %d %d \n", a, b);

    rema = a % 2;
    remb = b % 2;

    if (b == 0)
        return a;    
    if (rema == 0 && remb == 0) {
        return 2 * gcd(a/2, b/2);
    } else if (rema != 0 && remb == 0) {
        return gcd(a, b/2);
    } else if (rema == 0 && remb != 0) {
        return gcd(b, a/2);
    } else {
        return gcd((a - b)/2, b);
    }
}