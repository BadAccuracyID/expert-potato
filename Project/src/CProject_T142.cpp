#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ncurses.h>
#include <unistd.h>
#include <string>

struct InGameItems {
    int potion = 0;
    int maxPotion = 0;
    int energyDrink = 0;
    int maxEnergyDrink = 0;
    int bomb = 0;
};

struct SpaceShip {
    char name[128]{};
    char spaceShip[10][10]{};

    int height = 5;
    int width = 5;
    int bulletOffset = 2;

    void construct(int aHeight, int aWidth, int aBulletOffset, const char *aName, char aSpaceShip[10][10]) {
        height = aHeight;
        width = aWidth;
        bulletOffset = aBulletOffset;
        strcpy(this->name, aName);
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                spaceShip[i][j] = aSpaceShip[i][j];
            }
        }
    }
};

SpaceShip playerShips[4];

struct LobbyPlayer {
    char name[128] = "Unknown";
    int x = 17;
    int y = 16;

    int level = 1;
    int xp = 0;
    int money = 0;

    int health = 100;
    int energy = 50;
    int armor = 1;

    InGameItems items;
    SpaceShip spaceShip = playerShips[0];
};

struct GamePlayer {
    char name[128] = "Unknown";
    int x = 17;
    int y = 16;

    int xpGained = 0;
    int moneyGained = 0;

    int score = 0;

    int bullets = 10;
    int maxBullets = 10;

    int health = 100;
    int maxHealth = 100;

    int energy = 50;
    int maxEnergy = 50;

    int armor = 1;

    InGameItems items;
    SpaceShip spaceShip;
};

void gamePlayerConstructor(GamePlayer *gamePlayer, LobbyPlayer lobbyPlayer) {
    strcpy(gamePlayer->name, lobbyPlayer.name);
    gamePlayer->health = lobbyPlayer.health;
    gamePlayer->maxHealth = lobbyPlayer.health;

    gamePlayer->energy = lobbyPlayer.energy;
    gamePlayer->maxEnergy = lobbyPlayer.energy;

    gamePlayer->armor = lobbyPlayer.armor;

    gamePlayer->items = lobbyPlayer.items;
    gamePlayer->spaceShip = lobbyPlayer.spaceShip;
}

void lobbyPlayerConstructor(LobbyPlayer *lobbyPlayer, GamePlayer gamePlayer) {
    lobbyPlayer->money += gamePlayer.moneyGained;
    lobbyPlayer->xp += gamePlayer.xpGained;
    lobbyPlayer->items = gamePlayer.items;

    // level up system
    while (lobbyPlayer->xp >= 100) {
        lobbyPlayer->level++;
        lobbyPlayer->xp -= 100;
    }
}

void clearScreen() {
    clear();
    refresh();
}

class Game {
private:

    struct EnemyShip {
        bool spawned = false;
        char spaceShip[10][10]{};

        int height = 1;
        int width = 3;

        int health = 1;
        int x = 0;
        int y = 0;

        void construct(int aHeight, int aWidth, char aSpaceShip[10][10]) {
            height = aHeight;
            width = aWidth;
            for (int i = 0; i < height; i++) {
                for (int j = 0; j < width; j++) {
                    spaceShip[i][j] = aSpaceShip[i][j];
                }
            }
        }

        void damage(int damage) {
            this->health -= damage;
        }
    };

    EnemyShip enemyShips[3];

    GamePlayer gamePlayer;
    LobbyPlayer *lobbyPlayer{};
    EnemyShip enemies[3200]{};

    char gameMap[22][49]{};
    char notification[128]{};
    bool doTick = true;

    int reloadTick = 0;
    bool isReloading = false;

    int energyTick = 0;

    int randSpawnTicks[6] = {2250000, 4500000, 6750000, 9000000, 11250000, 18000000};
    int randShootTicks[5] = {4500000, 9000000, 13500000, 18000000, 22500000};
    int spawnerTick = 0;
    int movementTick = 0;
    int shootTick = 0;

    int notificationTick = 0;

//    int xDir[4] = {1, -1, 0, 0};
//    int yDir[4] = {0, 0, -1, 0};

    int xDir[4] = {0, 1, -1, 0};
    int yDir[4] = {-1, 1, -1, 1};

    void printMap() {
        clearScreen();
        for (auto &it: gameMap) {
            for (char c: it) {
                if (c == '\0') {
                    printw(" ");
                } else {
                    printw("%c", c);
                }
            }
            printw("\n");
        }

        move(2, 51);
        printw("C Space Invader");
        move(3, 51);
        printw("%s", gamePlayer.spaceShip.name);

        move(9, 51);
        printw("Score: %d", gamePlayer.score);
        move(10, 51);
        printw("Bullets %d/%d", gamePlayer.bullets, gamePlayer.maxBullets);

        move(12, 51);
        printw("HP [%d/%d] | +%d Armor", gamePlayer.health, gamePlayer.maxHealth, gamePlayer.armor);
        move(13, 51);
        printw("Energy %d/%d", gamePlayer.energy, gamePlayer.maxEnergy);

        move(15, 51);
        printw("Inventory");
        move(16, 52);
        printw("Potion: %d", gamePlayer.items.potion);
        move(17, 52);
        printw("Max Potion: %d", gamePlayer.items.maxPotion);
        move(18, 52);
        printw("Energy Drink: %d", gamePlayer.items.energyDrink);
        move(19, 52);
        printw("Max Energy Drink: %d", gamePlayer.items.maxEnergyDrink);
        move(20, 52);
        printw("Bomb: %d", gamePlayer.items.bomb);


        // print da spaceship
        for (int i = 0; i < gamePlayer.spaceShip.height; i++) {
            move(gamePlayer.y + i, gamePlayer.x);
            printw("%s", gamePlayer.spaceShip.spaceShip[i]);
        }
//        debugPositions();
    }

    void debugPositions() {
        int i = 0;
        for (auto &enemy: enemies) {
            if (enemy.spawned) {
                move(30 + i, 0);
                printw("Enemy %d at Y:%d, X:%d", i, enemy.y, enemy.x);
                refresh();
                i++;
            }
        }
    }

    static bool isShipPart(char c) {
        return c == '\\' || c == '/' || c == 'o' || c == 'x' || c == 'v';
    }

    struct Bullet {
        enum Direction {
            UP, DOWN
        };

    private:
        int tick = 0;
        bool active = false;
        int yPos = 0;
        int xPos = 0;
        int damage = 1;

        Direction direction = UP;

    public:
        int shoot(int ayPos, int axPos, Direction aDirection) {
            this->yPos = ayPos;
            this->xPos = axPos;
            this->direction = aDirection;
            this->active = true;
            this->tick = 0;
            return 0;
        }

        int getYPos() const {
            return this->yPos;
        }

        int getXPos() const {
            return this->xPos;
        }

        int getDamage() const {
            return this->damage;
        };

        bool isActive() const {
            return this->active;
        }

        int doTick() {
            return this->tick++;
        }

        void resetTick() {
            this->tick = 0;
        }

        void setDamage(int aDamage) {
            this->damage = aDamage;
        }

        void move() {
            if (this->direction == UP) {
                this->yPos--;
            } else {
                this->yPos++;
            }
        }

        void stop() {
            this->active = false;
        }

        void checkMapBounds() {
            if (this->direction == UP) {
                if (this->yPos < 0) {
                    stop();
                }
            } else {
                if (this->yPos >= 21) {
                    stop();
                }
            }
        }
    };

    Bullet bullets[11]{};

    void doShoot() {
        for (auto &bullet: bullets) {
            if (!bullet.isActive()) {
                bullet.shoot(gamePlayer.y, gamePlayer.x + gamePlayer.spaceShip.bulletOffset, Bullet::Direction::UP);
                break;
            }
        }
    }

    void shootingTask() {
        for (auto &bullet: bullets) {
            if (!bullet.isActive()) {
                continue;
            }

            int tick = bullet.doTick();
            if (tick == 35000) {
                move(bullet.getYPos(), bullet.getXPos());
                printw(" ");
                refresh();

                bullet.move();
                bullet.resetTick();
            } else if (tick == 30000) {
                move(bullet.getYPos(), bullet.getXPos());
                printw("^");
                refresh();

                char hit = gameMap[bullet.getYPos()][bullet.getXPos()];
                if (isShipPart(hit)) {
                    onEnemyShot(bullet.getYPos(), bullet.getXPos(), hit);
                    bullet.stop();
                    continue;
                }
            } else {
                continue;
            }

            bullet.checkMapBounds();
        }
    }


    struct SkillBullet {

