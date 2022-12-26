#include <cstdio>
#include <cstring>

#include <cstdlib>

#ifdef _WIN32
#include <Windows.h>
#else

#include <unistd.h>

#endif

struct TelephoneBook {
    char name[128];
    char number[128];
    char address[128];
};
char epLogo[128][128];

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void printHeader() {
    printf(" __      __  _   _______ _                        ____              _    ");
    printf("\n");
    printf(R"( \ \    / / | | |__   __| |                      |  _ \            | |   )");
    printf("\n");
    printf("  \\ \\  / /__| | ___| |  | |__   ___  _ __   ___  | |_) | ___   ___ | | __");
    printf("\n");
    printf(R"(   \ \/ / _ \ |/ _ \ |  | '_ \ / _ \| '_ \ / _ \ |  _ < / _ \ / _ \| |/ /)");
    printf("\n");
    printf("    \\  /  __/ |  __/ |  | | | | (_) | | | |  __/ | |_) | (_) | (_) |   < ");
    printf("\n");
    printf(R"(     \/ \___|_|\___|_|  |_| |_|\___/|_| |_|\___| |____/ \___/ \___/|_|\_\)");
    printf("\n\n");
}

void addTelephoneBook() {
    clearScreen();
    printHeader();

    TelephoneBook telephoneBook{};

    char name[128];
    printf("Name [> 4 characters]: ");
    scanf("%[^\n]", name);
    getchar();
    while (true) {
        if (strlen(name) > 4) {
            break;
        }

        printf("Name must be more than 4 characters!\n");
        printf("Name [> 4 characters]: ");
        scanf("%[^\n]", name);
        getchar();
    }
    strcpy(telephoneBook.name, name);

    char number[128];
    printf("Number [Must init with 0 | 9..11 characters long]: ");
    scanf("%s", number);
    getchar();
    while (true) {
        bool startsWithZero = number[0] == '0';
        if (startsWithZero && strlen(number) >= 9 && strlen(number) <= 11) {
            break;
        }

        if (!startsWithZero) {
            printf("Number must init with 0!\n");
        } else {
            printf("Number must be 9 to 11 characters long!\n");
        }
        printf("Number [Must init with 0 | 9..11 characters long]: ");
        scanf("%s", number);
        getchar();
    }
    strcpy(telephoneBook.number, number);

    char address[128];
    printf("Address [Must end with 'Street']: ");
    scanf("%[^\n]", address);
    getchar();
    while (true) {
        if (strstr(address, "Street") != nullptr) {
            break;
        }

        printf("Address must end with 'Street'!\n");
        printf("Address [Must end with 'Street']: ");
        scanf("%[^\n]", address);
        getchar();
    }
    strcpy(telephoneBook.address, address);

    FILE *file = fopen("telephonebook.txt", "a");
    fprintf(file, "%s#%s#%s \n", telephoneBook.name, telephoneBook.number, telephoneBook.address);
    fclose(file);

    printf("\n\nData added successfully!\n");
    printf("Press any key to continue...");
    getchar();
}

