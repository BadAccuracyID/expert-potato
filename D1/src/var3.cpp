#include <stdio.h>

int main() {
    int input;
    scanf("%d", &input);
    getchar();

    // O = 1
    // S = 6
    // N = 9
    // X = 10
    // L = 50
    // C = 100
    // R = 600
    // M = 1000
    char converted[128];
    int i = 0;
    while (input > 0) {
        if (input >= 1000) {
            converted[i] = 'M';
            input -= 1000;
        } else if (input >= 600) {
            converted[i] = 'R';
            input -= 600;
        } else if (input >= 100) {
            converted[i] = 'C';
            input -= 100;
        } else if (input >= 50) {
            converted[i] = 'L';
            input -= 50;
        } else if (input >= 10) {
            converted[i] = 'X';
            input -= 10;
        } else if (input >= 9) {
            converted[i] = 'N';
            input -= 9;
        } else if (input >= 6) {
            converted[i] = 'S';
            input -= 6;
        } else if (input >= 1) {
            converted[i] = 'O';
            input -= 1;
        }
        i++;
    }

    printf("%s\n", converted);

    return 0;
}