    private:
        int tick = 0;
        bool active = false;
        int yPos = 0;
        int xPos = 0;

        bool bullet1Hit = false;
        bool bullet2Hit = false;
        bool bullet3Hit = false;

    public:
        int shoot(int ayPos, int axPos) {
            this->yPos = ayPos;
            this->xPos = axPos;
            this->active = true;
            this->tick = 0;
            this->bullet1Hit = false;
            this->bullet2Hit = false;
            this->bullet3Hit = false;
            return 0;
        }

        int getYPos() const {
            return this->yPos;
        }

        int getXPos() const {
            return this->xPos;
        }

        bool isBullet1Hit() const {
            return this->bullet1Hit;
        }

        bool isBullet2Hit() const {
            return this->bullet2Hit;
        }

        bool isBullet3Hit() const {
            return this->bullet3Hit;
        }

        bool isActive() const {
            return this->active;
        }

        int doTick() {
            return this->tick++;
        }

        void onBullet1Hit() {
            this->bullet1Hit = true;
        }

        void onBullet2Hit() {
            this->bullet2Hit = true;
        }

        void onBullet3Hit() {
            this->bullet3Hit = true;
        }

        void resetTick() {
            this->tick = 0;
        }

        void move() {
            this->yPos--;
        }

        void stop() {
            this->active = false;
        }

        void checkMapBounds() {
            if (this->yPos < 0) {
                stop();
            }
        }
    };

    SkillBullet skillBullets[4]{};

    void doSkillShoot() {
        for (auto &skillBullet: skillBullets) {
            if (!skillBullet.isActive()) {
                skillBullet.shoot(gamePlayer.y, gamePlayer.x + gamePlayer.spaceShip.bulletOffset - 3);
                break;
            }
        }
    }

    void skillShootingTask() {
        for (auto &bullet: skillBullets) {
            if (!bullet.isActive()) {
                continue;
            }

            int tick = bullet.doTick();
            if (tick == 35000) {
                move(bullet.getYPos(), bullet.getXPos());
                printw("       ");
                refresh();

                bullet.move();
                bullet.resetTick();
            } else if (tick == 30000) {
                std::string bulletShape;

                char hit{};
                if (!bullet.isBullet1Hit()) {
                    hit = gameMap[bullet.getYPos()][bullet.getXPos()];
                    if (isShipPart(hit)) {
                        onEnemyShot(bullet.getYPos(), bullet.getXPos(), hit);
                        bullet.onBullet1Hit();
                    } else {
                        bulletShape.append("o");
                    }
                } else {
                    bulletShape.append(" ");
                }

                if (!bullet.isBullet2Hit()) {
                    hit = gameMap[bullet.getYPos()][bullet.getXPos() + 3];
                    if (isShipPart(hit)) {
                        onEnemyShot(bullet.getYPos(), bullet.getXPos() + 3, hit);
                        bullet.onBullet2Hit();
                    } else {
                        bulletShape.append("  o");
                    }
                } else {
                    bulletShape.append("   ");
                }

                if (!bullet.isBullet3Hit()) {
                    hit = gameMap[bullet.getYPos()][bullet.getXPos() + 6];
                    if (isShipPart(hit)) {
                        onEnemyShot(bullet.getYPos(), bullet.getXPos() + 6, hit);
                        bullet.onBullet3Hit();
                    } else {
                        bulletShape.append("  o");
                    }
                } else {
                    bulletShape.append("   ");
                }

                move(bullet.getYPos(), bullet.getXPos());
                printw("%s", bulletShape.c_str());
                refresh();
            } else {
                continue;
            }

            if (bullet.isBullet1Hit() && bullet.isBullet2Hit() && bullet.isBullet3Hit()) {
                bullet.stop();
                return;
            }

            bullet.checkMapBounds();
        }
    }

    void shoot() {
        if (isReloading) {
            postNotification("[!] Reloading...");
            return;
        }
        if (gamePlayer.bullets <= 0) {
            postNotification("[!] No bullets left!");
            return;
        }

        gamePlayer.bullets--;
        doShoot();
//        for (int i = gamePlayer.y; i >= 0; i--) {
//            move(i, gamePlayer.x + 2);
//            printw("^");
//            refresh();
//            usleep(20000);
//
//            move(i, gamePlayer.x + 2);
//            printw(" ");
//            refresh();
//
//            char hit = gameMap[i][gamePlayer.x + 2];
//            if (isShipPart(hit)) {
//                onEnemyShot(i, gamePlayer.x + 2, hit);
//                break;
//            }
//        }
    }

    void skillShot() {
        if (isReloading) {
            postNotification("[!] Reloading...");
            return;
        }
        if (gamePlayer.energy < 30) {
            postNotification("[!] Not enough energy!");
            return;
        }

        if (gamePlayer.bullets < 3) {
            postNotification("[!] Not enough bullets!");
            return;
        }

        gamePlayer.energy -= 30;
        gamePlayer.bullets -= 3;

        doSkillShoot();

//        bool isB1Hit = false;
//        bool isB2Hit = false;
//        bool isB3Hit = false;
//
//        for (int i = gamePlayer.y - 1; i >= 0; i--) {
//            std::string bulletShape;
//
//            char hit1 = gameMap[i][gamePlayer.x - 1];
//            if (!isB1Hit && isShipPart(hit1)) {
//                onEnemyShot(i, gamePlayer.x - 1, hit1);
//                isB1Hit = true;
//            } else {
//                if (!isB1Hit) {
//                    bulletShape.append("o");
//                }
//            }
//
//            char hit2 = gameMap[i][gamePlayer.x + 2];
//            if (!isB2Hit && isShipPart(hit2)) {
//                onEnemyShot(i, gamePlayer.x + 2, hit2);
//                isB2Hit = true;
//            } else {
//                if (!isB2Hit) {
//                    bulletShape.append("  o");
//                }
//            }
//
//            char hit3 = gameMap[i][gamePlayer.x + 5];
//            if (!isB3Hit && isShipPart(hit3)) {
//                onEnemyShot(i, gamePlayer.x + 5, hit3);
//                isB3Hit = true;
//            } else {
//                if (!isB3Hit) {
//                    bulletShape.append("  o");
//                }
//            }
//
//            move(i, gamePlayer.x - 1);
//            printw("%s", bulletShape.c_str());
//            refresh();
//            usleep(20000);
//
//            move(i, gamePlayer.x - 1);
//            printw("       ");
//            refresh();
//        }
    }

    enum EnemyShipPart {
        LEFT_WING,
        MID,
        RIGHT_WING
    };

    void onEnemyShot(int y, int x, char part) {
        onEnemyShot(y, x, part, false);
    }

    void onEnemyShot(int y, int x, char part, bool instaKill) {
        std::string msg;
        EnemyShipPart shipPart;
        switch (part) {
            case '\\':
                shipPart = LEFT_WING;
                msg.append("Hit at Left Wing ");
                break;
            case '/':
                shipPart = RIGHT_WING;
                msg.append("Hit at Right Wing ");
                break;
            default:
                shipPart = MID;
                msg.append("Hit at Mid ");
                break;
        }

        // locator
        EnemyShip *enemyShip;
        switch (shipPart) {
            case LEFT_WING: {
                break;
            }
            case MID: {
                x -= 1;
                break;
            }
            case RIGHT_WING: {
                x -= 2;
                break;
            }
        }

        bool found = false;
        for (auto &enemy: enemies) {
            if (enemy.x == x && enemy.y == y && enemy.spawned) {
                enemyShip = &enemy;
                found = true;
                break;
            }
        }

        if (!found) {
            // shouldn't happen, but ok...
            std::string errorMsg = "[!] ERROR! Enemy ship not found at ";
            errorMsg.append(std::to_string(x));
            errorMsg.append(", ");
            errorMsg.append(std::to_string(y));
            postNotification(errorMsg.c_str());
            return;
        }

        enemyShip->damage(1);
        msg.append("| Health at ");
        msg.append(std::to_string(enemyShip->health));
//        postNotification(msg.c_str());

        if (enemyShip->health <= 0) {
            // they dead bro
            gameMap[y][x] = ' ';
            gameMap[y][x + 1] = ' ';
            gameMap[y][x + 2] = ' ';
            printMap();

            enemyShip->spawned = false;
            enemyShip->y = 0;
            enemyShip->x = 0;
            enemyShip->health = 20;

            gamePlayer.score += 10;
            gamePlayer.moneyGained += 10;
        }
    }

