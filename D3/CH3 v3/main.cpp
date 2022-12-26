#include <math.h>
#include <stdio.h>

int main() {
    int arrSize = 0;
    scanf("%d", &arrSize);
    arrSize = pow(2, arrSize);

    int arr[arrSize];
    for (int i = 0; i < arrSize; i++) {
        scanf("%d", &arr[i]);
    }

    if (arrSize == 1) {
        printf("%d", arr[0]);
        return 0;
    }

    for (int z = 0; z < arrSize / 2; z++) {
        int i = 0;
        do {
            int a = arr[i];
            int b = arr[i + 1];
            if (a < b) {
                arr[i] = a;
            } else {
                arr[i] = b;
            }

            int c = arr[i + 2];
            int d = arr[i + 3];
            if (c > d) {
                arr[i + 1] = c;
            } else {
                arr[i + 1] = d;
            }

            i += 4;
        } while (i < sizeof(arr) / sizeof(arr[0]));
    }

    printf("%d", arr[0]);
    return 0;
}