#include <stdio.h>

int main() {
    int diamondSize, input;
    scanf("%d", &input);
    getchar();

    if (input % 2 == 0) {
        printf("Invalid input\n");
        return 0;
    }

    // 5 = 5 - 3
    // 7 = 7 - 4
    // 9 = 9 - 5

    int decreaser = 2;
    for (int z = 5; z <= input; z += 2) {
        decreaser++;
    }
    diamondSize = input - decreaser;

    // 1st row
    for (int i = 0; i < input; i++) {
        printf("*");
    }
    printf("\n");

    // middle hollow diamond thing
    for (int i = 1; i <= diamondSize; i++) {
        int k = 0;

        // left wall
        printf("*");

        for (int j = 1; j <= diamondSize - i; j++) {
            printf(" ");
        }
        while (k != (2 * i - 1)) {
            if (k == 0 || k == 2 * i - 2) {
                printf("*");
            } else {
                printf(" ");
            }
            k++;
        }

        if (k == input - 2) {
            printf("*\n");
        } else {
            for (int j = 1; j <= diamondSize - i; j++) {
                printf(" ");
            }
            printf("*\n");
        }
    }
    diamondSize--;
    for (int i = diamondSize; i >= 1; i--) {
        // left wall
        printf("*");

        for (int j = 0; j <= diamondSize - i; j++) {
            printf(" ");
        }
        
        int k = 0;
        while (k != (2 * i - 1)) {
            if (k == 0 || k == 2 * i - 2) {
                printf("*");
            } else {
                printf(" ");
            }
            k++;
        }

        if (k == input - 2) {
            printf("*\n");
        } else {
            for (int j = 0; j <= diamondSize - i; j++) {
                printf(" ");
            }
            printf("*\n");
        }
    }

    // last row
    for (int i = 0; i < input; i++) {
        printf("*");
    }
    printf("\n");

    return 0;
}