    void consumePotion() {
        if (gamePlayer.items.potion <= 0) {
            postNotification("[!] No potions left!");
            return;
        }

        gamePlayer.items.potion--;
        gamePlayer.health += 50;
        if (gamePlayer.health > gamePlayer.maxHealth) {
            gamePlayer.health = gamePlayer.maxHealth;
        }
        postNotification("[!] Health restored! [+50]");
    }

    void consumeMaxPotion() {
        if (gamePlayer.items.maxPotion <= 0) {
            postNotification("[!] No max potions left!");
            return;
        }

        gamePlayer.items.maxPotion--;
        gamePlayer.health = gamePlayer.maxHealth;
        postNotification("[!] Health restored! [MAX]");
    }

    void consumeEnergyDrink() {
        if (gamePlayer.items.energyDrink <= 0) {
            postNotification("[!] No energy drinks left!");
            return;
        }

        gamePlayer.items.energyDrink--;
        gamePlayer.energy += 50;
        if (gamePlayer.energy > gamePlayer.maxEnergy) {
            gamePlayer.energy = gamePlayer.maxEnergy;
        }
        postNotification("[!] Energy restored! [+50]");
    }

    void consumeMaxEnergyDrink() {
        if (gamePlayer.items.maxEnergyDrink <= 0) {
            postNotification("[!] No max energy drinks left!");
            return;
        }

        gamePlayer.items.maxEnergyDrink--;
        gamePlayer.energy = gamePlayer.maxEnergy;
        postNotification("[!] Energy restored! [MAX]");
    }

    bool isCollidingWithPlayer(int x, int y) {
        bool colliding = false;
        int playerX = gamePlayer.x;
        int playerY = gamePlayer.y;
        int height = gamePlayer.spaceShip.height;
        int width = gamePlayer.spaceShip.width;

        if (x >= playerX && x < playerX + width) {
            if (y >= playerY && y < playerY + height) {
                colliding = true;
            }
        }

        return colliding;
    }

    bool isCollidingWithEnemy(int x, int y) {
        bool colliding = false;
        for (auto &enemy: enemies) {
            if (enemy.spawned) {
                int enemyX = enemy.x;
                int enemyY = enemy.y;

                if (y == enemyY && x == enemyX) {
                    colliding = true;
                    break;
                }
            }
        }

        return colliding;
    }

    void floodFill(int x, int y, char newChar, bool display) {
        if (x < 0 || x >= 49 || y < 0 || y >= 22) {
            return;
        }

        char c = gameMap[y][x];
        if (c == newChar) {
            return;
        }

        if (isShipPart(c)) {
            onEnemyShot(y, x, c, true);
        }

        if (isCollidingWithPlayer(x, y)) {
            return;
        }

        if (c == '|' || c == '_' || c == '-') {
            return;
        }

        gameMap[y][x] = newChar;
        printMap();
        if (display) {
            postNotification("[!] Bombs away!");
            refresh();
            usleep(2000);
        }

        for (int i = 0; i < 4; i++) {
            floodFill(x + xDir[i], y + yDir[i], newChar, display);
        }
    }

    void bomb() {
        if (gamePlayer.items.bomb <= 0) {
            postNotification("[!] No bombs left!");
            return;
        }

        postNotification("[!] Bombs away!");
        gamePlayer.items.bomb--;
        // flood-fill
        floodFill(gamePlayer.x + 2, gamePlayer.y - (gamePlayer.spaceShip.height / 2), '.', true);
        floodFill(gamePlayer.x + 2, gamePlayer.y - (gamePlayer.spaceShip.height / 2), ' ', false);
    }

    void spawnEnemy(EnemyShip enemyShip, int index) {
        int x;
        int y = 0;

        int tries = 0;
        bool occupied = false;
        do {
            x = rand() % 49;

            // check surroundings
            // -2, -1, 0, 1, 2, 3
            for (int i = -2; i <= 3; i++) {
                char c = gameMap[y][x + i];
                if (isShipPart(c) || c == '|') {
                    occupied = true;
                    break;
                }
            }
            if (!occupied) {
                occupied = isCollidingWithPlayer(x, y);
            }

            tries++;
        } while (occupied && tries <= 10);

        if (tries >= 10) {
            return;
        }

        enemyShip.spawned = true;
        enemyShip.y = y;
        enemyShip.x = x;

        enemies[index] = enemyShip;

        for (int i = 0; i < enemyShip.height; i++) {
            for (int j = 0; j < enemyShip.width; j++) {
                gameMap[y + i][x + j] = enemyShip.spaceShip[i][j];
            }
        }
        printMap();
    }

    void postNotification(const char *message) {
        notificationTick = 0;
        strcpy(notification, message);
    }

    void reload() {
        if (!isReloading) {
            isReloading = true;
            postNotification("[!] Reloading...");
        }
    }

    void doEnergyTask() {
        if (energyTick++ >= 4500000) {
            if (gamePlayer.energy < gamePlayer.maxEnergy) {
                gamePlayer.energy += 5;
                printMap();
            }
            energyTick = 0;
        }
    }

    void doReloadTask() {
        if (isReloading) {
            if (reloadTick++ >= 4500000) {
                gamePlayer.bullets = gamePlayer.maxBullets;

                reloadTick = 0;
                isReloading = false;
                postNotification("[!] Reloaded!");
                printMap();
            }
        }
    }

    void doEnemySpawnTask() {
        if (spawnerTick++ >= randSpawnTicks[rand() % 6]) {
            int index = 0;
            for (auto &enemy: enemies) {
                if (!enemy.spawned) {
                    break;
                }
                index++;
            }

            // random choose from enemyShips
            EnemyShip enemy = enemyShips[rand() % 3];

            spawnEnemy(enemy, index);
            spawnerTick = 0;
        }
    }

    void doEnemyMovementTask() {
        if (movementTick++ >= randSpawnTicks[rand() % 6]) {
            for (auto &enemy: enemies) {
                if (!enemy.spawned) {
                    continue;
                }

                if (enemy.y + 1 > 21) {
                    // they reached the bottom
                    gameMap[enemy.y][enemy.x] = ' ';
                    gameMap[enemy.y][enemy.x + 1] = ' ';
                    gameMap[enemy.y][enemy.x + 2] = ' ';
                    enemy.y++;
                    gameMap[enemy.y][enemy.x] = enemy.spaceShip[0][0];
                    gameMap[enemy.y][enemy.x + 1] = enemy.spaceShip[0][1];
                    gameMap[enemy.y][enemy.x + 2] = enemy.spaceShip[0][2];
                    endGame();
                    break;
                }

                // move down
                gameMap[enemy.y][enemy.x] = ' ';
                gameMap[enemy.y][enemy.x + 1] = ' ';
                gameMap[enemy.y][enemy.x + 2] = ' ';
                enemy.y++;

                if (isCollidingWithPlayer(enemy.x, enemy.y)) {
                    damagePlayerAndCheck(30);
                    onEnemyShot(enemy.y, enemy.x, enemy.spaceShip[0][0], false);
                    continue;
                } else if (isCollidingWithPlayer(enemy.x + 1, enemy.y)) {
                    damagePlayerAndCheck(30);
                    onEnemyShot(enemy.y, enemy.x + 1, enemy.spaceShip[0][1], false);
                    continue;
                } else if (isCollidingWithPlayer(enemy.x + 2, enemy.y)) {
                    damagePlayerAndCheck(30);
                    onEnemyShot(enemy.y, enemy.x + 2, enemy.spaceShip[0][2], false);
                    continue;
                }

                gameMap[enemy.y][enemy.x] = enemy.spaceShip[0][0];
                gameMap[enemy.y][enemy.x + 1] = enemy.spaceShip[0][1];
                gameMap[enemy.y][enemy.x + 2] = enemy.spaceShip[0][2];
                printMap();
            }

            movementTick = 0;
        }
    }

    Bullet enemyBullets[32]{};

    void doShootEnemy(EnemyShip *randomEnemy) {
        for (auto &bullet: enemyBullets) {
            if (!bullet.isActive()) {
                bullet.shoot(randomEnemy->y + 2, randomEnemy->x + 1, Bullet::Direction::DOWN);

                char c = randomEnemy->spaceShip[0][1];
                if (c == 'v') {
                    bullet.setDamage(10);
                } else if (c == 'x') {
                    bullet.setDamage(20);
                } else {
                    bullet.setDamage(25);
                }
                break;
            }
        }

//        pthread_create(&threads[tC], nullptr, [](void *arg) -> void * {
//            auto *enemy = (EnemyShip *) arg;
//            for (int i = enemy->y + 1; i < 21; i++) {
//                int x = enemy->x;
//
//                move(i, x + 1);
//                printw("*");
//                refresh();
//                usleep(100000);
//
//                move(i, x + 1);
//                printw(" ");
//                refresh();
//            }
//            return nullptr;
//        }, randomEnemy);
//        pthread_exit(nullptr);
//        tC++;

//        std::thread thread([this](EnemyShip *randomEnemy) {
//            int x = randomEnemy->x;
//            for (int i = randomEnemy->y + 1; i < 21; i++) {
//            gameMap[i][x + 1] = '*';
//            printMap();
//            refresh();
//            usleep(1000);
//
//            gameMap[i][x + 1] = ' ';
//            printMap();
//            refresh();
//                move(i, x + 1);
//                printw("*");
//                refresh();
//                usleep(10000);
//
//                move(i, x + 1);
//                printw(" ");
//                refresh();
//            }
//        }, randomEnemy);
    }

