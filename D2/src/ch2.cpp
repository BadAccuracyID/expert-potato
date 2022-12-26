#include <stdio.h>
#include <string.h>

#include <cctype>
#include <cstdlib>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

struct Stock {
    char name[128];
    double price;

    char code[128];
    char label[128];
};

Stock stocks[128];

double getIDXComposite() {
    int size = 0;
    for (int i = 0; i < 128; i++) {
        if (stocks[i].price == 0) {
            break;
        }

        size++;
    }

    double total = 0;
    for (int i = 0; i < size; i++) {
        total += stocks[i].price;
    }
    total = ((total / size) * 3) / 2;

    return total;
}

void displayLogo() {
    // BStockMarket
    printf("  ____   _____ _______         _    __  __            _        _   \n");
    printf(" |  _ \\ / ____|__   __|       | |  |  \\/  |          | |      | |  \n");
    printf(" | |_) | (___    | | ___   ___| | _| \\  / | __ _ _ __| | _____| |_ \n");
    printf(" |  _ < \\___ \\   | |/ _ \\ / __| |/ / |\\/| |/ _` | '__| |/ / _ \\ __|\n");
    printf(" | |_) |____) |  | | (_) | (__|   <| |  | | (_| | |  |   <  __/ |_ \n");
    printf(" |____/|_____/   |_|\\___/ \\___|_|\\_\\_|  |_|\\__,_|_|  |_|\\_\\___|\\__|\n");
    printf("                                                                   \n");
    printf("\n\n");
}

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void insertStock() {
    clearScreen();
    displayLogo();

    int emptyIndex = 0;
    for (int i = 0; i < 128; i++) {
        if (strlen(stocks[i].name) == 0) {
            emptyIndex = i;
            break;
        }
    }

    char stockName[128] = {0};
    printf("Input the stock name [>= 4 characters]: ");
    scanf("%[^\n]", stockName);
    getchar();
    while (true) {
        if (strlen(stockName) < 4) {
            printf("Invalid stock name. Please try again.\n");
            printf("Input the stock name [>= 4 characters]: ");
            scanf("%[^\n]", stockName);
            getchar();
        } else {
            break;
        }
    }
    strcpy(stocks[emptyIndex].name, stockName);

    double stockPrice = 0;
    printf("Input the stock price [>= 500]: ");
    scanf("%lf", &stockPrice);
    getchar();
    while (true) {
        if (stockPrice < 500) {
            printf("Invalid stock price. Please try again.\n");
            printf("Input the stock price [>= 500]: ");
            scanf("%lf", &stockPrice);
            getchar();
        } else {
            break;
        }
    }
    stocks[emptyIndex].price = stockPrice;

    // generate stock code
    char stockCode[128] = {0};
    do {
        // 4 random uppercase letter from stockName
        for (int i = 0;; i++) {
            char c = stockName[rand() % strlen(stockName)];
            if (islower(c)) {
                c = toupper(c);
            }
            if (!isalpha(c)) {
                continue;
            }

            stockCode[i] = c;
            if (strlen(stockCode) == 4) {
                break;
            }
        }
        // to uppercase
        for (int i = 0; i < 4; i++) {
            stockCode[i] = toupper(stockCode[i]);
        }

        // get random number from 1 to 100
        int randomNumber = rand() % 100 + 1;
        // convert randomNumber to string
        char randomNumberString[128] = {0};
        sprintf(randomNumberString, "%d", randomNumber);
        // append randomNumberString to stockCode
        strcat(stockCode, randomNumberString);

        // check for duplicates
        bool isDuplicate = false;
        for (int i = 0; i < 128; i++) {
            if (strcmp(stockCode, stocks[i].code) == 0) {
                isDuplicate = true;
                break;
            }
        }

        if (!isDuplicate) {
            break;
        }
    } while (true);
    strcpy(stocks[emptyIndex].code, stockCode);

    // generate stock label
    int index = stockPrice * (rand() % 100 + 1);
    while (index > 8562) {
        // (stock price / (((random number % 37) + 1) * 2)) * ((random number % 100) + 1)
        index = stockPrice / (((rand() % 37) + 1) * 2) * ((rand() % 100) + 1);
    }

    if (index >= 5708 && index <= 8562) {
        strcpy(stocks[emptyIndex].label, "Blue Chip");
    } else if (index >= 2854 && index <= 5707) {
        strcpy(stocks[emptyIndex].label, "Middle Chip");
    } else if (index >= 1 && index <= 2853) {
        strcpy(stocks[emptyIndex].label, "Small Chip");
    }

    printf("\n\nSuccessfully added stock.\n");
    printf("Name: %s\n", stockName);
    printf("Code: %s\n", stockCode);

    // for debugging
    printf("Price: %.2lf\n", stockPrice);
    printf("Label: %s\n\n", stocks[emptyIndex].label);

    printf("\n\nPress ENTER to continue...\n");
    getchar();
}

