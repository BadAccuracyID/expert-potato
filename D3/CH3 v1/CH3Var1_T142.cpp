#include <math.h>
#include <stdio.h>

#include <algorithm>

int main() {
    int arrSize = 0;
    scanf("%d", &arrSize);
    bool odd = false;
    if (arrSize % 2 == 1) {
        odd = true;
    }

    int arr[arrSize];
    for (int i = 0; i < arrSize; i++) {
        scanf("%d", &arr[i]);
    }

    // actual stuff
    for (int z = 0; z < (odd ? (arrSize + 1) / 2 : arrSize / 2); z++) {
        int i = 0;
        int j = 0;
        do {
            bool modify = false;
            if (odd) {
                if (i + 1 >= arrSize) {
                    modify = true;
                }
            }
            int a = arr[modify ? i - 1 : i];
            int b = arr[modify ? i : i + 1];

            int gcd = std::__gcd(a, b);
            arr[i - j] = gcd;

            i += 2;
            j += 1;
        } while (i < (sizeof(arr) / sizeof(arr[0])));
    }

    printf("%d\n", arr[0]);
    return 0;
}