    void enemyShootingTask() {
        for (auto &bullet: enemyBullets) {
            if (!bullet.isActive()) {
                continue;
            }

            int tick = bullet.doTick();
            if (tick == 155000) {
                move(bullet.getYPos(), bullet.getXPos());
                printw(" ");
                refresh();

                bullet.move();
                bullet.resetTick();
            } else if (tick == 150000) {
                move(bullet.getYPos(), bullet.getXPos());
                printw("*");
                refresh();

                if (isCollidingWithPlayer(bullet.getXPos(), bullet.getYPos())) {
                    bullet.stop();
                    int damage = 10;

                    damagePlayerAndCheck(bullet.getDamage());
                    continue;
                }
            } else {
                continue;
            }

            bullet.checkMapBounds();
        }
    }

    void damagePlayerAndCheck(int damage) {
        int aDamage = damage;

        if (gamePlayer.armor > 0) {
            if (gamePlayer.armor - aDamage <= 0) {
                aDamage -= gamePlayer.armor;
                gamePlayer.armor = 0;
            } else {
                aDamage -= gamePlayer.armor;
                gamePlayer.armor -= damage;

                return;
            }
        }

        gamePlayer.health -= aDamage;
        printMap();
        if (gamePlayer.health <= 0) {
            endGame();
        }
    }

    void doEnemyShootTask() {
        if (shootTick++ >= randShootTicks[rand() % 5]) {
            EnemyShip *enemiesCopy[3200]{};

            int i = 0;
            for (auto &enemy: enemies) {
                if (!enemy.spawned) {
                    continue;
                }

                enemiesCopy[i] = &enemy;
                i++;
            }

            if (i > 0) {
                // select random enemy
                for (int j = 0; j < i; ++j) { //<---- Sounds good, but too heavy bcs i suck
                    EnemyShip *randomEnemy = enemiesCopy[rand() % (i)];
                    doShootEnemy(randomEnemy);
                }
            }


            shootTick = 0;
        }
    }

    int collisionTick = 0;

    void doCollisionTask() {
        if (collisionTick++ >= 9000000) {
            for (auto &item: enemies) {
                if (item.spawned) {
                    // check collision with player
                    if (isCollidingWithPlayer(item.x, item.y)) {
                        damagePlayerAndCheck(30);
                        onEnemyShot(item.y, item.x, item.spaceShip[0][0], false);
                        continue;
                    } else if (isCollidingWithPlayer(item.x + 1, item.y)) {
                        damagePlayerAndCheck(30);
                        onEnemyShot(item.y, item.x + 1, item.spaceShip[0][1], false);
                        continue;
                    } else if (isCollidingWithPlayer(item.x + 2, item.y)) {
                        damagePlayerAndCheck(30);
                        onEnemyShot(item.y, item.x + 2, item.spaceShip[0][2], false);
                        continue;
                    }
                }
            }

            collisionTick = 0;
        }
    }

    void doNotificationTask() {
        notificationTick++;

        if (notification[0] != '\0' && notificationTick % 10000 == 0) {
            move(6, 51);
            printw("%s", notification);

            if (notificationTick == 9000000) {
                for (auto &it: notification) {
                    notification[it] = '\0';
                }

                notificationTick = 0;
                return;
            }
        }
    }

    void onKeyPress(char key) {
        switch (key) {
            case 'w': {
                char c = gameMap[gamePlayer.y - 1][gamePlayer.x];
                if (c == ' ' && gamePlayer.y > 0) {
                    gamePlayer.y--;
                }
                break;
            }
            case 'a': {
                char c = gameMap[gamePlayer.y][gamePlayer.x - 1];
                if (c == ' ' && gamePlayer.x > 0) {
                    gamePlayer.x--;
                    // check collision with enemy
                    if (isCollidingWithEnemy(gamePlayer.x, gamePlayer.y)) {
                        damagePlayerAndCheck(10);
                    }
                }
                break;
            }
            case 's': {
                char c = gameMap[gamePlayer.y + (gamePlayer.spaceShip.height - 1) + 1][gamePlayer.x];
                if (c == ' ' && gamePlayer.y < 21) {
                    gamePlayer.y++;
                }
                break;
            }
            case 'd': {
                char c = gameMap[gamePlayer.y][gamePlayer.x + (gamePlayer.spaceShip.width - 1) + 1];
                if (c == ' ' && gamePlayer.x < 48) {
                    gamePlayer.x++;
                }
                break;
            }
            case ' ': {
                shoot();
                break;
            }
            case 'f': {
                skillShot();
                break;
            }
            case 'r': {
                reload();
                break;
            }
            case '1': {
                consumePotion();
                break;
            }
            case '2': {
                consumeMaxPotion();
                break;
            }
            case '3': {
                consumeEnergyDrink();
                break;
            }
            case '4': {
                consumeMaxEnergyDrink();
                break;
            }
            case 'g': {
                bomb();
                break;
            }
        }
    }

    bool confirmExit() {
        clearScreen();
        echo();
        nodelay(stdscr, FALSE);

        printw("Are you sure you want to exit? [y/n]\n");
        printw(">> ");
        refresh();

        bool val;

        char input;
        while (true) {
            scanw("%c", &input);
            if (input == 'y') {
                val = true;
                break;
            } else if (input == 'n') {
                noecho();
                val = false;
                break;
            }
        }

        return val;
    }

    void endGame() {
        nodelay(stdscr, FALSE);

        clearScreen();

        printw("C Space Invader\n");
        printw("=================================\n\n");
        printw("Game Over!\n");
        printw("Score: %d\n", gamePlayer.score);
        printw("Money: %d\n", gamePlayer.moneyGained);

        gamePlayer.xpGained = gamePlayer.score * 3 / 4;
        printw("You gained %d exp\n\n", gamePlayer.xpGained);

        int levelUp = 0;
        int tempXp = gamePlayer.xpGained;
        while (tempXp >= 100) {
            levelUp++;
            tempXp -= 100;
        }

        if (levelUp > 0) {
            printw("You leveled up %d times!\n", levelUp);
            printw("You are now level %d\n", lobbyPlayer->level + levelUp);
        }

        lobbyPlayerConstructor(lobbyPlayer, gamePlayer);
        saveScore();

        printw("Press any key to exit...\n");

        refresh();
        doTick = false;

        getch();
    }

    void saveScore() {
        FILE *file = fopen("database/score.dat", "a");
        fprintf(file, "%s#%d\n", gamePlayer.name, gamePlayer.score);
        fclose(file);
    }

    void tick() {
        doTick = true;
        nodelay(stdscr, TRUE);

        while (doTick) {
            doEnergyTask();
            doReloadTask();
            doEnemySpawnTask();
            doEnemyMovementTask();
            doEnemyShootTask();
            doCollisionTask();
            doNotificationTask();

            shootingTask();
            skillShootingTask();
            enemyShootingTask();

            char c = tolower(getch());
            switch (c) {
                case 'q': {
                    if (confirmExit()) {
                        endGame();
                        return;
                    }

                    nodelay(stdscr, TRUE);
                    printMap();
                    break;
                }

                case 'w':
                case 'a':
                case 's':
                case 'd':
                case 'r':
                case 'f':
                case '1':
                case '2':
                case '3':
                case '4':
                case 'g':
                case ' ': {
                    onKeyPress(c);
                    printMap();
                    break;
                }

                default: {
                    break;
                }
            }
        }
    }

    void loadShips() {
        EnemyShip enemyShip;
        char enemyShip1[10][10] = {
                "\\v/"
        };
        enemyShip.construct(1, 3, enemyShip1);
        enemyShips[0] = enemyShip;

        EnemyShip enemyShip2;
        char enemyShip2_1[10][10] = {
                "\\x/"
        };
        enemyShip2.construct(1, 3, enemyShip2_1);
        enemyShips[1] = enemyShip2;

        EnemyShip enemyShip3;
        char enemyShip3_1[10][10] = {
                "\\o/"
        };
        enemyShip3.construct(1, 3, enemyShip3_1);
        enemyShips[2] = enemyShip3;
    }