void searchBook() {
    clearScreen();
    printHeader();

    char name[128];
    printf("Name: ");
    scanf("%[^\n]", name);
    getchar();

    FILE *file = fopen("telephonebook.txt", "r");
    bool found = false;
    char bufferedName[128];
    char bufferedNumber[128];
    char bufferedAddress[128];
    while (fscanf(file, "%[^#]#%[^#]#%[^\n]\n", bufferedName, bufferedNumber, bufferedAddress) != EOF) {
#ifdef _WIN32
        if (stricmp(bufferedName, name) == 0) {
#else
        if (strcasecmp(bufferedName, name) == 0) {
#endif
            found = true;

            clearScreen();
            printHeader();
            printf("Name: %s\n", bufferedName);
            printf("Number: %s\n", bufferedNumber);
            printf("Address: %s\n", bufferedAddress);
            break;
        }
    }

    if (!found) {
        printf("Data not found!\n");
    }

    fclose(file);
    printf("\nPress any key to continue...");
    getchar();
}

void viewBook() {
    clearScreen();
    printHeader();

    char sort[16];
    printf("How would you like to view the data? [ASC | DESC]: ");
    scanf("%s", sort);
    getchar();
    while (true) {
#ifdef _WIN32
        if (stricmp(sort, "ASC") == 0 || stricmp(sort, "DESC") == 0) {
#else
        if (strcasecmp(sort, "ASC") == 0 || strcasecmp(sort, "DESC") == 0) {
#endif
            break;
        }

        printf("Invalid input!\n");
        printf("How would you like to view the data? [ASC | DESC]: ");
        scanf("%s", sort);
        getchar();
    }

    char bufferedName[128];
    char bufferedNumber[128];
    char bufferedAddress[128];

    TelephoneBook telephoneBooks[128];
    int index = 0;

    FILE *file = fopen("telephonebook.txt", "r");
    while (fscanf(file, "%[^#]#%[^#]#%[^\n]\n", bufferedName, bufferedNumber, bufferedAddress) != EOF) {
        strcpy(telephoneBooks[index].name, bufferedName);
        strcpy(telephoneBooks[index].number, bufferedNumber);
        strcpy(telephoneBooks[index].address, bufferedAddress);
        index++;
    }
    fclose(file);

#ifdef _WIN32
    if (stricmp(sort, "ASC") == 0) {
#else
    if (strcasecmp(sort, "ASC") == 0) {
#endif
        // bubble sort ascending
        for (int i = 0; i < index; i++) {
            for (int j = 0; j < index - 1; j++) {
                if (strcmp(telephoneBooks[j].name, telephoneBooks[j + 1].name) > 0) {
                    TelephoneBook temp = telephoneBooks[j];
                    telephoneBooks[j] = telephoneBooks[j + 1];
                    telephoneBooks[j + 1] = temp;
                }
            }
        }
    } else {
        // bubble sort descending
        for (int i = 0; i < index; i++) {
            for (int j = 0; j < index - 1; j++) {
                if (strcmp(telephoneBooks[j].name, telephoneBooks[j + 1].name) < 0) {
                    TelephoneBook temp = telephoneBooks[j];
                    telephoneBooks[j] = telephoneBooks[j + 1];
                    telephoneBooks[j + 1] = temp;
                }
            }
        }
    }

    int i = 0;
    // create table header
    printf("----------------------------------------------------------------------------------------------------------------");
    printf("\n| %-3s | %-32s | %-32s | %-32s |", "No.", "Name", "Phone Number", "Address");
    printf("\n----------------------------------------------------------------------------------------------------------------");
    // table contents
    for (auto &item: telephoneBooks) {
        if (strcmp(item.name, "") == 0) {
            break;
        }

        printf("\n| %-3d | %-32s | %-32s | %-32s |", ++i, item.name, item.number, item.address);
    }
    printf("\n----------------------------------------------------------------------------------------------------------------");

    printf("\nPress any key to continue...");
    getchar();
}

void mainMenu() {
    clearScreen();
    printHeader();

    printf("1. Add telephone book\n");
    printf("2. Search telephone book\n");
    printf("3. View telephone book\n");
    printf("4. Exit\n");
    printf(">> ");

    int choice = 0;
    scanf("%d", &choice);
    getchar();

    while (true) {
        if (choice == 1) {
            addTelephoneBook();
            break;
        } else if (choice == 2) {
            searchBook();
            break;
        } else if (choice == 3) {
            viewBook();
            break;
        } else if (choice == 4) {
            clearScreen();
            for (const auto &item: epLogo) {
                if (strcmp(item, "") == 0) {
                    break;
                }
                printf("%s", item);
            }

            printf("\n\n            Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nullam sit.\n");
            printf("                                ~ RedJacket 23-1 ~\n");
            sleep(1);
            exit(0);
        } else {
            printf("Invalid input\n");
            printf(">> ");
            scanf("%d", &choice);
            getchar();
        }
    }
}

void initLogo() {
    FILE *file = fopen("logo.txt", "r");
    int i = 0;
    // also read spaces
    while (i < 30) {
        fgets(epLogo[i], 1280000, file);
        i++;
    }
    fclose(file);
}

int main() {
    initLogo();
    while (true) {
        mainMenu();
    }
}