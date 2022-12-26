#include <stdio.h>

int main() {
    int tinggi, lebar, columnPlaced;
    scanf("%d %d %d", &tinggi, &lebar, &columnPlaced);

    int matrix[tinggi][lebar];
    for (int i = 0; i < tinggi; i++) {
        for (int j = 0; j < lebar; j++) {
            scanf("%d", &matrix[i][j]);
        }
    }

    int totalPoints = 0;

    int robotX = columnPlaced;
    int robotY = 0;

    do {
        // robot can only move diagonally downwards
        for (int scannedTinggi = robotY; scannedTinggi < tinggi; scannedTinggi++) {
            int scannedLebar = columnPlaced;
            scannedLebar += scannedTinggi;
            if (scannedLebar >= lebar) {
                break;
            }

            printf("Scanning1 at (%d, %d)\n", scannedTinggi, scannedLebar);
            int points = matrix[scannedTinggi][scannedLebar];
            totalPoints += points;

            robotX = scannedLebar;
            robotY = scannedTinggi;
        }

        robotY += 1;
        robotX -= 1;

        if (robotY >= tinggi) {
            printf("TOT1: %d\n", totalPoints);
            return 0;
        }

        int del = 0;
        for (int scannedTinggi = robotY; scannedTinggi < tinggi; scannedTinggi++) {
            int scannedLebar = robotX;
            scannedLebar -= del++;
            if (scannedLebar < 0) {
                printf("break2\n");
                break;
            }

            printf("Scanning2 at (%d, %d)\n", scannedTinggi, scannedLebar);
            int points = matrix[scannedTinggi][scannedLebar];
            totalPoints += points;

            robotX = scannedLebar;
            robotY = scannedTinggi;
        }

        robotY += 1;
        robotX += 1;

        if (robotY >= tinggi) {
            printf("TOT2: %d\n", totalPoints);
            return 0;
        }
    } while (true);

    printf("TOT3: %d\n", totalPoints);
    return 0;
}