    void readFromFile() {
        FILE *file = fopen("assets/game.txt", "r");

        // also read spaces
        int i = 0;
        while (fgets(gameMap[i], 22 * 49, file) != nullptr) {
            i++;
        }
        fclose(file);
    }

public:

    void init(LobbyPlayer *aLobbyPlayer) {
        lobbyPlayer = aLobbyPlayer;
        gamePlayerConstructor(&gamePlayer, *aLobbyPlayer);

        noecho();

        loadShips();
        readFromFile();
        printMap();
        tick();
    }

};

class Lobby {
private:
    LobbyPlayer lobbyPlayer;
    bool doTick = true;
    char lobbyMap[22][34]{};
    char cheatSequence15[20]{};
    char cheatSequence11[20]{};

    enum Area {
        SPACESHIP,
        ARENA,
        UPGRADE,
        ITEM,
        LEADERBOARD,
        SAVE,
        NONE
    };

    struct LeaderboardPlayer {
        char name[128] = "Unknown";
        int score = 0;
    };

    Area checkPosition() const {
        if (lobbyPlayer.y == 7) {
            if (lobbyPlayer.x >= 6 && lobbyPlayer.x <= 9) {
                if (lobbyPlayer.level < 10) {
                    move(14, 38);
                    printw("You need to be level 10 to unlock spaceship!");
                    return NONE;
                }
            }
        }

        if (lobbyPlayer.y >= 8 && lobbyPlayer.y <= 12) {
            if (lobbyPlayer.x >= 13 && lobbyPlayer.x <= 21) {
                move(14, 38);
                printw("Press 'SPACE' to go to game arena.");
                return ARENA;
            }
        }

        if (lobbyPlayer.y >= 3 && lobbyPlayer.y <= 5) {
            if (lobbyPlayer.x >= 25 && lobbyPlayer.x <= 27) {
                move(14, 38);
                printw("Press 'SPACE' to go select spaceship.");
                return SPACESHIP;
            }
        }

        if (lobbyPlayer.y >= 15 && lobbyPlayer.y <= 17) {
            if (lobbyPlayer.x >= 4 && lobbyPlayer.x <= 8) {
                move(11, 38);
                printw("Hohoo! This is an upgrade shop");
                move(12, 38);
                printw("Anything I can do for you?");
                move(14, 38);
                printw("Press 'SPACE' to interact.");
                return UPGRADE;
            } else if (lobbyPlayer.x >= 26 && lobbyPlayer.x <= 28) {
                move(11, 38);
                printw("Welcome to item shop!");
                move(12, 38);
                printw("Do you want to buy any items?");
                move(14, 38);
                printw("Press 'SPACE' to interact.");
                return ITEM;
            }
        }

        if (lobbyPlayer.y >= 19 && lobbyPlayer.y <= 20) {
            if (lobbyPlayer.x >= 15 && lobbyPlayer.x <= 20) {
                move(14, 38);
                printw("Press 'SPACE' to save your data.");
                return SAVE;
            }
        }

        if (lobbyPlayer.x == 5) {
            if (lobbyPlayer.y >= 9 && lobbyPlayer.y <= 11) {
                move(14, 38);
                printw("Press 'SPACE' to open the leaderboards.");
                return LEADERBOARD;
            }
        }

        return NONE;
    }

    void printMap() {
        clearScreen();
        for (auto &it: lobbyMap) {
            for (char c: it) {
                if (c == '\0') {
                    printw(" ");
                } else {
                    printw("%c", c);
                }
            }
            printw("\n");
        }

        move(3, 38);
        printw("Name: %s", lobbyPlayer.name);
        move(4, 38);
        printw("Level: %d", lobbyPlayer.level);
        move(5, 38);
        printw("XP: %d", lobbyPlayer.xp);
        move(6, 38);
        printw("Money: %d", lobbyPlayer.money);

        checkPosition();

        move(lobbyPlayer.y, lobbyPlayer.x);
        printw("P");
    }

    void printBackpack() {
        clearScreen();
        printw("%s's backpack\n", lobbyPlayer.name);
        printw("=====================\n");
        printw("- %-16s : %d\n", "Potion", lobbyPlayer.items.potion);
        printw("- %-16s : %d\n", "Max Potion", lobbyPlayer.items.maxPotion);
        printw("- %-16s : %d\n", "Energy Drink", lobbyPlayer.items.energyDrink);
        printw("- %-16s : %d\n", "Max Energy Drink", lobbyPlayer.items.maxEnergyDrink);
        printw("- %-16s : %d\n", "Bomb", lobbyPlayer.items.bomb);

        printw("\n\nBack to game [press any key]");
        getch();
    }

    void printLeaderboard() {
        clearScreen();

        printw("C Space Invader Scoreboard\n");
        printw("=================================\n\n");

        LeaderboardPlayer leaderboardPlayers[128];

        FILE *file = fopen("database/score.dat", "r");
        // format: name#score
        char nameBuffer[128];
        int scoreBuffer;
        int i = 0;
        while (fscanf(file, "%[^#]#%d\n", nameBuffer, &scoreBuffer) != EOF) {
            strcpy(leaderboardPlayers[i].name, nameBuffer);
            leaderboardPlayers[i].score = scoreBuffer;
            i++;
        }
        fclose(file);

        // sort by score
        for (auto &leaderboardPlayer: leaderboardPlayers) {
            for (auto &k: leaderboardPlayers) {
                if (leaderboardPlayer.score > k.score) {
                    LeaderboardPlayer temp = leaderboardPlayer;
                    leaderboardPlayer = k;
                    k = temp;
                }
            }
        }

        int placement = 1;
        for (auto &leaderboardPlayer: leaderboardPlayers) {
            if (leaderboardPlayer.score == 0) {
                break;
            }
            printw("%d. %s - %d\n", placement, leaderboardPlayer.name, leaderboardPlayer.score);
            placement++;
        }
        refresh();

        printw("\n\nBack to game [press any key]");
        getch();
    }

    void openUpgradeShop() {
        bool loop = true;
        int selection = 0;

        char message[128]{};

        while (loop) {
            clearScreen();
            printw("Do you need anything to upgrade?\n");
            printw("=================================\n");

            printw("%-1d. %-7s %-3d / %-3d - $%-2d", 1, "Health", lobbyPlayer.health, 300, 50);
            selection == 0 ? printw("\t<\n") : printw("\n");
            printw("%-1d. %-7s %-3d / %-3d - $%-2d", 2, "Energy", lobbyPlayer.energy, 500, 50);
            selection == 1 ? printw("\t<\n") : printw("\n");
            printw("%-1d. %-7s %-3d / %-3d - $%-2d", 3, "Armor", lobbyPlayer.armor, 30, 30);
            selection == 2 ? printw("\t<\n") : printw("\n");

            printw("\n4. Back");
            selection == 3 ? printw("\t\t\t\t<\n") : printw("\n");

            printw("\n%s", message);

            char c = tolower(getch());
            switch (c) {
                case 'w':
                case 'a': {
                    if (selection > 0) {
                        selection--;
                    } else {
                        selection = 3;
                    }
                    break;
                }
                case 's':
                case 'd': {
                    if (selection < 3) {
                        selection++;
                    } else {
                        selection = 0;
                    }
                    break;
                }
                case ' ': {
                    if (selection == 0) {
                        if (lobbyPlayer.money < 50) {
                            strcpy(message, "You don't have enough money!");
                            break;
                        }

                        if (lobbyPlayer.health + 1 > 300) {
                            strcpy(message, "You can't upgrade your health more than 300!");
                            break;
                        }

                        lobbyPlayer.health += 1;
                        lobbyPlayer.money -= 50;
                        strcpy(message, "You have upgraded your health!");
                    } else if (selection == 1) {
                        if (lobbyPlayer.money < 50) {
                            strcpy(message, "You don't have enough money!");
                            break;
                        }

                        if (lobbyPlayer.energy + 1 > 500) {
                            strcpy(message, "You can't upgrade your energy more than 500!");
                            break;
                        }

                        lobbyPlayer.energy += 1;
                        lobbyPlayer.money -= 50;
                        strcpy(message, "You have upgraded your energy!");
                    } else if (selection == 2) {
                        if (lobbyPlayer.money < 30) {
                            strcpy(message, "You don't have enough money!");
                            break;
                        }

                        if (lobbyPlayer.armor + 1 > 30) {
                            strcpy(message, "You can't upgrade your armor more than 30!");
                            break;
                        }

                        lobbyPlayer.armor += 1;
                        lobbyPlayer.money -= 30;
                        strcpy(message, "You have upgraded your armor!");
                    } else if (selection == 3) {
                        loop = false;
                    }
                    break;
                }
            }
        }
    }