void viewAllStock(bool exitable) {
    clearScreen();
    displayLogo();
    if (strlen(stocks[0].name) == 0) {
        printf("No stock available.\n");

        printf("\n\nPress ENTER to continue...\n");
        getchar();
        return;
    }

    // table
    printf("Name\t\tCode\t\tPrice\t\tLabel\n");
    printf("----\t\t----\t\t-----\t\t-----\n");
    for (int i = 0; i < 128; i++) {
        if (strlen(stocks[i].name) == 0) {
            break;
        }
        printf("%s\t\t%s\t\t%.2lf\t\t%s\n", stocks[i].name, stocks[i].code, stocks[i].price, stocks[i].label);
    }

    printf("\n\nIDX Composite: %.2lf\n", getIDXComposite());

    if (exitable) {
        printf("\n\nPress ENTER to continue...\n");
        getchar();
    }
}

void viewAllStock() {
    viewAllStock(true);
}

void deleteStock() {
    viewAllStock(false);

    char stockCode[128] = {0};
    printf("\n\nInput the stock code [<= 7 characters] [case insensitive] [type 'cancel' to cancel the deletion]: ");
    scanf("%s", stockCode);
    getchar();
    while (true) {
        if (strlen(stockCode) > 7) {
            printf("Invalid stock code. Please try again.\n");
            printf("Input the stock code [<= 7 characters] [case insensitive] [type 'cancel' to cancel the deletion]: ");
            scanf("%s", stockCode);
            getchar();
            continue;
        }

        if (strcmp(stockCode, "cancel") == 0) {
            return;
        }

        bool isFound = false;

#ifdef _WIN32
        for (int i = 0; i < 128; i++) {
            if (stricmp(stockCode, stocks[i].code) == 0) {
                isFound = true;
                break;
            }
        }
#else
        for (int i = 0; i < 128; i++) {
            if (strcasecmp(stockCode, stocks[i].code) == 0) {
                isFound = true;
                break;
            }
        }
#endif

        if (!isFound) {
            printf("Stock code not found. Please try again.\n");
            printf("Input the stock code [<= 7 characters] [case insensitive] [type 'cancel' to cancel the deletion]: ");
            scanf("%s", stockCode);
            getchar();
            continue;
        }

        break;
    }

    for (int i = 0; i < 128; i++) {
#ifdef _WIN32
        if (stricmp(stockCode, stocks[i].code) == 0) {
            for (int j = i; j < 128; j++) {
                if (strlen(stocks[j + 1].name) == 0) {
                    break;
                }
                strcpy(stocks[j].name, stocks[j + 1].name);
                strcpy(stocks[j].code, stocks[j + 1].code);
                stocks[j].price = stocks[j + 1].price;
                strcpy(stocks[j].label, stocks[j + 1].label);
            }
            break;
        }
#else
        if (strcasecmp(stockCode, stocks[i].code) == 0) {
            for (int j = i; j < 128; j++) {
                if (strlen(stocks[j + 1].name) == 0) {
                    break;
                }
                strcpy(stocks[j].name, stocks[j + 1].name);
                strcpy(stocks[j].code, stocks[j + 1].code);
                stocks[j].price = stocks[j + 1].price;
                strcpy(stocks[j].label, stocks[j + 1].label);
            }
            break;
        }
#endif
    }

    // delete the last non null stock
    for (int i = 128; i >= 0; i--) {
        if (strlen(stocks[i].name) != 0) {
            strcpy(stocks[i].name, "");
            strcpy(stocks[i].code, "");
            stocks[i].price = 0;
            strcpy(stocks[i].label, "");
            break;
        }
    }

    printf("\n\nSuccessfully deleted stock: %s\n", stockCode);

    printf("\n\nPress ENTER to continue...\n");
    getchar();
}

int displayMainMenu() {
    clearScreen();
    displayLogo();

    printf("1. Insert stock\n");
    printf("2. View all stock\n");
    printf("3. Delete Stock\n");
    printf("4. Exit Program\n");
    printf(">> ");

    int choice = 0;
    scanf("%d", &choice);
    getchar();

    while (true) {
        if (choice == 1) {
            insertStock();
            break;
        } else if (choice == 2) {
            viewAllStock();
            break;
        } else if (choice == 3) {
            deleteStock();
            break;
        } else if (choice == 4) {
            clearScreen();
            printf("Alongside courage and perseverence\n");
            printf("  we shape and define our future\n");
            printf("         ~ 23-1 ~\n");
            sleep(1);
            exit(0);
            return 1;
        } else {
            printf("Invalid input\n");
            printf(">> ");
            scanf("%d", &choice);
            getchar();
        }
    }

    return 0;
}

int main() {
    while (true) {
        displayMainMenu();
    }

    return 0;
}