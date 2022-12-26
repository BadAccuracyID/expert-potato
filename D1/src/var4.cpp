#include <stdio.h>

int main() {
    unsigned int input;
    scanf("%u", &input);
    getchar();

    // count total number of digit 1, 3, and 6
    int count = 0;
    for (int i = 1; i <= input; i++) {
        int temp = i;
        while (temp > 0) {
            int digit = temp % 10;
            if (digit == 1) {
                // 1
                count++;
            } else if (digit == 3) {
                // 3
                count++;
            } else if (digit == 6) {
                // 6
                count++;
            }
            temp /= 10;
        }
    }

    // print the result
    printf("%d\n", count);

    return 0;
}