    void openItemShop() {
        bool loop = true;
        int selection = 0;

        char message[128]{};

        while (loop) {
            clearScreen();
            printw("Welcome to the item shop!\n");
            printw("=================================\n");

            printw("%-1d. %-16s [$%-2d]", 1, "Potion", 5);
            selection == 0 ? printw("\t<\n") : printw("\n");
            printw("%-1d. %-16s [$%-2d]", 2, "Max Potion", 20);
            selection == 1 ? printw("\t<\n") : printw("\n");
            printw("%-1d. %-16s [$%-2d]", 3, "Energy Drink", 3);
            selection == 2 ? printw("\t<\n") : printw("\n");
            printw("%-1d. %-16s [$%-2d]", 4, "Max Energy Drink", 15);
            selection == 3 ? printw("\t<\n") : printw("\n");
            printw("%-1d. %-16s [$%-2d]", 5, "Bomb", 10);
            selection == 4 ? printw("\t<\n") : printw("\n");

            printw("\n6. Back");
            selection == 5 ? printw("\t\t\t\t<\n") : printw("\n");

            printw("\n%s", message);
            refresh();
            if (message[0] != '\0') {
                refresh();
                sleep(1);

                // clear message
                for (const auto &item: message) {
                    message[item] = '\0';
                }
            }

            char c = tolower(getch());
            switch (c) {
                case 'w':
                case 'a': {
                    if (selection > 0) {
                        selection--;
                    } else {
                        selection = 5;
                    }
                    break;
                }
                case 's':
                case 'd': {
                    if (selection < 5) {
                        selection++;
                    } else {
                        selection = 0;
                    }
                    break;
                }
                case ' ': {
                    echo();
                    if (selection == 0) {
                        printw("How many potions do you want to buy?\n");
                        printw("> ");

                        int amount = 0;
                        scanw("%d", &amount);
                        refresh();

                        if (amount <= 0) {
                            strcpy(message, "You can't buy 0 or less potions!");
                            break;
                        }

                        int price = amount * 5;
                        if (lobbyPlayer.money < price) {
                            strcpy(message, "You don't have enough money!");
                            break;
                        }

                        lobbyPlayer.items.potion += amount;
                        lobbyPlayer.money -= price;
                        std::string string;
                        string.append("Successfully bought ");
                        string.append(std::to_string(amount));
                        string.append(" potion(s)!");

                        strcpy(message, string.c_str());
                    } else if (selection == 1) {
                        printw("How many max potions do you want to buy?\n");
                        printw("> ");

                        int amount = 0;
                        scanw("%d", &amount);
                        refresh();

                        if (amount <= 0) {
                            strcpy(message, "You can't buy 0 or less max potions!");
                            break;
                        }

                        int price = amount * 20;
                        if (lobbyPlayer.money < price) {
                            strcpy(message, "You don't have enough money!");
                            break;
                        }

                        lobbyPlayer.items.maxPotion += amount;
                        lobbyPlayer.money -= price;
                        std::string string;
                        string.append("Successfully bought ");
                        string.append(std::to_string(amount));
                        string.append(" max potion(s)!");

                        strcpy(message, string.c_str());
                    } else if (selection == 2) {
                        printw("How many energy drinks do you want to buy?\n");
                        printw("> ");

                        int amount = 0;
                        scanw("%d", &amount);
                        refresh();

                        if (amount <= 0) {
                            strcpy(message, "You can't buy 0 or less energy drinks!");
                            break;
                        }

                        int price = amount * 3;
                        if (lobbyPlayer.money < price) {
                            strcpy(message, "You don't have enough money!");
                            break;
                        }

                        lobbyPlayer.items.energyDrink += amount;
                        lobbyPlayer.money -= price;
                        std::string string;
                        string.append("Successfully bought ");
                        string.append(std::to_string(amount));
                        string.append(" energy drink(s)!");

                        strcpy(message, string.c_str());
                    } else if (selection == 3) {
                        printw("How many max energy drinks do you want to buy?\n");
                        printw("> ");

                        int amount = 0;
                        scanw("%d", &amount);
                        refresh();

                        if (amount <= 0) {
                            strcpy(message, "You can't buy 0 or less max energy drinks!");
                            break;
                        }

                        int price = amount * 15;
                        if (lobbyPlayer.money < price) {
                            strcpy(message, "You don't have enough money!");
                            break;
                        }

                        lobbyPlayer.items.maxEnergyDrink += amount;
                        lobbyPlayer.money -= price;
                        std::string string;
                        string.append("Successfully bought ");
                        string.append(std::to_string(amount));
                        string.append(" max energy drink(s)!");

                        strcpy(message, string.c_str());
                    } else if (selection == 4) {
                        printw("How many bombs do you want to buy?\n");
                        printw("> ");

                        int amount = 0;
                        scanw("%d", &amount);
                        refresh();

                        if (amount <= 0) {
                            strcpy(message, "You can't buy 0 or less bombs!");
                            break;
                        }

                        int price = amount * 10;
                        if (lobbyPlayer.money < price) {
                            strcpy(message, "You don't have enough money!");
                            break;
                        }

                        lobbyPlayer.items.bomb += amount;
                        lobbyPlayer.money -= price;
                        std::string string;
                        string.append("Successfully bought ");
                        string.append(std::to_string(amount));
                        string.append(" bomb(s)!");

                        strcpy(message, string.c_str());
                    } else if (selection == 5) {
                        loop = false;
                    }
                    refresh();
                    break;
                }
            }
        }
    }

    void openSpaceShipShop() {
        bool loop = true;
        int selection = 0;

        SpaceShip selectedShip;
        selectedShip = lobbyPlayer.spaceShip;
        char message[128]{};

        while (loop) {
            clearScreen();
            printw("Welcome to the station!\n");
            printw(" Select your spaceship!\n\n");

            // spaceship name
            printw("   %s\n\n", selectedShip.name);
            // clear da spaceship
            for (int i = 0; i < 5; i++) {
                move(5 + i, 7);
                printw(" ");
            }
            // print da spaceship
            for (int i = 0; i < selectedShip.height; i++) {
                move(5 + i, 7);
                printw("%s", selectedShip.spaceShip[i]);
            }


            move(11, 0);
            printw("   [press 'q' to exit]   \n");
            printw(" [press SPACE to choose] \n");
            printw("< --------------------- >");

            char c = tolower(getch());
            switch (c) {
                case 'w':
                case 'a': {
                    if (selection > 0) {
                        selection--;
                    } else {
                        selection = 3;
                    }

                    selectedShip = playerShips[selection];
                    break;
                }
                case 's':
                case 'd': {
                    if (selection < 3) {
                        selection++;
                    } else {
                        selection = 0;
                    }

                    selectedShip = playerShips[selection];
                    break;
                }
                case ' ':
                case '\n': {
                    lobbyPlayer.spaceShip = selectedShip;
                    loop = false;
                    break;
                }
                case 'q': {
                    loop = false;
                    break;
                }
            }
        }

    }

    void saveData() {
        clearScreen();

        LobbyPlayer loadedPlayers[128];

        FILE *file = fopen("database/player.dat", "r");
        // name#money#xp#level#health#energy#armor#damage
        char bufferedName[128];
        int bufferedMoney;
        int bufferedXp;
        int bufferedLevel;
        int bufferedHealth;
        int bufferedEnergy;
        int bufferedArmor;
        int bufferedDamage;
        int i = 0;
        while (fscanf(file, "%[^#]#%d#%d#%d#%d#%d#%d#%d\n", bufferedName, &bufferedMoney, &bufferedXp, &bufferedLevel,
                      &bufferedHealth, &bufferedEnergy, &bufferedArmor, &bufferedDamage) != EOF) {
            strcpy(loadedPlayers[i].name, bufferedName);
            loadedPlayers[i].money = bufferedMoney;
            loadedPlayers[i].xp = bufferedXp;
            loadedPlayers[i].level = bufferedLevel;
            loadedPlayers[i].health = bufferedHealth;
            loadedPlayers[i].energy = bufferedEnergy;
            loadedPlayers[i].armor = bufferedArmor;
            i++;
        }
        fclose(file);

        file = fopen("database/player.dat", "w");
        for (auto &loadedPlayer: loadedPlayers) {
            if (strcmp(loadedPlayer.name, "Unknown") == 0) {
                break;
            }

            if (strcmp(loadedPlayer.name, lobbyPlayer.name) == 0) {
                fprintf(file, "%s#%d#%d#%d#%d#%d#%d#%d\n", lobbyPlayer.name, lobbyPlayer.money, lobbyPlayer.xp,
                        lobbyPlayer.level, lobbyPlayer.health, lobbyPlayer.energy, lobbyPlayer.armor, 1);
            } else {
                fprintf(file, "%s#%d#%d#%d#%d#%d#%d#%d\n", loadedPlayer.name, loadedPlayer.money, loadedPlayer.xp,
                        loadedPlayer.level, loadedPlayer.health, loadedPlayer.energy, loadedPlayer.armor, 1);
            }
        }
        fclose(file);

        printw("Data saved!\n");
        printw("Press any key to continue...");
        refresh();
        getch();
    }

