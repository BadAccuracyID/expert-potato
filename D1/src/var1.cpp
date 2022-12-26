#include <stdio.h>

int main() {
    int cases;
    scanf("%d", &cases);

    for (int i = 0; i < cases; i++) {
        int distance, minLimit, maxLimit, divisor;
        scanf("%d %d %d %d", &distance, &minLimit, &maxLimit, &divisor);
        getchar();

        // check
        if (distance < minLimit || distance > maxLimit || distance % divisor != 0) {
            printf("Not Eligible\n");
        } else {
            printf("Eligible\n");
        }
    }

    return 0;
}