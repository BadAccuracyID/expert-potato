#include <stdio.h>

int main() {
    int cases;
    scanf("%d", &cases);

    for (int i = 0; i < cases; i++) {
        int choclate, students;
        scanf("%d %d", &choclate, &students);
        getchar();

        int toBuy = 0;
        if (choclate <= students + 1) {
            toBuy = students - choclate + 1;
        }

        printf("%d\n", toBuy);
    }

    return 0;
}