    void interact() {
        Area area = checkPosition();
        switch (area) {
            case LEADERBOARD: {
                printLeaderboard();
                break;
            }
            case UPGRADE: {
                openUpgradeShop();
                break;
            }
            case ITEM: {
                openItemShop();
                break;
            }
            case SAVE: {
                saveData();
                break;
            }
            case SPACESHIP: {
                openSpaceShipShop();
                break;
            }
            case ARENA: {
                Game game;
                game.init(&lobbyPlayer);
                noecho();
                printMap();
                break;
            }
        }
    }

    void cheatCode(char c) {
        // put tail to cheatSequence15
        for (int i = 0; i < 15; i++) {
            cheatSequence15[i] = cheatSequence15[i + 1];
        }
        cheatSequence15[15] = c;

        // put tail to cheatSequence11
        for (int i = 0; i < 11; i++) {
            cheatSequence11[i] = cheatSequence11[i + 1];
        }
        cheatSequence11[11] = c;

        bool activated = false;
        if (strcasecmp(cheatSequence15, "daswdaswdaswdasw") == 0) {
            activated = true;
            lobbyPlayer.level = 100;
        } else if (strcasecmp(cheatSequence15, "wasdwasdwasdwasd") == 0) {
            activated = true;
            lobbyPlayer.items.bomb += 99;
        } else if (strcasecmp(cheatSequence11, "wswswsadadad") == 0) {
            activated = true;
            lobbyPlayer.health = 300;
            lobbyPlayer.energy = 500;
            lobbyPlayer.armor = 30;
        } else if (strcasecmp(cheatSequence11, "asdasdasdasd") == 0) {
            activated = true;
            // all item except bomb
            lobbyPlayer.items.potion += 99;
            lobbyPlayer.items.maxPotion += 99;
            lobbyPlayer.items.energyDrink += 99;
            lobbyPlayer.items.maxEnergyDrink += 99;
        }

        if (activated) {
            move(16, 38);
            printw("Cheat code activated!\n");
            refresh();
            sleep(1);

            for (int i = 0; i < 16; i++) {
                cheatSequence15[i] = '\0';
            }
        }
    }

    void onKeyPress(char key) {
        switch (key) {
            case 'w': {
                cheatCode(key);
                char c = lobbyMap[lobbyPlayer.y - 1][lobbyPlayer.x];
                if (c == ' ') {
                    lobbyPlayer.y--;
                }
                break;
            }
            case 'a': {
                cheatCode(key);
                char c = lobbyMap[lobbyPlayer.y][lobbyPlayer.x - 1];
                if (c == ' ') {
                    lobbyPlayer.x--;
                }
                break;
            }
            case 's': {
                cheatCode(key);
                char c = lobbyMap[lobbyPlayer.y + 1][lobbyPlayer.x];
                if (c == ' ') {
                    lobbyPlayer.y++;
                }
                break;
            }
            case 'd': {
                cheatCode(key);
                char c = lobbyMap[lobbyPlayer.y][lobbyPlayer.x + 1];
                if (c == ' ') {
                    lobbyPlayer.x++;
                }
                break;
            }
            case 'o': {
                printBackpack();
                break;
            }
            case '\n' :
            case ' ' : {
                interact();
                break;
            }
        }
    }

    bool confirmExit() {
        clearScreen();
        echo();

        printw("Are you sure you want to exit? [y/n]\n");
        printw("All unsaved data will be lost!\n\n");
        printw(">> ");
        refresh();

        bool val;

        char input;
        while (true) {
            scanw("%c", &input);
            if (input == 'y') {
                val = true;
                break;
            } else if (input == 'n') {
                noecho();
                val = false;
                break;
            }
        }

        return val;
    }

    void tick() {
        doTick = true;
        while (doTick) {
            char c = tolower(getch());
            switch (c) {
                case 'q': {
                    if (confirmExit()) {
                        endwin();
                        doTick = false;
                        return;
                    }

                    printMap();
                    break;
                }

                case 'w':
                case 'a':
                case 's':
                case 'd':
                case 'o':
                case ' ':
                case '\n': {
                    onKeyPress(c);
                    printMap();
                    break;
                }
            }
        }
    }

    void readFromFile(bool doorClosed) {
        FILE *file;
        file = fopen("assets/lobby.txt", "r");

        // also read spaces
        int i = 0;
        while (fgets(lobbyMap[i], 22 * 34, file) != nullptr) {
            i++;
        }
        fclose(file);

        if (doorClosed) {
            lobbyMap[6][6] = '-';
            lobbyMap[6][7] = '-';
            lobbyMap[6][8] = '-';
            lobbyMap[6][9] = '-';
        } else {
            lobbyMap[7][5] = '/';
            lobbyMap[7][10] = '\\';
        }

        lobbyMap[4][26] = 'W';

        lobbyMap[16][6] = 'U';
        lobbyMap[16][27] = 'I';
    }


public:
    void init(char name[128], int money, int xp, int level, int health, int energy, int armor) {
        strcpy(lobbyPlayer.name, name);
        lobbyPlayer.money = money;
        lobbyPlayer.xp = xp;
        lobbyPlayer.level = level;
        lobbyPlayer.health = health;
        lobbyPlayer.energy = energy;
        lobbyPlayer.armor = armor;

        lobbyPlayer.x = 17;
        lobbyPlayer.y = 16;

        noecho();

        readFromFile(level < 10);
        printMap();
        tick();
    }
};

class Menu {
private:
    int menuSelection = 0;
    bool choosingPlayer = false;
    Lobby lobby;
    LobbyPlayer loadedPlayers[128];
    char epLogo[22][80]{};

    void newGame() {
        clearScreen();
        echo();

        char name[128];

        bool validate = true;
        while (validate) {
            printw("Enter your name [> 3 characters | type 'cancel' to cancel]: ");
            scanw("%[^\n]", name);

            if (strcmp(name, "cancel") == 0) {
                return;
            } else if (strlen(name) < 3) {
                printw("Name must be at least 3 characters long!\n");
                refresh();
                sleep(1);
                clearScreen();
            } else {
                validate = false;

                if (strcasecmp(name, "admin") == 0) {
                    lobby.init(name, 1000000, 1000000, 100, 300, 500, 30);
                } else if (strcasecmp(name, "cancel") == 0) {
                    return;
                }
            }
        }




        // check save file
        FILE *file = fopen("database/player.dat", "r");
        bool found = false;
        char bufferedName[128];
        int bufferedMoney;
        int bufferedXp;
        int bufferedLevel;
        int bufferedHealth;
        int bufferedEnergy;
        int bufferedArmor;
        int bufferedDamage;

        while (fscanf(file, "%[^#]#%d#%d#%d#%d#%d#%d#%d\n", bufferedName, &bufferedMoney, &bufferedXp, &bufferedLevel,
                      &bufferedHealth, &bufferedEnergy, &bufferedArmor, &bufferedDamage) != EOF) {
            if (strcasecmp(bufferedName, name) == 0) {
                found = true;
                break;
            }
        }


        fclose(file);
        if (found) {
            printw("\nSave file already exists!\n");
            printw("Press any key to continue...");
            refresh();
            getch();
            return;
        }

        printw("Creating save file...\n");
        refresh();

        // create save file
        file = fopen("database/player.dat", "a");
        // name#money#xp#level#health#energy#armor#damage
        fprintf(file, "%s#%d#%d#%d#%d#%d#%d#%d\n", name, 0, 0, 1, 100, 100, 1, 1);
        fclose(file);

        printw("Save file created!\n");
        printw("\n\nPress any key to continue...");
        refresh();
        getch();

        lobby.init(name, 0, 0, 1, 100, 100, 1);
    }

    void loadGame() {
        clearScreen();
        echo();

        choosingPlayer = true;

        // check save file
        FILE *file = fopen("database/player.dat", "r");
        char bufferedName[128];
        int bufferedMoney;
        int bufferedXp;
        int bufferedLevel;
        int bufferedHealth;
        int bufferedEnergy;
        int bufferedArmor;
        int bufferedDamage;

        int i = 0;
        while (fscanf(file, "%[^#]#%d#%d#%d#%d#%d#%d#%d\n", bufferedName, &bufferedMoney, &bufferedXp, &bufferedLevel,
                      &bufferedHealth, &bufferedEnergy, &bufferedArmor, &bufferedDamage) != EOF) {
            strcpy(loadedPlayers[i].name, bufferedName);
            loadedPlayers[i].money = bufferedMoney;
            loadedPlayers[i].xp = bufferedXp;
            loadedPlayers[i].level = bufferedLevel;
            loadedPlayers[i].health = bufferedHealth;
            loadedPlayers[i].energy = bufferedEnergy;
            loadedPlayers[i].armor = bufferedArmor;

            i++;
        }
        fclose(file);

        printw("------------------------------");
        printw("\n| %-3s | %-12s | %-5s |", "No", "Name", "Level");
        printw("\n------------------------------\n");

        i = 0;
        for (const auto &item: loadedPlayers) {
            if (strcasecmp(item.name, "Unknown") == 0) {
                break;
            }

            printw("| %-3d | %-12s | %-5d |", i + 1, item.name, item.level);
            menuSelection == i ? printw("\t<\n") : printw("\n");
            i++;
        }

        i++;
        printw("| %-3s | %-12s | %-5s |", " ", " ", " ");
        printw("\n| %-3d | %-12s | %-5s |", i, "Back", " ");
        menuSelection == (i - 1) ? printw("\t<\n") : printw("\n");
        printw("------------------------------");
        refresh();
    }

    static void showSettings() {
        clearScreen();

        // show controls
        printw("Welcome to the game!\n\n");

        printw("Basic Movement:\n");
        printw("W - Move Up\n");
        printw("A - Move Left\n");
        printw("S - Move Down\n");
        printw("D - Move Right\n\n");

        printw("Lobby Arena\n");
        printw("SPACE - Interact\n");
        printw("Q - Quit\n\n");

        printw("Game Arena\n");
        printw("SPACE - Shoot\n");
        printw("R - Reload\n");
        printw("F - Skill\n");
        printw("1 - Use potion\n");
        printw("2 - Use max potion\n");
        printw("3 - Use energy\n");
        printw("4 - Use max energy\n");
        printw("g - Use bomb\n");
        printw("Q - Quit\n\n");

        printw("Press any key to continue...");
        getch();
    }

    static void showHowToPlay() {
        clearScreen();
        printw("Space Invader \n\n\n");
        printw("The object of the game is, basically, to shoot the invaders with your shooter\n");
        printw("while avoiding their shots and preventing an invasion. Amassing a high score is a further\n");
        printw("objective and one that must be prioritised against your continued survival.\n\n\n");
        printw("There is no time limit.\n");
        printw("except for the fact that if you do not shoot them all before they reach\n");
        printw("the bottom of the screen, the game ends.\n\n\n");
        printw("Credit : TinTin Winata, Efran Nathanael\n\n");
        printw("Back to menu ? [press any key]");
        getch();
    }

    int getPlayerSize() {
        int count = 0;
        for (const auto &item: loadedPlayers) {
            if (strcasecmp(item.name, "Unknown") == 0) {
                break;
            }

            count++;
        }

        return count;
    }

    void selectPlayer() {
        int playersSize = getPlayerSize();
        if (menuSelection > playersSize - 1) {
            // exit
            menuSelection = 0;
            choosingPlayer = false;
            return;
        }

        choosingPlayer = false;
        lobby.init(loadedPlayers[menuSelection].name, loadedPlayers[menuSelection].money,
                   loadedPlayers[menuSelection].xp, loadedPlayers[menuSelection].level,
                   loadedPlayers[menuSelection].health,
                   loadedPlayers[menuSelection].energy, loadedPlayers[menuSelection].armor);
        menuSelection = 0;
    }

    void onKeyPress(char c) {
        switch (c) {
            case 'w':
            case 'a': {
                int limit = choosingPlayer ? getPlayerSize() : 4;
                if (menuSelection > 0) {
                    menuSelection--;
                } else {
                    menuSelection = limit;
                }
                break;
            }
            case 's':
            case 'd': {
                int limit = choosingPlayer ? getPlayerSize() : 4;
                if (menuSelection < limit) {
                    menuSelection++;
                } else {
                    menuSelection = 0;
                }
                break;
            }
            case '\n':
            case ' ': {
                if (!choosingPlayer) {
                    switch (menuSelection) {
                        case 0: {
                            newGame();
                            break;
                        }
                        case 1: {
                            menuSelection = 0;
                            loadGame();
                            break;
                        }
                        case 2: {
                            showSettings();
                            break;
                        }
                        case 3: {
                            showHowToPlay();
                            break;
                        }
                        case 4: {
                            clearScreen();
                            for (auto &y: epLogo) {
                                for (char ch: y) {
                                    if (ch == '\0') {
                                        printw(" ");
                                    } else {
                                        printw("%c", ch);
                                    }
                                }
                                printw("\n");
                                refresh();
                                usleep(15000);
                            }
                            printw("\n\n            Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nullam sit.\n");
                            printw("                                ~ RedJacket 23-1 ~\n");
                            refresh();

                            sleep(1);
                            endwin();
                            exit(0);
                        }
                    }
                } else {
                    selectPlayer();
                    break;
                }
            }
        }
    }

    void mainMenu() const {
        clearScreen();
        noecho();

        printw("C Space Invader\n");
        printw("=================================\n\n");

        printw("1. New Game ");
        menuSelection == 0 ? printw("\t<\n") : printw("\n");
        printw("2. Load Game");
        menuSelection == 1 ? printw("\t<\n") : printw("\n");
        printw("3. Settings");
        menuSelection == 2 ? printw("\t<\n") : printw("\n");
        printw("4. How to play");
        menuSelection == 3 ? printw("\t<\n") : printw("\n");
        printw("5. Exit");
        menuSelection == 4 ? printw("\t\t<\n") : printw("\n");
    }

    [[noreturn]] void menuTick() {
        while (true) {
            char c = tolower(getch());
            onKeyPress(c);
            if (!choosingPlayer) {
                mainMenu();
            } else {
                loadGame();
            }
        }
    }

    void readFromFile() {
        FILE *file = fopen("assets/logo.txt", "r");

        // also read spaces
        int i = 0;
        while (fgets(epLogo[i], 22 * 80, file) != nullptr) {
            i++;
        }
        fclose(file);
    }

public:
    void init(Lobby aLobby) {
        cbreak();
        scrollok(stdscr, TRUE);
        noecho();

        readFromFile();
        lobby = aLobby;

        mainMenu();
        menuTick();
    }
};

void loadShips() {
    // hard coded bcs there's no ship name in da fileee!
    SpaceShip spaceShip;
    std::string name;
    char ship1[10][10] = {
            " / \\ ",
            "/ . \\",
            "|.x.|",
            "\\___/",
            "  v  "
    };
    name = "Default Spaceship";
    spaceShip.construct(5, 5, 2, name.c_str(), ship1);
    playerShips[0] = spaceShip;

    SpaceShip spaceShip2;
    char ship2[10][10] = {
            "   ^",
            " /   \\",
            "|  x  |",
            " \\___/",
            "   v"
    };
    name = "Great Spaceship";
    spaceShip2.construct(5, 7, 2, name.c_str(), ship2);
    playerShips[1] = spaceShip2;

    SpaceShip spaceShip3;
    char ship3[10][10] = {
            "/ \\",
            "|x|",
            "---",
            " v "
    };
    name = "Thin Spaceship";
    spaceShip3.construct(4, 3, 1, name.c_str(), ship3);
    playerShips[2] = spaceShip3;

    SpaceShip spaceShip4;
    char ship4[10][10] = {
            " _____",
            "|     |",
            "|  x  |",
            "|_____|",
            "   v   "
    };
    name = "Fat Spaceship";
    spaceShip4.construct(5, 7, 3, name.c_str(), ship4);
    playerShips[3] = spaceShip4;
}

int main() {
    initscr();
    loadShips();

    Menu menu;
    Lobby lobby;

    menu.init(lobby);

    endwin();
    return 0;
}