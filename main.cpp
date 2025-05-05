#include "SFML/Graphics.hpp"
#include "SFML/Audio.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <map>

using namespace sf;
using namespace std;


Texture bulletTexture, bulletTexture2, asteroidGreenTexture, asteroidBlueTexture, asteroidRedTexture;
Texture shipTexture1, shipTexture2, shipTexture3, shipTexture4, shipTexture5, shipTexture6, shipTexture7, shipTexture8, shipTexture9, shipTexture10, backgroundTexture, menuBackgroundTexture;
Texture settingsBackgroundTexture, leaderboardBackgroundTexture, authBackgroundTexture, shopbackgroundTexture, lockTexture;
Texture damageBuffTexture, fireRateBuffTexture, healthPackTexture, doubleShotBuffTexture, explosionPointTexture, shieldBuffTexture;
Music shopMusic;
SoundBuffer errorSoundBuffer;
Sound errorSound;
float currentSpeedMultiplier = 1.0;
int selectedShipSkin = 1;
const string SKIN_FILE = "C:\\Users\\User\\Desktop\\SFML\\Users\\selected_skin.txt";
float currentAsteroidSpawnTime = 0.3;

vector<string> musicFiles = {
    "C:\\Users\\User\\Desktop\\SFML\\Sounds\\Asteroids_main_menu.mp3",
    "C:\\Users\\User\\Desktop\\SFML\\Sounds\\GOOD CREDIT.mp3",
    "C:\\Users\\User\\Desktop\\SFML\\Sounds\\Evolution.mp3",
    "C:\\Users\\User\\Desktop\\SFML\\Sounds\\Honey bunn.mp3",
    "C:\\Users\\User\\Desktop\\SFML\\Sounds\\Bronny James.mp3",
    "C:\\Users\\User\\Desktop\\SFML\\Sounds\\In my zone.mp3"
};
vector<string> musicNames = { "BACKR00MS", "GOOD CREDIT", "Evolution", "Honey Bunn", "Bronny James","Im in my zone" };
int currentMusicIndex = 0;


float soundVolume = 20.0f;
float musicVolume = 20.0f;
const string SETTINGS_FILE = "C:\\Users\\User\\Desktop\\SFML\\Users\\settings.txt";

float clamp(float value, float min, float max) {
    if (min > max) swap(min, max);
    return std::max(min, std::min(max, value));
}

void loadSelectedSkin() {
    ifstream file(SKIN_FILE);
    if (file.is_open()) {
        file >> selectedShipSkin;
        file.close();
    }
}

void saveSelectedSkin() {
    ofstream file(SKIN_FILE);
    if (file.is_open()) {
        file << selectedShipSkin;
        file.close();
    }
}

void loadSettings() {
    ifstream file(SETTINGS_FILE);
    if (file.is_open()) {
        file >> soundVolume >> musicVolume >> currentMusicIndex;
        file.close();
    }
}

void saveSettings() {
    ofstream file(SETTINGS_FILE);
    if (file.is_open()) {
        file << soundVolume << " " << musicVolume << " " << currentMusicIndex;
        file.close();
    }
}

class UserManager {
private:
    map<string, pair<string, string>> users;
    map<string, int> highScores;
    map<string, int> userCoins;
    map<string, vector<int>> purchasedSkins;
    const string PURCHASED_SKINS_FILE = "C:\\Users\\User\\Desktop\\SFML\\Users\\purchased_skins.txt";
    const string USERS_FILE = "C:\\Users\\User\\Desktop\\SFML\\Users\\users.txt";
    const string SCORES_FILE = "C:\\Users\\User\\Desktop\\SFML\\Users\\user_scores.txt";
    int infiniteModeHighScore = 0;
    int timeModeHighScore = 0;
    string currentUser;

public:
    void addPurchasedSkin(const string& login, int skinIndex) {
        if (find(purchasedSkins[login].begin(), purchasedSkins[login].end(), skinIndex) == purchasedSkins[login].end()) {
            purchasedSkins[login].push_back(skinIndex);
            savePurchasedSkins();
        }
    }
    int getUserInfiniteModeHighScore() const {
        if (!isLoggedIn()) return 0;
        return infiniteModeHighScore;
    }
    bool hasPurchasedSkin(const string& login, int skinIndex) {
        auto it = purchasedSkins.find(login);
        if (it != purchasedSkins.end()) {
            return find(it->second.begin(), it->second.end(), skinIndex) != it->second.end();
        }
        return false;
    }
    void loadPurchasedSkins() {
        ifstream file(PURCHASED_SKINS_FILE);
        if (file.is_open()) {
            string line;
            while (getline(file, line)) {
                istringstream iss(line);
                string username;
                iss >> username;

                int skin;
                while (iss >> skin) {
                    purchasedSkins[username].push_back(skin);
                }
            }
            file.close();
        }
    }
    void savePurchasedSkins() {
        ofstream file(PURCHASED_SKINS_FILE);
        if (file.is_open()) {
            for (const auto& entry : purchasedSkins) {
                file << entry.first;
                for (int skin : entry.second) {
                    file << " " << skin;
                }
                file << "\n";
            }
            file.close();
        }
    }
    int getUserCoins() const {
        if (!isLoggedIn()) return 0;
        auto it = userCoins.find(currentUser);
        return (it != userCoins.end()) ? it->second : 0;
    }
    void addUserCoins(int coins) {
        if (!isLoggedIn()) return;
        userCoins[currentUser] += coins;
        saveUserCoins();
    }
    void saveUserCoins() {
        ofstream file("C:\\Users\\User\\Desktop\\SFML\\Users\\user_coins.txt");
        if (file.is_open()) {
            for (const auto& entry : userCoins) {
                file << entry.first << " " << entry.second << "\n";
            }
            file.close();
        }
    }
    void loadUserCoins() {
        ifstream file("C:\\Users\\User\\Desktop\\SFML\\Users\\user_coins.txt");
        if (file.is_open()) {
            string line;
            while (getline(file, line)) {
                istringstream iss(line);
                string username;
                int coins;
                if (iss >> username >> coins) {
                    userCoins[username] = coins;
                }
            }
            file.close();
        }
    }
    UserManager() {
        loadUsers();
        loadUserScores();
        loadUserCoins();
        loadPurchasedSkins();
        for (auto& user : users) {
            if (find(purchasedSkins[user.first].begin(), purchasedSkins[user.first].end(), 1) == purchasedSkins[user.first].end()) {
                purchasedSkins[user.first].push_back(1);
            }
        }
    }

    void loadUsers() {
        ifstream file(USERS_FILE);
        if (file.is_open()) {
            string line;
            while (getline(file, line)) {
                size_t separator1 = line.find(':');
                size_t separator2 = line.find(':', separator1 + 1);
                if (separator1 != string::npos && separator2 != string::npos) {
                    string login = line.substr(0, separator1);
                    string password = line.substr(separator1 + 1, separator2 - separator1 - 1);
                    string nickname = line.substr(separator2 + 1);
                    users[login] = make_pair(password, nickname);
                }
            }
            file.close();
        }
    }

    void loadUserScores() {
        ifstream file(SCORES_FILE);
        if (file.is_open()) {
            string line;
            while (getline(file, line)) {
                istringstream iss(line);
                string username;
                int infiniteScore, timeScore;
                if (iss >> username >> infiniteScore >> timeScore) {
                    if (username == currentUser) {
                        infiniteModeHighScore = infiniteScore;
                        timeModeHighScore = timeScore;
                    }
                }
            }
            file.close();
        }
    }

    void saveUserScores() {
        ofstream file(SCORES_FILE);
        if (file.is_open()) {
            file << currentUser << " " << infiniteModeHighScore << " " << timeModeHighScore << "\n";
            file.close();
        }
    }

    void saveUsers() {
        ofstream file(USERS_FILE);
        if (file.is_open()) {
            for (const auto& user : users) {
                file << user.first << ":" << user.second.first << ":" << user.second.second << "\n";
            }
            file.close();
        }
    }

    bool registerUser(const string& login, const string& password, const string& nickname) {
        if (login.empty() || password.empty() || nickname.empty()) return false;
        if (users.count(login)) return false;
        users[login] = make_pair(password, nickname);
        highScores[login] = 0;
        saveUsers();
        saveUserScores();
        return true;
    }

    bool loginUser(const string& login, const string& password) {
        auto it = users.find(login);
        if (it != users.end() && it->second.first == password) {
            currentUser = login;
            return true;
        }
        return false;
    }

    string getCurrentUser() const { return currentUser; }
    string getCurrentNickname() const {
        auto it = users.find(currentUser);
        return (it != users.end()) ? it->second.second : "";
    }
    bool isLoggedIn() const { return !currentUser.empty(); }
    void logout() { currentUser.clear(); }

    int getUserHighScore() const {
        if (!isLoggedIn()) return 0;
        auto it = highScores.find(currentUser);
        return (it != highScores.end()) ? it->second : 0;
    }
    int getUserTimeModeHighScore() const {
        if (!isLoggedIn()) return 0;
        return timeModeHighScore;
    }
    void updateUserHighScore(int score, bool isTimeMode) {
        if (!isLoggedIn()) return;

        if (isTimeMode) {
            cout << "Current time mode high: " << timeModeHighScore
                << ", new score: " << score << endl;

            if (score > timeModeHighScore) {
                timeModeHighScore = score;
                saveUserScores();
            }
        }
        else {
            if (score > infiniteModeHighScore) {
                infiniteModeHighScore = score;
                saveUserScores();
            }
        }
    }
};

class Bullet {
public:
    Bullet(float x, float y, float angle, const Texture& bulletTex) :
        damage(20), speed(800.f)
    {
        sprite.setTexture(bulletTex);
        sprite.setOrigin(bulletTex.getSize().x / 2, bulletTex.getSize().y / 2);
        sprite.setPosition(x, y);
        sprite.setRotation(angle + 90);

        if (&bulletTex == &bulletTexture2) {
            sprite.setScale(1.2f, 1.2f);
        }
    }

    void update(float deltaTime) {
        float radianAngle = (sprite.getRotation() - 180) * 3.14159f / 180.f;
        sprite.move(cos(radianAngle) * speed * deltaTime,
            sin(radianAngle) * speed * deltaTime);
    }

    Sprite sprite;
    int damage;

private:
    float speed;
};

class Asteroid {
public:
    Asteroid(float x, float y, int type, float angle) : speed(0), health(0), direction(angle) {
        switch (type) {
        case 0:
            sprite.setTexture(asteroidGreenTexture);
            health = 50;
            speed = 185.f;
            break;
        case 1:
            sprite.setTexture(asteroidBlueTexture);
            health = 80;
            speed = 135.f;
            break;
        case 2:
            sprite.setTexture(asteroidRedTexture);
            health = 100;
            speed = 100.f;
            break;
        }
        sprite.setOrigin(sprite.getTexture()->getSize().x / 2, sprite.getTexture()->getSize().y / 2);
        sprite.setPosition(x, y);
        sprite.setRotation(angle + 90);
    }

    void update(float deltaTime) {
        float radianAngle = direction * 3.14159f / 180.f;
        sprite.move(cos(radianAngle) * speed * deltaTime,
            sin(radianAngle) * speed * deltaTime);
    }

    float getRadius() const {
        return sprite.getTexture()->getSize().x / 2.0f * 0.34f;
    }

    Vector2f getCenter() const {
        return sprite.getPosition();
    }

    void takeDamage(int damage) {
        health -= damage;
    }

    int calculateScore() const {
        if (sprite.getTexture() == &asteroidGreenTexture) return 50;
        if (sprite.getTexture() == &asteroidBlueTexture) return 100;
        if (sprite.getTexture() == &asteroidRedTexture) return 200;
        return 0;
    }

    int calculateCoins() const {
        if (sprite.getTexture() == &asteroidGreenTexture) return 1;
        if (sprite.getTexture() == &asteroidBlueTexture) return 2;
        if (sprite.getTexture() == &asteroidRedTexture) return 3;
        return 0;
    }
    Sprite sprite;
    void setSpeed(float newSpeed) {
        speed = newSpeed;
    }
    int health;
private:
    float speed;
    float direction;
};

class Bonus {
public:
    enum BonusType { DAMAGE, FIRE_RATE, HEALTH, DOUBLE_SHOT, EXPLOSION_POINT, SHIELD };

    Bonus(float x, float y, BonusType type) : type(type), speed(150.f) {
        switch (type) {
        case DAMAGE:
            sprite.setTexture(damageBuffTexture);
            break;
        case FIRE_RATE:
            sprite.setTexture(fireRateBuffTexture);
            break;
        case HEALTH:
            sprite.setTexture(healthPackTexture);
            break;
        case DOUBLE_SHOT:
            sprite.setTexture(doubleShotBuffTexture);
            break;
        case EXPLOSION_POINT:
            sprite.setTexture(explosionPointTexture);
            break;
        case SHIELD:
            sprite.setTexture(shieldBuffTexture);
            break;
        }

        sprite.setOrigin(sprite.getTexture()->getSize().x / 2, sprite.getTexture()->getSize().y / 2);
        sprite.setPosition(x, y);
    }

    void update(float deltaTime) {
        sprite.move(0, speed * deltaTime);
    }
    int calculateScore() const {
        if (sprite.getTexture() == &asteroidGreenTexture) return 50;
        if (sprite.getTexture() == &asteroidBlueTexture) return 100;
        if (sprite.getTexture() == &asteroidRedTexture) return 200;
        return 0;
    }

    Sprite sprite;
    BonusType type;

private:
    float speed;
};

vector<Bonus> bonuses;
vector<Bullet> bullets;
vector<Asteroid> asteroids;
SoundBuffer shootBuffer, asteroidDestroyBuffer, explosionSoundBuffer;
Sound shootSound, asteroidDestroySound, explosionSound;

const string GLOBAL_HIGHSCORES_FILE = "C:\\Users\\User\\Desktop\\SFML\\Users\\global_highscores.txt";
const int MAX_HIGHSCORES = 10;

struct Highscore {
    string name;
    int score;
    bool isTimeMode;

    bool operator<(const Highscore& other) const {
        return score > other.score;
    }
};

vector<Highscore> globalHighscores;

void loadGlobalHighscores() {
    globalHighscores.clear();
    ifstream file(GLOBAL_HIGHSCORES_FILE);
    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            istringstream iss(line);
            Highscore hs;
            string mode;
            if (iss >> hs.name >> hs.score >> mode) {
                hs.isTimeMode = (mode == "time");
                globalHighscores.push_back(hs);
            }
        }
        file.close();
    }

    sort(globalHighscores.begin(), globalHighscores.end());
}

void saveGlobalHighscores() {
    ofstream file(GLOBAL_HIGHSCORES_FILE);
    if (file.is_open()) {
        for (const auto& hs : globalHighscores) {
            file << hs.name << " " << hs.score << " " << (hs.isTimeMode ? "time" : "infinite") << "\n";
        }
        file.close();
    }
}


void updateGlobalHighscores(UserManager& userManager, int score, bool isTimeMode) {
    string name = userManager.getCurrentNickname();
    bool found = false;

    for (auto& hs : globalHighscores) {
        if (hs.name == name && hs.isTimeMode == isTimeMode) {
            if (score > hs.score) {
                hs.score = score;
            }
            found = true;
            break;
        }
    }

    if (!found) {
        globalHighscores.push_back({ name, score, isTimeMode });
    }

    sort(globalHighscores.begin(), globalHighscores.end());
    if (globalHighscores.size() > MAX_HIGHSCORES * 2) {
        globalHighscores.resize(MAX_HIGHSCORES * 2);
    }

    saveGlobalHighscores();
}

void showLeaderboard(RenderWindow& window, Font& font) {
    loadGlobalHighscores();
    bool showTimeMode = false;

    Sprite background(leaderboardBackgroundTexture);
    background.setScale(
        window.getSize().x / float(leaderboardBackgroundTexture.getSize().x),
        window.getSize().y / float(leaderboardBackgroundTexture.getSize().y)
    );

    Text title("LEADERBOARD", font, 60);
    title.setFillColor(Color::White);
    title.setPosition(window.getSize().x / 2 - title.getGlobalBounds().width / 2, 50);


    RectangleShape modeButton(Vector2f(400, 50));
    modeButton.setFillColor(Color(70, 70, 70, 200));
    modeButton.setOutlineThickness(2);
    modeButton.setOutlineColor(Color::White);
    modeButton.setPosition(window.getSize().x / 2 - 200, 600);

    Text modeButtonText("Show Time Attack Mode", font, 30);
    modeButtonText.setFillColor(Color::White);
    modeButtonText.setPosition(
        modeButton.getPosition().x + (modeButton.getSize().x - modeButtonText.getGlobalBounds().width) / 2,
        modeButton.getPosition().y + (modeButton.getSize().y - modeButtonText.getGlobalBounds().height) / 2
    );

    vector<Text> entries;
    RectangleShape backButton(Vector2f(300, 60));
    backButton.setFillColor(Color(70, 70, 70, 200));
    backButton.setOutlineThickness(2);
    backButton.setOutlineColor(Color::White);
    backButton.setPosition(
        window.getSize().x / 2 - 150,
        window.getSize().y - 120
    );

    Text backButtonText("Back to Menu", font, 30);
    FloatRect textRect = backButtonText.getLocalBounds();
    backButtonText.setOrigin(textRect.left + textRect.width / 2.0f,
        textRect.top + textRect.height / 2.0f);
    backButtonText.setPosition(
        backButton.getPosition().x + backButton.getSize().x / 2,
        backButton.getPosition().y + backButton.getSize().y / 2
    );
    backButtonText.setFillColor(Color::White);

    while (window.isOpen()) {

        entries.clear();
        int count = 0;
        for (size_t i = 0; i < globalHighscores.size() && count < MAX_HIGHSCORES; ++i) {
            if (globalHighscores[i].isTimeMode == showTimeMode) {
                Text entry(to_string(count + 1) + ". " + globalHighscores[i].name + " - " + to_string(globalHighscores[i].score),
                    font, 30);
                entry.setFillColor(Color::White);
                entry.setPosition(window.getSize().x / 2 - entry.getGlobalBounds().width / 2, 200 + count * 40);
                entries.push_back(entry);
                count++;
            }
        }

        Event e;
        while (window.pollEvent(e)) {
            if (e.type == Event::Closed) window.close();
            if (e.type == Event::KeyPressed && e.key.code == Keyboard::Escape) {
                return;
            }

            if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left) {
                Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));
                if (modeButton.getGlobalBounds().contains(mousePos)) {
                    showTimeMode = !showTimeMode;
                    modeButtonText.setString(showTimeMode ? "Show Infinite Mode" : "Show Time Attack Mode");
                }
                else if (backButton.getGlobalBounds().contains(mousePos)) {
                    return;
                }
            }
        }

        Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));


        if (modeButton.getGlobalBounds().contains(mousePos)) {
            modeButton.setFillColor(Color(100, 100, 100, 200));
            modeButtonText.setFillColor(Color::Yellow);
        }
        else {
            modeButton.setFillColor(Color(70, 70, 70, 200));
            modeButtonText.setFillColor(Color::White);
        }

        if (backButton.getGlobalBounds().contains(mousePos)) {
            backButton.setFillColor(Color(100, 100, 100, 200));
            backButtonText.setFillColor(Color::Yellow);
        }
        else {
            backButton.setFillColor(Color(70, 70, 70, 200));
            backButtonText.setFillColor(Color::White);
        }

        window.clear();
        window.draw(background);
        window.draw(title);


        Text modeTitle(showTimeMode ? "TIME ATTACK MODE" : "INFINITE MODE", font, 40);
        modeTitle.setFillColor(showTimeMode ? Color::Cyan : Color::Green);
        modeTitle.setPosition(window.getSize().x / 2 - modeTitle.getGlobalBounds().width / 2, 120);
        window.draw(modeTitle);

        window.draw(modeButton);
        window.draw(modeButtonText);

        for (auto& entry : entries) window.draw(entry);
        window.draw(backButton);
        window.draw(backButtonText);
        window.display();
    }
}

bool showGameOverWithName(RenderWindow& window, Font& font, UserManager& userManager, int score, bool isTimeMode) {
    userManager.updateUserHighScore(score, isTimeMode);
    userManager.saveUserCoins();
    updateGlobalHighscores(userManager, score, isTimeMode);

    string playerName = userManager.getCurrentUser();
    Text nameText(playerName, font, 30);
    nameText.setFillColor(Color::White);
    nameText.setPosition(window.getSize().x / 2 - 100, window.getSize().y / 2 + 50);

    Text gameOver("GAME OVER", font, 70);
    gameOver.setFillColor(Color::Red);
    gameOver.setPosition(window.getSize().x / 2 - gameOver.getGlobalBounds().width / 2,
        window.getSize().y / 2 - 150);


    int highScore = isTimeMode ? userManager.getUserTimeModeHighScore() : userManager.getUserHighScore();
    Text highScoreText("Best (Infinite): " + to_string(userManager.getUserInfiniteModeHighScore()) +
        "\nBest (Time): " + to_string(userManager.getUserTimeModeHighScore()), font, 50);
    highScoreText.setFillColor(Color::Yellow);
    highScoreText.setPosition(window.getSize().x / 2 - highScoreText.getGlobalBounds().width / 2,
        gameOver.getPosition().y + gameOver.getGlobalBounds().height + 30);


    Text coinsText("Coins: " + to_string(userManager.getUserCoins()), font, 20);
    coinsText.setFillColor(Color::Yellow);
    coinsText.setPosition(20, window.getSize().y + 200);
    coinsText.setPosition(20, window.getSize().x + 200);
    Text options("Press R to Retry or M for Menu", font, 30);
    options.setFillColor(Color::White);
    options.setPosition(window.getSize().x / 2 - options.getGlobalBounds().width / 2,
        window.getSize().y / 2 + 100);

    while (window.isOpen()) {
        Event e;
        while (window.pollEvent(e)) {
            if (e.type == Event::Closed) return false;
            if (e.type == Event::KeyPressed) {
                if (e.key.code == Keyboard::R) return true;
                if (e.key.code == Keyboard::M) return false;
            }
        }

        window.clear();
        window.draw(gameOver);
        window.draw(highScoreText);
        window.draw(options);
        window.display();
    }
    return false;
}

bool confirmExit(RenderWindow& window, Font& font, UserManager& userManager) {
    Sprite bg(menuBackgroundTexture);
    bg.setScale(
        window.getSize().x / float(menuBackgroundTexture.getSize().x),
        window.getSize().y / float(menuBackgroundTexture.getSize().y)
    );

    Text prompt("ARE U SURE?", font, 40);
    prompt.setFillColor(Color::White);
    prompt.setPosition(window.getSize().x / 2 - prompt.getGlobalBounds().width / 2, window.getSize().y / 2 - 100);

    RectangleShape yesButton(Vector2f(300, 60));
    yesButton.setFillColor(Color(70, 70, 70, 200));
    yesButton.setOutlineThickness(2);
    yesButton.setOutlineColor(Color::White);
    yesButton.setPosition(window.getSize().x / 2 - 150, window.getSize().y / 2);

    Text yesButtonText("Yes", font, 30);
    FloatRect textRect = yesButtonText.getLocalBounds();
    yesButtonText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    yesButtonText.setPosition(yesButton.getPosition().x + yesButton.getSize().x / 2, yesButton.getPosition().y + yesButton.getSize().y / 2);
    yesButtonText.setFillColor(Color::White);

    RectangleShape noButton(Vector2f(300, 60));
    noButton.setFillColor(Color(70, 70, 70, 200));
    noButton.setOutlineThickness(2);
    noButton.setOutlineColor(Color::White);
    noButton.setPosition(window.getSize().x / 2 - 150, window.getSize().y / 2 + 80);

    Text noButtonText("No", font, 30);
    textRect = noButtonText.getLocalBounds();
    noButtonText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    noButtonText.setPosition(noButton.getPosition().x + noButton.getSize().x / 2, noButton.getPosition().y + noButton.getSize().y / 2);
    noButtonText.setFillColor(Color::White);

    while (window.isOpen()) {
        Event e;
        Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));

        while (window.pollEvent(e)) {
            if (e.type == Event::Closed) return false;
            if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left) {
                if (yesButton.getGlobalBounds().contains(mousePos)) {
                    userManager.saveUserCoins();
                    window.close();
                    return true;
                }
                if (noButton.getGlobalBounds().contains(mousePos)) return false;
            }
            if (e.type == Event::KeyPressed) {
                if (e.key.code == Keyboard::Y) {
                    window.close();
                    return true;
                }
                if (e.key.code == Keyboard::N) return false;
            }
        }

        if (yesButton.getGlobalBounds().contains(mousePos)) {
            yesButton.setFillColor(Color(100, 100, 100, 200));
            yesButton.setOutlineColor(Color::Yellow);
            yesButtonText.setFillColor(Color::Yellow);
        }
        else {
            yesButton.setFillColor(Color(70, 70, 70, 200));
            yesButton.setOutlineColor(Color::White);
            yesButtonText.setFillColor(Color::White);
        }

        if (noButton.getGlobalBounds().contains(mousePos)) {
            noButton.setFillColor(Color(100, 100, 100, 200));
            noButton.setOutlineColor(Color::Yellow);
            noButtonText.setFillColor(Color::Yellow);
        }
        else {
            noButton.setFillColor(Color(70, 70, 70, 200));
            noButton.setOutlineColor(Color::White);
            noButtonText.setFillColor(Color::White);
        }

        window.clear();
        window.draw(bg);
        window.draw(prompt);
        window.draw(yesButton);
        window.draw(yesButtonText);
        window.draw(noButton);
        window.draw(noButtonText);
        window.display();
    }
    return false;
}

void showSettings(RenderWindow& window, Font& font, Music& music, Sound& shootSound, Sound& asteroidDestroySound, Sound& explosionSound) {
    Sprite bg(settingsBackgroundTexture);
    bg.setScale(
        window.getSize().x / float(settingsBackgroundTexture.getSize().x),
        window.getSize().y / float(settingsBackgroundTexture.getSize().y)
    );

    Text title("SETTINGS", font, 40);
    title.setFillColor(Color::White);
    title.setPosition(window.getSize().x / 2 - title.getGlobalBounds().width / 2, 50);


    Text musicText("MUSIC VOLUME:", font, 30);
    musicText.setFillColor(Color::White);
    musicText.setPosition(150, 150);

    RectangleShape musicSliderBg(Vector2f(400, 10));
    musicSliderBg.setFillColor(Color(100, 100, 100));
    musicSliderBg.setPosition(150, 200);

    RectangleShape musicHandle(Vector2f(20, 30));
    musicHandle.setFillColor(Color::White);
    musicHandle.setPosition(150 + (musicVolume / 100.f * 400), 190);


    Text soundText("SOUND EFFECTS:", font, 30);
    soundText.setFillColor(Color::White);
    soundText.setPosition(150, 250);

    RectangleShape soundSliderBg(Vector2f(400, 10));
    soundSliderBg.setFillColor(Color(100, 100, 100));
    soundSliderBg.setPosition(150, 300);

    RectangleShape soundHandle(Vector2f(20, 30));
    soundHandle.setFillColor(Color::White);
    soundHandle.setPosition(150 + (soundVolume / 100.f * 400), 290);


    Text musicSelectionText("SELECT MUSIC TRACK:", font, 30);
    musicSelectionText.setFillColor(Color::White);
    musicSelectionText.setPosition(150, 350);


    const int musicCount = 6;
    vector<RectangleShape> musicButtons;
    vector<Text> musicButtonTexts;

    for (int i = 0; i < musicCount; ++i) {
        RectangleShape button(Vector2f(200, 40));
        button.setFillColor(currentMusicIndex == i ? Color(100, 200, 100, 200) : Color(70, 70, 70, 200));
        button.setOutlineThickness(2);
        button.setOutlineColor(currentMusicIndex == i ? Color::Yellow : Color::White);
        button.setPosition(150 + ((i % 3) * 220), 400 + (i / 3 * 50));
        musicButtons.push_back(button);

        Text text(musicNames[i], font, 25);
        text.setFillColor(currentMusicIndex == i ? Color::Yellow : Color::White);
        FloatRect textRect = text.getLocalBounds();
        text.setOrigin(textRect.left + textRect.width / 2.0f,
            textRect.top + textRect.height / 2.0f);
        text.setPosition(
            button.getPosition().x + button.getSize().x / 2,
            button.getPosition().y + button.getSize().y / 2
        );
        musicButtonTexts.push_back(text);
    }

    RectangleShape backButton(Vector2f(300, 60));
    backButton.setFillColor(Color(70, 70, 70, 200));
    backButton.setOutlineThickness(2);
    backButton.setOutlineColor(Color::White);
    backButton.setPosition(
        window.getSize().x / 2 - 150,
        window.getSize().y - 120
    );

    Text backButtonText("Back to Menu", font, 30);
    FloatRect textRect = backButtonText.getLocalBounds();
    backButtonText.setOrigin(textRect.left + textRect.width / 2.0f,
        textRect.top + textRect.height / 2.0f);
    backButtonText.setPosition(
        backButton.getPosition().x + backButton.getSize().x / 2,
        backButton.getPosition().y + backButton.getSize().y / 2
    );

    while (window.isOpen()) {
        Event e;
        while (window.pollEvent(e)) {
            if (e.type == Event::Closed) {
                saveSettings();
                return;
            }

            if (e.type == Event::KeyPressed && e.key.code == Keyboard::Escape) {
                saveSettings();
                return;
            }

            if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left) {
                Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));

                if (backButton.getGlobalBounds().contains(mousePos)) {
                    saveSettings();
                    return;
                }


                if (musicSliderBg.getGlobalBounds().contains(mousePos)) {
                    float vol = ((mousePos.x - musicSliderBg.getPosition().x) / musicSliderBg.getSize().x) * 100;
                    vol = clamp(vol, 0, 100);
                    musicVolume = vol;
                    music.setVolume(musicVolume);
                    shopMusic.setVolume(musicVolume);
                    musicHandle.setPosition(musicSliderBg.getPosition().x + musicVolume / 100.f * musicSliderBg.getSize().x, 190);
                }

                else if (soundSliderBg.getGlobalBounds().contains(mousePos)) {
                    soundVolume = ((mousePos.x - soundSliderBg.getPosition().x) / soundSliderBg.getSize().x) * 100;
                    soundVolume = clamp(soundVolume, 0, 100);
                    shootSound.setVolume(soundVolume);
                    asteroidDestroySound.setVolume(soundVolume);
                    explosionSound.setVolume(soundVolume);
                    soundHandle.setPosition(soundSliderBg.getPosition().x + soundVolume / 100.f * soundSliderBg.getSize().x, 290);
                }

                else {
                    for (int i = 0; i < musicCount; ++i) {
                        if (musicButtons[i].getGlobalBounds().contains(mousePos)) {
                            currentMusicIndex = i;
                            music.stop();
                            if (music.openFromFile(musicFiles[currentMusicIndex])) {
                                music.setLoop(true);
                                music.setVolume(musicVolume);
                                shopMusic.setVolume(musicVolume);
                                music.play();
                            }
                            break;
                        }
                    }
                }
            }


            if (e.type == Event::MouseMoved && Mouse::isButtonPressed(Mouse::Left)) {
                Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));


                if (musicSliderBg.getGlobalBounds().contains(mousePos)) {
                    float vol = ((mousePos.x - musicSliderBg.getPosition().x) / musicSliderBg.getSize().x) * 100;
                    vol = clamp(vol, 0, 100);
                    musicVolume = vol;
                    music.setVolume(musicVolume);
                    musicHandle.setPosition(musicSliderBg.getPosition().x + musicVolume / 100.f * musicSliderBg.getSize().x, 190);
                }

                else if (soundSliderBg.getGlobalBounds().contains(mousePos)) {
                    soundVolume = ((mousePos.x - soundSliderBg.getPosition().x) / soundSliderBg.getSize().x) * 100;
                    soundVolume = clamp(soundVolume, 0, 100);
                    shootSound.setVolume(soundVolume);
                    asteroidDestroySound.setVolume(soundVolume);
                    explosionSound.setVolume(soundVolume);
                    soundHandle.setPosition(soundSliderBg.getPosition().x + soundVolume / 100.f * soundSliderBg.getSize().x, 290);
                }
            }
        }


        Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));
        if (backButton.getGlobalBounds().contains(mousePos)) {
            backButton.setFillColor(Color(100, 100, 100, 200));
            backButtonText.setFillColor(Color::Yellow);
        }
        else {
            backButton.setFillColor(Color(70, 70, 70, 200));
            backButtonText.setFillColor(Color::White);
        }


        for (int i = 0; i < musicCount; ++i) {
            if (musicButtons[i].getGlobalBounds().contains(mousePos) && currentMusicIndex != i) {
                musicButtons[i].setFillColor(Color(100, 100, 100, 200));
                musicButtonTexts[i].setFillColor(Color::Yellow);
            }
            else if (currentMusicIndex == i) {
                musicButtons[i].setFillColor(Color(100, 200, 100, 200));
                musicButtons[i].setOutlineColor(Color::Yellow);
                musicButtonTexts[i].setFillColor(Color::Yellow);
            }
            else {
                musicButtons[i].setFillColor(Color(70, 70, 70, 200));
                musicButtons[i].setOutlineColor(Color::White);
                musicButtonTexts[i].setFillColor(Color::White);
            }
        }

        window.clear();
        window.draw(bg);
        window.draw(title);


        window.draw(musicText);
        window.draw(musicSliderBg);
        window.draw(musicHandle);


        window.draw(soundText);
        window.draw(soundSliderBg);
        window.draw(soundHandle);


        window.draw(musicSelectionText);
        for (int i = 0; i < musicCount; ++i) {
            window.draw(musicButtons[i]);
            window.draw(musicButtonTexts[i]);
        }

        window.draw(backButton);
        window.draw(backButtonText);
        window.display();
    }
}
const string SHOP_MUSIC_FILE = "C:\\Users\\User\\Desktop\\SFML\\Sounds\\shop.mp3";
void showSkinSelection(RenderWindow& window, Font& font, Music& backgroundMusic, UserManager& userManager) {
    bool wasMainMusicPlaying = (backgroundMusic.getStatus() == Music::Playing);
    float mainMusicPosition = backgroundMusic.getPlayingOffset().asSeconds();

    backgroundMusic.stop();
    shopMusic.setVolume(musicVolume);

    if (!shopMusic.openFromFile(SHOP_MUSIC_FILE)) {
        cerr << "Error loading shop music!" << endl;
        return;
    }
    shopMusic.setLoop(true);
    shopMusic.setVolume(musicVolume);
    shopMusic.play();

    auto restoreMusic = [&]() {
        shopMusic.stop();
        if (wasMainMusicPlaying) {
            backgroundMusic.setPlayingOffset(seconds(mainMusicPosition));
            backgroundMusic.play();
        }
        };

    Sprite bg(shopbackgroundTexture);
    bg.setPosition(0, 0);
    bg.setScale(
        window.getSize().x / float(shopbackgroundTexture.getSize().x),
        window.getSize().y / float(shopbackgroundTexture.getSize().y)
    );

    struct ShipStats {
        string name;
        int damage;
        float fireRate;
        float speed;
        int price;
        bool owned;
    };

    vector<ShipStats> shipsStats = {
        {"Default Fighter",  20, 0.28f, 300.f, 0, true},
        {"Swift Scout",     20, 0.23f, 350.f, 250, userManager.hasPurchasedSkin(userManager.getCurrentUser(), 2)},
        {"Heavy Cruiser",   30, 0.40f, 300.f, 500, userManager.hasPurchasedSkin(userManager.getCurrentUser(), 3)},
        {"Stealth Ship",    25, 0.30f, 280.f, 750, userManager.hasPurchasedSkin(userManager.getCurrentUser(), 4)},
        {"Assault Ship",    22, 0.20f, 320.f, 1000, userManager.hasPurchasedSkin(userManager.getCurrentUser(), 5)},
        {"Guardian",        20, 0.22f, 310.f, 1250, userManager.hasPurchasedSkin(userManager.getCurrentUser(), 6)},
        {"Tactical Ship",   27, 0.35f, 270.f, 1500, userManager.hasPurchasedSkin(userManager.getCurrentUser(), 7)},
        {"Interceptor",     20, 0.10f, 400.f, 1750, userManager.hasPurchasedSkin(userManager.getCurrentUser(), 8)},
        {"Battle Frigate",  35, 0.60f, 230.f, 2000, userManager.hasPurchasedSkin(userManager.getCurrentUser(), 9)},
        {"Dreadnought",     40, 0.50f, 200.f, 2250, userManager.hasPurchasedSkin(userManager.getCurrentUser(), 10)}
    };

    RectangleShape rightPanel(Vector2f(400, window.getSize().y - 100));
    rightPanel.setFillColor(Color(50, 50, 50, 200));
    rightPanel.setOutlineThickness(2);
    rightPanel.setOutlineColor(Color::White);
    rightPanel.setPosition(window.getSize().x - 450, 50);

    RectangleShape descriptionBox(Vector2f(350, 400));
    descriptionBox.setFillColor(Color(30, 30, 30, 220));
    descriptionBox.setOutlineThickness(1);
    descriptionBox.setOutlineColor(Color::White);
    descriptionBox.setPosition(rightPanel.getPosition().x + 25, rightPanel.getPosition().y + 150);

    RectangleShape buyButton(Vector2f(200, 50));
    buyButton.setFillColor(Color(70, 70, 70, 200));
    buyButton.setOutlineThickness(2);
    buyButton.setOutlineColor(Color::White);
    buyButton.setPosition(descriptionBox.getPosition().x + (descriptionBox.getSize().x - 200) / 2,
        descriptionBox.getPosition().y + descriptionBox.getSize().y - 70);

    Text buyButtonText("BUY", font, 30);
    buyButtonText.setFillColor(Color::White);
    FloatRect buyTextRect = buyButtonText.getLocalBounds();
    buyButtonText.setOrigin(buyTextRect.left + buyTextRect.width / 2.0f,
        buyTextRect.top + buyTextRect.height / 2.0f);
    buyButtonText.setPosition(buyButton.getPosition().x + buyButton.getSize().x / 2,
        buyButton.getPosition().y + buyButton.getSize().y / 2);

    Text priceText("", font, 25);
    priceText.setFillColor(Color::Yellow);
    priceText.setPosition(buyButton.getPosition().x + (buyButton.getSize().x - priceText.getGlobalBounds().width) / 2,
        buyButton.getPosition().y + buyButton.getSize().y - 80);

    Text title("SELECT SHIP SKIN", font, 60);
    title.setFillColor(Color::White);
    title.setPosition(window.getSize().x / 2 - title.getGlobalBounds().width / 2, 50);

    vector<RectangleShape> skinBgs;
    vector<Sprite> shipSprites;
    vector<Text> skinTexts;

    Texture* shipTextures[] = { &shipTexture1, &shipTexture2, &shipTexture3, &shipTexture4, &shipTexture5,
                              &shipTexture6, &shipTexture7, &shipTexture8, &shipTexture9, &shipTexture10 };

    for (int i = 0; i < 10; i++) {
        RectangleShape bg(Vector2f(200, 150));
        bg.setFillColor(Color(70, 70, 70, 150));
        bg.setOutlineThickness(2);
        bg.setOutlineColor(selectedShipSkin == i + 1 ? Color::Yellow : Color::White);

        if (i < 5) {
            bg.setPosition(150 + (i * 220), window.getSize().y / 2 - 180);
        }
        else {
            bg.setPosition(150 + ((i - 5) * 220), window.getSize().y / 2 + 30);
        }
        skinBgs.push_back(bg);

        Sprite ship(*shipTextures[i]);
        ship.setOrigin(shipTextures[i]->getSize().x / 2, shipTextures[i]->getSize().y / 2);
        ship.setScale(1.5f, 1.5f);
        ship.setPosition(bg.getPosition().x + bg.getSize().x / 2, bg.getPosition().y + bg.getSize().y / 2 - 10);
        shipSprites.push_back(ship);

        Text text(shipsStats[i].name, font, 20);
        text.setFillColor(selectedShipSkin == i + 1 ? Color::Yellow : Color::White);
        text.setPosition(bg.getPosition().x + (bg.getSize().x - text.getGlobalBounds().width) / 2,
            bg.getPosition().y + bg.getSize().y - 30);
        skinTexts.push_back(text);
    }

    Text infoTitle("SHIP STATS", font, 40);
    infoTitle.setFillColor(Color::Yellow);
    infoTitle.setPosition(rightPanel.getPosition().x + (rightPanel.getSize().x - infoTitle.getGlobalBounds().width) / 2,
        rightPanel.getPosition().y + 30);

    Text shipName("", font, 30);
    shipName.setFillColor(Color::White);
    shipName.setPosition(descriptionBox.getPosition().x + 20, descriptionBox.getPosition().y + 20);

    Text shipDamage("Damage: ", font, 25);
    shipDamage.setFillColor(Color::Red);
    shipDamage.setPosition(descriptionBox.getPosition().x + 20, descriptionBox.getPosition().y + 70);

    Text shipFireRate("Fire Rate: ", font, 25);
    shipFireRate.setFillColor(Color::Yellow);
    shipFireRate.setPosition(descriptionBox.getPosition().x + 20, descriptionBox.getPosition().y + 110);

    Text shipSpeed("Speed: ", font, 25);
    shipSpeed.setFillColor(Color::Green);
    shipSpeed.setPosition(descriptionBox.getPosition().x + 20, descriptionBox.getPosition().y + 150);

    Text shipDescription("", font, 22);
    shipDescription.setFillColor(Color(200, 200, 200));
    shipDescription.setPosition(descriptionBox.getPosition().x + 20, descriptionBox.getPosition().y + 190);
    shipDescription.setLineSpacing(1.2f);

    Text coinsText("Coins: " + to_string(userManager.getUserCoins()), font, 30);
    coinsText.setFillColor(Color::Yellow);
    coinsText.setPosition(rightPanel.getPosition().x + (rightPanel.getSize().x - coinsText.getGlobalBounds().width) / 2,
        rightPanel.getPosition().y + rightPanel.getSize().y - 50);

    auto updateShipInfo = [&](int shipIndex) {
        shipIndex--;
        if (shipIndex >= 0 && shipIndex < 10) {
            shipName.setString(shipsStats[shipIndex].name);
            shipDamage.setString("Damage: " + to_string(shipsStats[shipIndex].damage));
            ostringstream stream;
            stream << std::fixed << setprecision(2);
            stream << "Fire Rate: " << shipsStats[shipIndex].fireRate << " sec";
            shipFireRate.setString(stream.str());
            shipSpeed.setString("Speed: " + to_string((int)shipsStats[shipIndex].speed) + " units");

            string description = "A well-balanced ship with\ngood overall performance.";
            switch (shipIndex) {
            case 1: description = "Fast but lightly armed\nscout ship."; break;
            case 2: description = "Heavily armed but slow\ncruiser."; break;
            }
            shipDescription.setString(description);

            bool canAfford = userManager.getUserCoins() >= shipsStats[shipIndex].price;
            bool alreadyOwned = userManager.hasPurchasedSkin(userManager.getCurrentUser(), shipIndex + 1);

            if (alreadyOwned) {
                buyButtonText.setString("OWNED");
                priceText.setString("");
                buyButton.setFillColor(Color(50, 150, 50, 200));
                buyButton.setOutlineColor(Color::White);
            }
            else if (canAfford) {
                buyButtonText.setString("BUY");
                priceText.setString(to_string(shipsStats[shipIndex].price) + " coins");
                buyButton.setFillColor(Color(70, 70, 70, 200));
                buyButton.setOutlineColor(Color::Yellow);
            }
            else {
                buyButtonText.setString("BUY");
                priceText.setString(to_string(shipsStats[shipIndex].price) + " coins");
                buyButton.setFillColor(Color(30, 30, 30, 200));
                buyButton.setOutlineColor(Color::Transparent);
            }

            priceText.setPosition(buyButton.getPosition().x + (buyButton.getSize().x - priceText.getGlobalBounds().width) / 2,
                buyButton.getPosition().y + buyButton.getSize().y - 80);
        }
        };

    updateShipInfo(selectedShipSkin);

    RectangleShape backButton(Vector2f(300, 60));
    backButton.setFillColor(Color(70, 70, 70, 200));
    backButton.setOutlineThickness(2);
    backButton.setOutlineColor(Color::White);
    backButton.setPosition(window.getSize().x / 2 - 150, window.getSize().y - 120);

    Text backButtonText("Back to Menu", font, 30);
    backButtonText.setPosition(
        backButton.getPosition().x + (backButton.getSize().x - backButtonText.getGlobalBounds().width) / 2,
        backButton.getPosition().y + (backButton.getSize().y - backButtonText.getGlobalBounds().height) / 2
    );
    backButtonText.setFillColor(Color::White);

    while (window.isOpen()) {
        Event e;
        while (window.pollEvent(e)) {
            if (e.type == Event::Closed) {
                restoreMusic();
                window.close();
                return;
            }
            if (e.type == Event::KeyPressed && e.key.code == Keyboard::Escape) {
                restoreMusic();
                return;
            }
            if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left) {
                Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));

                for (int i = 0; i < 10; i++) {
                    if (skinBgs[i].getGlobalBounds().contains(mousePos)) {
                        updateShipInfo(i + 1);
                        selectedShipSkin = i + 1;
                        for (int j = 0; j < 10; j++) {
                            skinBgs[j].setOutlineColor(selectedShipSkin == j + 1 ? Color::Yellow : Color::White);
                            skinTexts[j].setFillColor(selectedShipSkin == j + 1 ? Color::Yellow : Color::White);
                        }
                    }
                }

                if (buyButton.getGlobalBounds().contains(mousePos)) {
                    int skinIndex = selectedShipSkin - 1;
                    bool canAfford = userManager.getUserCoins() >= shipsStats[skinIndex].price;
                    bool alreadyOwned = userManager.hasPurchasedSkin(userManager.getCurrentUser(), skinIndex + 1);

                    if (!alreadyOwned && canAfford) {
                        userManager.addUserCoins(-shipsStats[skinIndex].price);
                        userManager.addPurchasedSkin(userManager.getCurrentUser(), skinIndex + 1);
                        shipsStats[skinIndex].owned = true;

                        coinsText.setString("Coins: " + to_string(userManager.getUserCoins()));
                        updateShipInfo(selectedShipSkin);
                        saveSelectedSkin();


                        for (int i = 0; i < 10; i++) {
                            skinBgs[i].setOutlineColor(selectedShipSkin == i + 1 ? Color::Yellow : Color::White);
                            skinTexts[i].setFillColor(selectedShipSkin == i + 1 ? Color::Yellow : Color::White);
                        }
                    }
                    else if (!canAfford) {
                        errorSound.play();
                    }
                }

                if (backButton.getGlobalBounds().contains(mousePos)) {
                    restoreMusic();
                    return;
                }
            }
        }

        Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));

        for (int i = 0; i < 10; i++) {
            if (skinBgs[i].getGlobalBounds().contains(mousePos) && selectedShipSkin != i + 1) {
                skinBgs[i].setOutlineColor(Color::Yellow);
                skinBgs[i].setFillColor(Color(100, 100, 100, 150));
            }
            else if (selectedShipSkin != i + 1) {
                skinBgs[i].setOutlineColor(Color::White);
                skinBgs[i].setFillColor(Color(70, 70, 70, 150));
            }
        }

        int currentSkinIndex = selectedShipSkin - 1;
        bool canAfford = userManager.getUserCoins() >= shipsStats[currentSkinIndex].price;
        bool alreadyOwned = userManager.hasPurchasedSkin(userManager.getCurrentUser(), currentSkinIndex + 1);

        if (buyButton.getGlobalBounds().contains(mousePos) && !alreadyOwned) {
            if (canAfford) {
                buyButton.setFillColor(Color(100, 100, 100, 200));
                buyButton.setOutlineColor(Color::Yellow);
                buyButtonText.setFillColor(Color::Yellow);
            }
            else {
                buyButton.setFillColor(Color(30, 30, 30, 200));
                buyButton.setOutlineColor(Color::Transparent);
                buyButtonText.setFillColor(Color::White);
            }
        }
        else if (!alreadyOwned) {
            if (canAfford) {
                buyButton.setFillColor(Color(70, 70, 70, 200));
                buyButton.setOutlineColor(Color::Yellow);
                buyButtonText.setFillColor(Color::White);
            }
            else {
                buyButton.setFillColor(Color(30, 30, 30, 200));
                buyButton.setOutlineColor(Color::Transparent);
                buyButtonText.setFillColor(Color::White);
            }
        }

        backButton.setFillColor(backButton.getGlobalBounds().contains(mousePos) ?
            Color(100, 100, 100, 200) : Color(70, 70, 70, 200));
        backButtonText.setFillColor(backButton.getGlobalBounds().contains(mousePos) ?
            Color::Yellow : Color::White);

        window.clear();
        window.draw(bg);

        window.draw(rightPanel);
        window.draw(infoTitle);
        window.draw(descriptionBox);
        window.draw(shipName);
        window.draw(shipDamage);
        window.draw(shipFireRate);
        window.draw(shipSpeed);
        window.draw(shipDescription);
        window.draw(buyButton);
        window.draw(buyButtonText);
        window.draw(priceText);
        window.draw(coinsText);

        window.draw(title);

        for (int i = 0; i < 10; i++) {
            bool purchased = userManager.hasPurchasedSkin(userManager.getCurrentUser(), i + 1);
            bool canAfford = userManager.getUserCoins() >= shipsStats[i].price;

            if (!purchased && canAfford) {
                skinBgs[i].setFillColor(Color(70, 70, 70, 150));
                shipSprites[i].setColor(Color::White);
            }
            else if (!purchased) {
                skinBgs[i].setFillColor(Color(30, 30, 30, 150));
                shipSprites[i].setColor(Color(150, 150, 150, 150));

                Sprite lock(lockTexture);
                lock.setOrigin(lockTexture.getSize().x / 2, lockTexture.getSize().y / 2);
                lock.setPosition(skinBgs[i].getPosition().x + skinBgs[i].getSize().x / 2,
                    skinBgs[i].getPosition().y + skinBgs[i].getSize().y / 2);
                window.draw(lock);
            }
            else {
                skinBgs[i].setFillColor(Color(70, 70, 70, 150));
                shipSprites[i].setColor(Color::White);
            }

            skinBgs[i].setOutlineColor(selectedShipSkin == i + 1 ? Color::Yellow : Color::White);
            window.draw(skinBgs[i]);
            window.draw(shipSprites[i]);
            window.draw(skinTexts[i]);
        }

        window.draw(backButton);
        window.draw(backButtonText);
        window.display();
    }
}

void showGameModeSelection(RenderWindow& window, Font& font, bool& infiniteMode, bool& timeMode) {
    Sprite bg(menuBackgroundTexture);
    bg.setScale(
        window.getSize().x / float(menuBackgroundTexture.getSize().x),
        window.getSize().y / float(menuBackgroundTexture.getSize().y)
    );

    RectangleShape infiniteModeButton(Vector2f(400, 60));
    infiniteModeButton.setFillColor(Color(70, 70, 70, 200));
    infiniteModeButton.setOutlineThickness(2);
    infiniteModeButton.setOutlineColor(Color::White);
    infiniteModeButton.setPosition(
        window.getSize().x / 2 - 200,
        window.getSize().y / 2 - 70
    );

    Text infiniteModeText("Infinite Mode", font, 40);
    FloatRect textRect = infiniteModeText.getLocalBounds();
    infiniteModeText.setOrigin(textRect.left + textRect.width / 2.0f,
        textRect.top + textRect.height / 2.0f);
    infiniteModeText.setPosition(
        infiniteModeButton.getPosition().x + infiniteModeButton.getSize().x / 2,
        infiniteModeButton.getPosition().y + infiniteModeButton.getSize().y / 2
    );
    infiniteModeText.setFillColor(Color::White);

    RectangleShape timeModeButton(Vector2f(400, 60));
    timeModeButton.setFillColor(Color(70, 70, 70, 200));
    timeModeButton.setOutlineThickness(2);
    timeModeButton.setOutlineColor(Color::White);
    timeModeButton.setPosition(
        window.getSize().x / 2 - 200,
        window.getSize().y / 2 + 30
    );

    Text timeModeText("Time Attack Mode", font, 40);
    textRect = timeModeText.getLocalBounds();
    timeModeText.setOrigin(textRect.left + textRect.width / 2.0f,
        textRect.top + textRect.height / 2.0f);
    timeModeText.setPosition(
        timeModeButton.getPosition().x + timeModeButton.getSize().x / 2,
        timeModeButton.getPosition().y + timeModeButton.getSize().y / 2
    );
    timeModeText.setFillColor(Color::White);

    RectangleShape backButton(Vector2f(300, 60));
    backButton.setFillColor(Color(70, 70, 70, 200));
    backButton.setOutlineThickness(2);
    backButton.setOutlineColor(Color::White);
    backButton.setPosition(
        window.getSize().x / 2 - 150,
        window.getSize().y - 120
    );

    Text backButtonText("Back to Menu", font, 30);
    textRect = backButtonText.getLocalBounds();
    backButtonText.setOrigin(textRect.left + textRect.width / 2.0f,
        textRect.top + textRect.height / 2.0f);
    backButtonText.setPosition(
        backButton.getPosition().x + backButton.getSize().x / 2,
        backButton.getPosition().y + backButton.getSize().y / 2
    );
    backButtonText.setFillColor(Color::White);

    Text title("SELECT GAME MODE", font, 60);
    title.setFillColor(Color::White);
    title.setPosition(window.getSize().x / 2 - title.getGlobalBounds().width / 2, 50);

    while (window.isOpen()) {
        Event e;
        while (window.pollEvent(e)) {
            if (e.type == Event::Closed) {
                window.close();
                return;
            }
            if (e.type == Event::KeyPressed && e.key.code == Keyboard::Escape) {
                shopMusic.stop();
                return;
            }
            if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left) {
                Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));

                if (infiniteModeButton.getGlobalBounds().contains(mousePos)) {
                    infiniteMode = true;
                    timeMode = false;
                    return;
                }
                else if (timeModeButton.getGlobalBounds().contains(mousePos)) {
                    infiniteMode = false;
                    timeMode = true;
                    return;
                }
                else if (backButton.getGlobalBounds().contains(mousePos)) {
                    shopMusic.stop();
                    return;
                }
            }
        }

        Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));

        if (infiniteModeButton.getGlobalBounds().contains(mousePos)) {
            infiniteModeButton.setFillColor(Color(100, 100, 100, 200));
            infiniteModeText.setFillColor(Color::Yellow);
        }
        else {
            infiniteModeButton.setFillColor(Color(70, 70, 70, 200));
            infiniteModeText.setFillColor(Color::White);
        }

        if (timeModeButton.getGlobalBounds().contains(mousePos)) {
            timeModeButton.setFillColor(Color(100, 100, 100, 200));
            timeModeText.setFillColor(Color::Yellow);
        }
        else {
            timeModeButton.setFillColor(Color(70, 70, 70, 200));
            timeModeText.setFillColor(Color::White);
        }

        if (backButton.getGlobalBounds().contains(mousePos)) {
            backButton.setFillColor(Color(100, 100, 100, 200));
        }
        else {
            backButton.setFillColor(Color(70, 70, 70, 200));
        }

        window.clear();
        window.draw(bg);
        window.draw(title);
        window.draw(infiniteModeButton);
        window.draw(infiniteModeText);
        window.draw(timeModeButton);
        window.draw(timeModeText);
        window.draw(backButton);
        window.draw(backButtonText);
        window.display();
    }
}

void increaseAsteroidsSpeed(float multiplier) {
    for (auto& asteroid : asteroids) {
        const Texture* texture = asteroid.sprite.getTexture();
        if (texture) {
            if (texture == &asteroidGreenTexture) {
                asteroid.setSpeed(185.f * multiplier);
            }
            else if (texture == &asteroidBlueTexture) {
                asteroid.setSpeed(135.f * multiplier);
            }
            else if (texture == &asteroidRedTexture) {
                asteroid.setSpeed(100.f * multiplier);
            }
        }
    }
}

int showPauseMenu(RenderWindow& window, Font& font) {
    RectangleShape overlay(Vector2f(window.getSize().x, window.getSize().y));
    overlay.setFillColor(Color(0, 0, 0, 180));

    Text pauseText("PAUSED", font, 70);
    pauseText.setFillColor(Color::White);
    pauseText.setPosition(
        window.getSize().x / 2 - pauseText.getGlobalBounds().width / 2,
        window.getSize().y / 2 - 150
    );

    vector<RectangleShape> buttons;
    vector<Text> buttonTexts;
    vector<string> options = { "RESUME", "BACK TO MENU" };

    for (size_t i = 0; i < options.size(); ++i) {
        RectangleShape button(Vector2f(400, 60));
        button.setFillColor(Color(70, 70, 70, 200));
        button.setOutlineThickness(2);
        button.setOutlineColor(Color::White);
        button.setPosition(
            window.getSize().x / 2 - 200,
            window.getSize().y / 2 - 30 + (i * 100)
        );
        buttons.push_back(button);

        Text text(options[i], font, 40);
        text.setFillColor(Color::White);
        FloatRect textRect = text.getLocalBounds();
        text.setOrigin(textRect.left + textRect.width / 2.0f,
            textRect.top + textRect.height / 2.0f);
        text.setPosition(
            button.getPosition().x + button.getSize().x / 2,
            button.getPosition().y + button.getSize().y / 2
        );
        buttonTexts.push_back(text);
    }

    while (window.isOpen()) {
        Event e;
        while (window.pollEvent(e)) {
            if (e.type == Event::Closed) {
                window.close();
                return 0;
            }

            if (e.type == Event::KeyPressed && e.key.code == Keyboard::Escape) {
                return 1;
            }

            if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left) {
                Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));

                for (size_t i = 0; i < buttons.size(); ++i) {
                    if (buttons[i].getGlobalBounds().contains(mousePos)) {
                        return i + 1;
                    }
                }
            }
        }

        Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));
        for (size_t i = 0; i < buttons.size(); ++i) {
            if (buttons[i].getGlobalBounds().contains(mousePos)) {
                buttons[i].setFillColor(Color(100, 100, 100, 200));
                buttonTexts[i].setFillColor(Color::Yellow);
            }
            else {
                buttons[i].setFillColor(Color(70, 70, 70, 200));
                buttonTexts[i].setFillColor(Color::White);
            }
        }

        window.clear();
        window.draw(overlay);
        window.draw(pauseText);
        for (size_t i = 0; i < buttons.size(); ++i) {
            window.draw(buttons[i]);
            window.draw(buttonTexts[i]);
        }

        window.display();
    }

    return 0;
}

bool isCirclesColliding(const Vector2f& center1, float radius1, const Vector2f& center2, float radius2) {
    float dx = center1.x - center2.x;
    float dy = center1.y - center2.y;
    float distance = sqrt(dx * dx + dy * dy);
    return distance < (radius1 + radius2);
}

void gameLoop(RenderWindow& window, Font& font, UserManager& userManager) {
    bool restart = true;
    while (restart) {
        bool infiniteMode = false, timeMode = false;
        showGameModeSelection(window, font, infiniteMode, timeMode);

        if (!infiniteMode && !timeMode) return;
        if (!userManager.hasPurchasedSkin(userManager.getCurrentUser(), selectedShipSkin)) {
            selectedShipSkin = 1;
            saveSelectedSkin();
        }

        currentSpeedMultiplier = 1.0f;
        bullets.clear();
        asteroids.clear();
        bonuses.clear();
        int score = 0, health = 100;
        float gameTime = 60.0f;
        float timerDuration = 30.0f;
        float remainingTime = timerDuration;

        int shieldHits = 0;
        float damageMultiplier = 1.0f;
        float fireRateMultiplier = 1.0f;
        bool doubleShotActive = false;
        bool shieldActive = false;
        int explosionPoints = 0;
        bool explosionActive = false;
        float explosionRadius = 0.f;
        Vector2f explosionCenter;
        Clock damageBuffTimer, fireRateBuffTimer, doubleShotTimer, explosionTimer, shieldTimer;
        const float BUFF_DURATION = 10.0f;
        const float DOUBLE_SHOT_DURATION = 10.0f;
        const float EXPLOSION_DURATION = 0.5f;
        const float SHIELD_DURATION = 5.0f;

        Text timerText("Time Left: " + to_string((int)gameTime), font, 30);
        timerText.setFillColor(Color::White);
        timerText.setPosition(20, 100);

        Texture& currentShipTexture =
            (selectedShipSkin == 1) ? shipTexture1 :
            (selectedShipSkin == 2) ? shipTexture2 :
            (selectedShipSkin == 3) ? shipTexture3 :
            (selectedShipSkin == 4) ? shipTexture4 :
            (selectedShipSkin == 5) ? shipTexture5 :
            (selectedShipSkin == 6) ? shipTexture6 :
            (selectedShipSkin == 7) ? shipTexture7 :
            (selectedShipSkin == 8) ? shipTexture8 :
            (selectedShipSkin == 9) ? shipTexture9 :
            shipTexture10;
        Sprite ship(currentShipTexture);
        ship.setOrigin(currentShipTexture.getSize().x / 2, currentShipTexture.getSize().y / 2);
        ship.setPosition(window.getSize().x / 2, window.getSize().y / 2);
        float moveSpeed = 300.f;
        float baseShootCooldown = 0.25f;
        int bulletDamageModifier = 1;
        switch (selectedShipSkin) {
        case 1: // defoult ship
            break;
        case 2: // Swift Scout
            moveSpeed = 350.f;
            baseShootCooldown = 0.15f;
            bulletDamageModifier = 1;
            break;
        case 3: // Heavy Cruiser
            moveSpeed = 250.f;
            baseShootCooldown = 0.40f;
            bulletDamageModifier = 1.5f;
            break;
        case 4: // Stealth Ship
            moveSpeed = 280.f;
            baseShootCooldown = 0.30f;
            bulletDamageModifier = 1.25f;
            break;
        case 5: // Assault Ship
            moveSpeed = 320.f;
            baseShootCooldown = 0.20f;
            bulletDamageModifier = 1.1f;
            break;
        case 6: // Guardian
            moveSpeed = 310.f;
            baseShootCooldown = 0.22f;
            bulletDamageModifier = 1.1f;
            break;
        case 7: // Tactical Ship
            moveSpeed = 270.f;
            baseShootCooldown = 0.35f;
            bulletDamageModifier = 1.35f;
            break;
        case 8: // Interceptor
            moveSpeed = 400.f;
            baseShootCooldown = 0.2f;
            bulletDamageModifier = 1;
            break;
        case 9: // Battle Frigate
            moveSpeed = 230.f;
            baseShootCooldown = 0.50f;
            bulletDamageModifier = 1.75f;
            break;
        case 10: // Dreadnought
            moveSpeed = 200.f;
            baseShootCooldown = 0.60f;
            bulletDamageModifier = 2;
            break;
        default:
            moveSpeed = 300.f;
            baseShootCooldown = 0.25f;
            bulletDamageModifier = 1;
            break;
        }
        if (!shootBuffer.loadFromFile("C:\\Users\\User\\Desktop\\SFML\\Sounds\\vistrel.wav") ||
            !asteroidDestroyBuffer.loadFromFile("C:\\Users\\User\\Desktop\\SFML\\Sounds\\bangLarge.wav") ||
            !explosionSoundBuffer.loadFromFile("C:\\Users\\User\\Desktop\\SFML\\Sounds\\explosion_sound.wav")) {
            cerr << "Error: sound files not found" << endl;
        }
        shootSound.setBuffer(shootBuffer);
        asteroidDestroySound.setBuffer(asteroidDestroyBuffer);
        explosionSound.setBuffer(explosionSoundBuffer);

        Sprite bg(backgroundTexture);
        bg.setScale(
            window.getSize().x / float(backgroundTexture.getSize().x),
            window.getSize().y / float(backgroundTexture.getSize().y)
        );


        bool isPaused = false;
        bool shouldExitToMenu = false;
        Clock shootClock, gameClock, asteroidClock, bonusSpawnClock;
        srand(static_cast<unsigned>(time(0)));

        Text scoreText("0", font, 80);
        scoreText.setFillColor(Color::White);
        scoreText.setPosition(
            window.getSize().x / 2 - scoreText.getGlobalBounds().width / 2,
            20
        );

        Text healthText("Health: 100", font, 30);
        healthText.setFillColor(Color::White);
        healthText.setPosition(20, 60);

        Text coinsText("Coins: " + to_string(userManager.getUserCoins()), font, 30);
        coinsText.setFillColor(Color::Yellow);
        coinsText.setPosition(20, 20);

        Text speedUpText("", font, 30);
        speedUpText.setFillColor(Color::White);
        speedUpText.setPosition(window.getSize().x / 2 - 150, window.getSize().y - 50);

        Text damageBuffText("", font, 25);
        damageBuffText.setFillColor(Color::Red);
        damageBuffText.setPosition(window.getSize().x - 200, 20);

        Text fireRateBuffText("", font, 25);
        fireRateBuffText.setFillColor(Color::Yellow);
        fireRateBuffText.setPosition(window.getSize().x - 200, 50);

        Text doubleShotText("", font, 25);
        doubleShotText.setFillColor(Color::Green);
        doubleShotText.setPosition(window.getSize().x - 200, 80);

        Text shieldText("", font, 25);
        shieldText.setFillColor(Color::Cyan);
        shieldText.setPosition(window.getSize().x - 200, 110);

        Text explosionPointsText("Explosion: 0/1", font, 25);
        explosionPointsText.setFillColor(Color::Red);
        explosionPointsText.setPosition(20, window.getSize().y - 50);

        while (window.isOpen() && health > 0 && (!timeMode || gameTime > 0) && !shouldExitToMenu) {
            Event e;
            while (window.pollEvent(e)) {
                if (e.type == Event::Closed) {
                    window.close();
                    return;
                }
                if (e.type == Event::KeyPressed && e.key.code == Keyboard::Escape) {
                    isPaused = true;
                }
                if (e.type == Event::KeyPressed && e.key.code == Keyboard::Space && explosionPoints >= 1) {
                    explosionActive = true;
                    explosionPoints--;
                    explosionCenter = ship.getPosition();
                    explosionRadius = 0.f;
                    explosionTimer.restart();
                    explosionPointsText.setString("Explosion: " + to_string(explosionPoints) + "/1");
                    explosionSound.play();
                }
            }
            if (isPaused) {
                int pauseResult = showPauseMenu(window, font);
                if (pauseResult == 1) {
                    isPaused = false;
                    gameClock.restart();
                }
                else if (pauseResult == 2) {
                    shouldExitToMenu = true;
                }
                else if (pauseResult == 0) {
                    window.close();
                    return;
                }
                continue;
            }

            float deltaTime = gameClock.restart().asSeconds();

            if (timeMode) {
                gameTime -= deltaTime;
                if (gameTime <= 0) break;
                timerText.setString("Time Left: " + to_string((int)gameTime));
            }
            else if (infiniteMode) {
                remainingTime -= deltaTime;
                if (remainingTime <= 0.0f) {
                    currentSpeedMultiplier += 0.15f;
                    increaseAsteroidsSpeed(currentSpeedMultiplier);
                    remainingTime = timerDuration;
                    speedUpText.setString("Asteroids speed increased!");
                }
                timerText.setString("Time Left: " + to_string((int)remainingTime));
            }

            if (bonusSpawnClock.getElapsedTime().asSeconds() >= 15.0f) {
                if (rand() % 100 < 20) {
                    Bonus::BonusType bonusType = static_cast<Bonus::BonusType>(rand() % 6);
                    bonuses.push_back(Bonus(
                        rand() % (window.getSize().x - 100) + 50, -50,
                        bonusType
                    ));
                }
                bonusSpawnClock.restart();
            }

            Vector2f movement(0, 0);
            if (Keyboard::isKeyPressed(Keyboard::W)) movement.y -= moveSpeed * deltaTime;
            if (Keyboard::isKeyPressed(Keyboard::S)) movement.y += moveSpeed * deltaTime;
            if (Keyboard::isKeyPressed(Keyboard::A)) movement.x -= moveSpeed * deltaTime;
            if (Keyboard::isKeyPressed(Keyboard::D)) movement.x += moveSpeed * deltaTime;

            ship.setPosition(
                clamp(ship.getPosition().x + movement.x, 0.f, float(window.getSize().x)),
                clamp(ship.getPosition().y + movement.y, 0.f, float(window.getSize().y))
            );

            Vector2f mousePos(Mouse::getPosition(window));
            float angle = atan2(mousePos.y - ship.getPosition().y,
                mousePos.x - ship.getPosition().x) * 180.f / 3.14159f - 270;
            ship.setRotation(angle);

            float currentShootCooldown = baseShootCooldown * fireRateMultiplier;
            if (Mouse::isButtonPressed(Mouse::Left) && shootClock.getElapsedTime().asSeconds() >= currentShootCooldown) {
                const Texture& bulletTex = (damageMultiplier > 1.0f) ? bulletTexture2 : bulletTexture;

                if (doubleShotActive) {
                    Bullet bullet1(ship.getPosition().x, ship.getPosition().y, angle + 15, bulletTex);
                    Bullet bullet2(ship.getPosition().x, ship.getPosition().y, angle - 15, bulletTex);

                    bullet1.damage = static_cast<int>(bullet1.damage * bulletDamageModifier * damageMultiplier);
                    bullet2.damage = static_cast<int>(bullet2.damage * bulletDamageModifier * damageMultiplier);
                    bullets.push_back(bullet1);
                    bullets.push_back(bullet2);
                }
                else {
                    Bullet newBullet(ship.getPosition().x, ship.getPosition().y, angle, bulletTex);
                    newBullet.damage = static_cast<int>(newBullet.damage * bulletDamageModifier * damageMultiplier);
                    bullets.push_back(newBullet);
                }
                shootSound.play();
                shootClock.restart();
            }

            bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [&window](const Bullet& b) {
                const Vector2f& pos = b.sprite.getPosition();
                return pos.x < -100 || pos.x > window.getSize().x + 100 ||
                    pos.y < -100 || pos.y > window.getSize().y + 100;
                }), bullets.end());

            for (auto& bullet : bullets) {
                bullet.update(deltaTime);
            }

            if (asteroidClock.getElapsedTime().asSeconds() >= currentAsteroidSpawnTime) {
                float spawnX, spawnY;
                int side = rand() % 4;

                switch (side) {
                case 0: spawnX = rand() % window.getSize().x; spawnY = -50; break;
                case 1: spawnX = window.getSize().x + 50; spawnY = rand() % window.getSize().y; break;
                case 2: spawnX = rand() % window.getSize().x; spawnY = window.getSize().y + 50; break;
                case 3: spawnX = -50; spawnY = rand() % window.getSize().y; break;
                }

                Vector2f center(window.getSize().x / 2, window.getSize().y / 2);
                Vector2f direction = center - Vector2f(spawnX, spawnY);
                float baseAngle = atan2(direction.y, direction.x) * 180.f / 3.14159f;
                float angleVariation = (rand() % 60) - 30;
                float finalAngle = baseAngle + angleVariation;

                Asteroid newAsteroid(spawnX, spawnY, rand() % 3, finalAngle);
                if (newAsteroid.sprite.getTexture() == &asteroidGreenTexture) {
                    newAsteroid.setSpeed(185.f * currentSpeedMultiplier);
                }
                else if (newAsteroid.sprite.getTexture() == &asteroidBlueTexture) {
                    newAsteroid.setSpeed(135.f * currentSpeedMultiplier);
                }
                else if (newAsteroid.sprite.getTexture() == &asteroidRedTexture) {
                    newAsteroid.setSpeed(100.f * currentSpeedMultiplier);
                }
                asteroids.push_back(newAsteroid);
                asteroidClock.restart();
            }

            for (auto& asteroid : asteroids) asteroid.update(deltaTime);
            asteroids.erase(remove_if(asteroids.begin(), asteroids.end(), [&window](const Asteroid& a) {
                Vector2f pos = a.sprite.getPosition();
                return pos.x < -200 || pos.x > window.getSize().x + 200 ||
                    pos.y < -200 || pos.y > window.getSize().y + 200;
                }), asteroids.end());

            for (auto& bonus : bonuses) bonus.update(deltaTime);
            bonuses.erase(remove_if(bonuses.begin(), bonuses.end(), [&window](const Bonus& b) {
                return b.sprite.getPosition().y > window.getSize().y + 50;
                }), bonuses.end());

            for (size_t i = 0; i < bonuses.size(); ++i) {
                if (ship.getGlobalBounds().intersects(bonuses[i].sprite.getGlobalBounds())) {
                    switch (bonuses[i].type) {
                    case Bonus::DAMAGE:
                        damageMultiplier = 2.0f;
                        damageBuffTimer.restart();
                        break;
                    case Bonus::FIRE_RATE:
                        fireRateMultiplier = 0.5f;
                        fireRateBuffTimer.restart();
                        break;
                    case Bonus::HEALTH:
                        health = min(100, health + 30);
                        break;
                    case Bonus::DOUBLE_SHOT:
                        doubleShotActive = true;
                        doubleShotTimer.restart();
                        break;
                    case Bonus::EXPLOSION_POINT:
                        explosionPoints = min(1, explosionPoints + 1);
                        explosionPointsText.setString("Explosion: " + to_string(explosionPoints) + "/1");
                        break;
                    case Bonus::SHIELD:
                        shieldActive = true;
                        shieldHits = 0;
                        shieldTimer.restart();
                        break;
                    }
                    bonuses.erase(bonuses.begin() + i--);
                    break;
                }
            }
            for (size_t i = 0; i < bullets.size(); ++i) {
                for (size_t j = 0; j < asteroids.size(); ++j) {
                    if (isCirclesColliding(
                        bullets[i].sprite.getPosition(),
                        bullets[i].sprite.getTexture()->getSize().x / 2.0f * 0.4f,
                        asteroids[j].getCenter(),
                        asteroids[j].getRadius())) {
                        asteroids[j].takeDamage(bullets[i].damage);
                        if (asteroids[j].health <= 0) {
                            score += asteroids[j].calculateScore();
                            userManager.addUserCoins(asteroids[j].calculateCoins());
                            coinsText.setString("Coins: " + to_string(userManager.getUserCoins()));

                            asteroidDestroySound.play();
                            scoreText.setString(to_string(score));
                            scoreText.setPosition(
                                window.getSize().x / 2 - scoreText.getGlobalBounds().width / 2,
                                20
                            );

                            if (rand() % 100 < 20) {
                                Bonus::BonusType bonusType = static_cast<Bonus::BonusType>(rand() % 6);
                                bonuses.push_back(Bonus(asteroids[j].getCenter().x,
                                    asteroids[j].getCenter().y,
                                    bonusType));
                            }
                            asteroids.erase(asteroids.begin() + j);
                        }
                        bullets.erase(bullets.begin() + i--);
                        break;
                    }
                }
            }

            for (size_t j = 0; j < asteroids.size(); ++j) {
                if (isCirclesColliding(
                    ship.getPosition(),
                    ship.getTexture()->getSize().x / 2.0f * 0.5,
                    asteroids[j].getCenter(),
                    asteroids[j].getRadius())) {

                    if (shieldActive) {
                        shieldHits++;
                        score += asteroids[j].calculateScore();
                        userManager.addUserCoins(asteroids[j].calculateCoins());
                        coinsText.setString("Coins: " + to_string(userManager.getUserCoins()));
                        asteroidDestroySound.play();

                        if (shieldHits >= 2) {
                            shieldActive = false;
                            shieldText.setString("");
                        }
                    }
                    else {
                        if (asteroids[j].sprite.getTexture() == &asteroidGreenTexture) health -= 20;
                        else if (asteroids[j].sprite.getTexture() == &asteroidBlueTexture) health -= 30;
                        else if (asteroids[j].sprite.getTexture() == &asteroidRedTexture) health -= 50;
                    }

                    scoreText.setString(to_string(score));
                    scoreText.setPosition(
                        window.getSize().x / 2 - scoreText.getGlobalBounds().width / 2,
                        20
                    );

                    asteroids.erase(asteroids.begin() + j--);
                }
            }



            if (shieldActive) {
                float shieldRadius = ship.getTexture()->getSize().x / 2.0f * 0.6;
                CircleShape shield(shieldRadius);
                shield.setFillColor(Color(0, 100, 255, 70));
                shield.setOrigin(shieldRadius, shieldRadius);
                shield.setPosition(ship.getPosition());
                window.draw(shield);
            }

            if (explosionActive) {
                float elapsed = explosionTimer.getElapsedTime().asSeconds();
                if (elapsed < EXPLOSION_DURATION) {
                    explosionRadius = 300.f * (elapsed / EXPLOSION_DURATION);

                    for (size_t j = 0; j < asteroids.size(); ++j) {
                        float distance = sqrt(pow(asteroids[j].getCenter().x - explosionCenter.x, 2) +
                            pow(asteroids[j].getCenter().y - explosionCenter.y, 2));
                        if (distance < explosionRadius) {
                            score += asteroids[j].calculateScore();
                            userManager.addUserCoins(asteroids[j].calculateCoins());
                            coinsText.setString("Coins: " + to_string(userManager.getUserCoins()));
                            asteroidDestroySound.play();
                            scoreText.setString(to_string(score));
                            scoreText.setPosition(
                                window.getSize().x / 2 - scoreText.getGlobalBounds().width / 2,
                                20
                            );
                            asteroids.erase(asteroids.begin() + j--);
                        }
                    }
                }
                else {
                    explosionActive = false;
                }
            }


            if (damageBuffTimer.getElapsedTime().asSeconds() > BUFF_DURATION) {
                damageMultiplier = 1.0f;
            }
            if (fireRateBuffTimer.getElapsedTime().asSeconds() > BUFF_DURATION) {
                fireRateMultiplier = 1.0f;
            }
            if (doubleShotTimer.getElapsedTime().asSeconds() > DOUBLE_SHOT_DURATION) {
                doubleShotActive = false;
            }
            if (shieldActive && shieldTimer.getElapsedTime().asSeconds() > SHIELD_DURATION) {
                shieldActive = false;
            }

            healthText.setString("Health: " + to_string(health));


            if (damageMultiplier > 1.0f) {
                float remaining = BUFF_DURATION - damageBuffTimer.getElapsedTime().asSeconds();
                damageBuffText.setString("Damage x2: " + to_string((int)remaining) + "s");
            }
            else {
                damageBuffText.setString("");
            }

            if (fireRateMultiplier < 1.0f) {
                float remaining = BUFF_DURATION - fireRateBuffTimer.getElapsedTime().asSeconds();
                fireRateBuffText.setString("Fire Rate +: " + to_string((int)remaining) + "s");
            }
            else {
                fireRateBuffText.setString("");
            }

            if (doubleShotActive) {
                float remaining = DOUBLE_SHOT_DURATION - doubleShotTimer.getElapsedTime().asSeconds();
                doubleShotText.setString("Double Shot: " + to_string((int)remaining) + "s");
            }
            else {
                doubleShotText.setString("");
            }

            if (shieldActive) {
                float remaining = SHIELD_DURATION - shieldTimer.getElapsedTime().asSeconds();
                shieldText.setString("Shield: " + to_string((int)remaining) + "s");
            }
            else {
                shieldText.setString("");
            }

            window.clear();
            window.draw(bg);

            for (auto& asteroid : asteroids) {
                CircleShape hitbox(asteroid.getRadius());
                hitbox.setFillColor(Color(255, 0, 0, 100));
                hitbox.setOrigin(asteroid.getRadius(), asteroid.getRadius());
                hitbox.setPosition(asteroid.getCenter());
                window.draw(hitbox);
            }

            if (explosionActive) {
                CircleShape explosion(explosionRadius);
                explosion.setFillColor(Color(255, 165, 0, 150));
                explosion.setOrigin(explosionRadius, explosionRadius);
                explosion.setPosition(explosionCenter);
                window.draw(explosion);
            }


            if (shieldActive) {
                CircleShape shield(ship.getTexture()->getSize().x / 2.0f * 0.5f);
                shield.setFillColor(Color(0, 100, 255, 70));
                shield.setOrigin(shield.getRadius(), shield.getRadius());
                shield.setPosition(ship.getPosition());
                window.draw(shield);
            }

            for (auto& bullet : bullets) window.draw(bullet.sprite);
            for (auto& asteroid : asteroids) window.draw(asteroid.sprite);
            for (auto& bonus : bonuses) window.draw(bonus.sprite);
            window.draw(ship);

            window.draw(scoreText);
            window.draw(healthText);
            window.draw(timerText);
            window.draw(speedUpText);
            window.draw(damageBuffText);
            window.draw(fireRateBuffText);
            window.draw(doubleShotText);
            window.draw(shieldText);
            window.draw(coinsText);
            window.draw(explosionPointsText);

            window.display();
        }
        coinsText.setString("Coins: " + to_string(userManager.getUserCoins()));
        if (window.isOpen() && !shouldExitToMenu) {
            if (timeMode && gameTime <= 0) {
                Text timeUpText("TIME'S UP!", font, 70);
                timeUpText.setFillColor(Color::Red);
                timeUpText.setPosition(window.getSize().x / 2 - timeUpText.getGlobalBounds().width / 2,
                    window.getSize().y / 2 - 150);

                window.clear();
                window.draw(bg);
                window.draw(timeUpText);
                window.draw(scoreText);
                window.display();
                sf::sleep(seconds(2));
            }

            restart = showGameOverWithName(window, font, userManager, score, timeMode);
        }
        else {
            restart = false;
        }
    }
}

bool showAuthWindow(RenderWindow& window, Font& font, UserManager& userManager) {
    enum class AuthMode { Login, Register } currentMode = AuthMode::Login;
    enum class ActiveField { Login, Password, Nickname } activeField = ActiveField::Login;

    Sprite bg(authBackgroundTexture);
    bg.setScale(
        window.getSize().x / float(authBackgroundTexture.getSize().x),
        window.getSize().y / float(authBackgroundTexture.getSize().y)
    );

    string login, password, nickname;
    Text loginText("", font, 30), passwordText("", font, 30), nicknameText("", font, 30);
    Text loginLabel("Login:", font, 30), passwordLabel("Password:", font, 30), nicknameLabel("Nickname:", font, 30);
    Text modeText("[TAB] Switch mode", font, 25);
    Text actionText("Press ENTER to Login", font, 30);
    Text errorText("", font, 25);

    RectangleShape loginField(Vector2f(300, 40)), passwordField(Vector2f(300, 40)), nicknameField(Vector2f(300, 40));
    loginField.setFillColor(Color(70, 70, 70, 200));
    passwordField.setFillColor(Color(70, 70, 70, 200));
    nicknameField.setFillColor(Color(70, 70, 70, 200));
    loginField.setOutlineThickness(2);
    passwordField.setOutlineThickness(2);
    nicknameField.setOutlineThickness(2);

    float centerX = window.getSize().x / 2 - 200;
    float fieldX = centerX + 120;

    loginLabel.setPosition(centerX, 200);

    passwordLabel.setPosition(centerX - 70, 260);

    nicknameLabel.setPosition(centerX - 70, 320);
    loginText.setPosition(fieldX + 20, 200);

    passwordText.setPosition(fieldX + 20, 260); //ввод текста

    nicknameText.setPosition(fieldX + 20, 320);
    loginField.setPosition(fieldX + 20, 195);
    passwordField.setPosition(fieldX + 20, 255);
    nicknameField.setPosition(fieldX + 20, 315);

    modeText.setPosition(centerX, 400);
    actionText.setPosition(centerX, 450);
    errorText.setPosition(centerX, 500);

    loginLabel.setFillColor(Color::Yellow);
    loginField.setOutlineColor(Color::Yellow);
    passwordLabel.setFillColor(Color::White);
    passwordField.setOutlineColor(Color::Transparent);
    nicknameLabel.setFillColor(Color::White);
    nicknameField.setOutlineColor(Color::Transparent);

    nicknameLabel.setFillColor(Color::Transparent);
    nicknameText.setFillColor(Color::Transparent);
    nicknameField.setFillColor(Color::Transparent);
    nicknameField.setOutlineColor(Color::Transparent);

    bool authenticated = false;
    while (window.isOpen() && !authenticated) {
        Event e;
        while (window.pollEvent(e)) {
            if (e.type == Event::Closed) {
                window.close();
                return false;
            }
            if (e.type == Event::KeyPressed && e.key.code == Keyboard::Escape) {
                window.close();
                return false;
            }

            if (e.type == Event::KeyPressed) {
                if (e.key.code == Keyboard::Tab) {
                    currentMode = (currentMode == AuthMode::Login) ? AuthMode::Register : AuthMode::Login;
                    actionText.setString(currentMode == AuthMode::Register ?
                        "Press ENTER to Register" : "Press ENTER to Login");
                    actionText.setFillColor(currentMode == AuthMode::Register ? Color::Cyan : Color::Green);
                    errorText.setString("");

                    if (currentMode == AuthMode::Register) {
                        nicknameLabel.setFillColor(activeField == ActiveField::Nickname ? Color::Yellow : Color::White);
                        nicknameText.setFillColor(Color::White);
                        nicknameField.setFillColor(Color(70, 70, 70, 200));
                    }
                    else {
                        nicknameLabel.setFillColor(Color::Transparent);
                        nicknameText.setFillColor(Color::Transparent);
                        nicknameField.setFillColor(Color::Transparent);
                        nicknameField.setOutlineColor(Color::Transparent);
                        if (activeField == ActiveField::Nickname) activeField = ActiveField::Password;
                    }
                }
                else if (e.key.code == Keyboard::Up) {
                    if (currentMode == AuthMode::Register) {
                        if (activeField == ActiveField::Nickname) activeField = ActiveField::Password;
                        else if (activeField == ActiveField::Password) activeField = ActiveField::Login;
                    }
                    else {
                        if (activeField == ActiveField::Password) activeField = ActiveField::Login;
                    }

                    loginLabel.setFillColor(activeField == ActiveField::Login ? Color::Yellow : Color::White);
                    passwordLabel.setFillColor(activeField == ActiveField::Password ? Color::Yellow : Color::White);
                    loginField.setOutlineColor(activeField == ActiveField::Login ? Color::Yellow : Color::Transparent);
                    passwordField.setOutlineColor(activeField == ActiveField::Password ? Color::Yellow : Color::Transparent);

                    if (currentMode == AuthMode::Register) {
                        nicknameLabel.setFillColor(activeField == ActiveField::Nickname ? Color::Yellow : Color::White);
                        nicknameField.setOutlineColor(activeField == ActiveField::Nickname ? Color::Yellow : Color::Transparent);
                    }
                }
                else if (e.key.code == Keyboard::Down) {
                    if (currentMode == AuthMode::Register) {
                        if (activeField == ActiveField::Login) activeField = ActiveField::Password;
                        else if (activeField == ActiveField::Password) activeField = ActiveField::Nickname;
                    }
                    else {
                        if (activeField == ActiveField::Login) activeField = ActiveField::Password;
                    }

                    loginLabel.setFillColor(activeField == ActiveField::Login ? Color::Yellow : Color::White);
                    passwordLabel.setFillColor(activeField == ActiveField::Password ? Color::Yellow : Color::White);
                    loginField.setOutlineColor(activeField == ActiveField::Login ? Color::Yellow : Color::Transparent);
                    passwordField.setOutlineColor(activeField == ActiveField::Password ? Color::Yellow : Color::Transparent);

                    if (currentMode == AuthMode::Register) {
                        nicknameLabel.setFillColor(activeField == ActiveField::Nickname ? Color::Yellow : Color::White);
                        nicknameField.setOutlineColor(activeField == ActiveField::Nickname ? Color::Yellow : Color::Transparent);
                    }
                }
                else if (e.key.code == Keyboard::Enter) {
                    if (login.empty() || password.empty() || (currentMode == AuthMode::Register && nickname.empty())) {
                        errorText.setString("All fields must be filled!");
                    }
                    else if (currentMode == AuthMode::Login) {
                        authenticated = userManager.loginUser(login, password);
                        if (!authenticated) errorText.setString("Invalid login or password!");
                    }
                    else {
                        if (userManager.registerUser(login, password, nickname)) {
                            errorText.setString("Registration successful!");
                            currentMode = AuthMode::Login;
                            actionText.setString("Press ENTER to Login");
                            actionText.setFillColor(Color::Green);

                            nicknameLabel.setFillColor(Color::Transparent);
                            nicknameText.setFillColor(Color::Transparent);
                            nicknameField.setFillColor(Color::Transparent);
                            nicknameField.setOutlineColor(Color::Transparent);
                            if (activeField == ActiveField::Nickname) activeField = ActiveField::Password;
                        }
                        else {
                            errorText.setString("User already exists!");
                        }
                    }
                }
                else if (e.key.code == Keyboard::BackSpace) {
                    if (activeField == ActiveField::Password && !password.empty()) {
                        password.pop_back();
                        passwordText.setString(string(password.size(), '*'));
                    }
                    else if (activeField == ActiveField::Login && !login.empty()) {
                        login.pop_back();
                        loginText.setString(login);
                    }
                    else if (activeField == ActiveField::Nickname && !nickname.empty()) {
                        nickname.pop_back();
                        nicknameText.setString(nickname);
                    }
                }
            }

            if (e.type == Event::TextEntered && e.text.unicode != '\t' && e.text.unicode != '\r' && e.text.unicode != '\b') {
                if (activeField == ActiveField::Password && password.size() < 20) {
                    password += static_cast<char>(e.text.unicode);
                    passwordText.setString(string(password.size(), '*'));
                }
                else if (activeField == ActiveField::Login && login.size() < 15) {
                    login += static_cast<char>(e.text.unicode);
                    loginText.setString(login);
                }
                else if (activeField == ActiveField::Nickname && nickname.size() < 15) {
                    nickname += static_cast<char>(e.text.unicode);
                    nicknameText.setString(nickname);
                }
            }

            if (e.type == Event::MouseButtonPressed) {
                if (loginField.getGlobalBounds().contains(e.mouseButton.x, e.mouseButton.y)) {
                    activeField = ActiveField::Login;
                    loginLabel.setFillColor(Color::Yellow);
                    passwordLabel.setFillColor(Color::White);
                    loginField.setOutlineColor(Color::Yellow);
                    passwordField.setOutlineColor(Color::Transparent);
                    if (currentMode == AuthMode::Register) {
                        nicknameLabel.setFillColor(Color::White);
                        nicknameField.setOutlineColor(Color::Transparent);
                    }
                }
                else if (passwordField.getGlobalBounds().contains(e.mouseButton.x, e.mouseButton.y)) {
                    activeField = ActiveField::Password;
                    loginLabel.setFillColor(Color::White);
                    passwordLabel.setFillColor(Color::Yellow);
                    loginField.setOutlineColor(Color::Transparent);
                    passwordField.setOutlineColor(Color::Yellow);
                    if (currentMode == AuthMode::Register) {
                        nicknameLabel.setFillColor(Color::White);
                        nicknameField.setOutlineColor(Color::Transparent);
                    }
                }
                else if (currentMode == AuthMode::Register && nicknameField.getGlobalBounds().contains(e.mouseButton.x, e.mouseButton.y)) {
                    activeField = ActiveField::Nickname;
                    loginLabel.setFillColor(Color::White);
                    passwordLabel.setFillColor(Color::White);
                    loginField.setOutlineColor(Color::Transparent);
                    passwordField.setOutlineColor(Color::Transparent);
                    nicknameLabel.setFillColor(Color::Yellow);
                    nicknameField.setOutlineColor(Color::Yellow);
                }
            }
        }

        window.clear();
        window.draw(bg);
        window.draw(loginField);
        window.draw(passwordField);
        window.draw(loginLabel);
        window.draw(passwordLabel);
        window.draw(loginText);
        window.draw(passwordText);

        if (currentMode == AuthMode::Register) {
            window.draw(nicknameField);
            window.draw(nicknameLabel);
            window.draw(nicknameText);
        }

        window.draw(modeText);
        window.draw(actionText);
        window.draw(errorText);
        window.display();
    }
    return authenticated;
}

int main() {
    if (!bulletTexture.loadFromFile("C:\\Users\\User\\Desktop\\SFML\\Skins\\bullet1.png") ||
        !asteroidGreenTexture.loadFromFile("C:\\Users\\User\\Desktop\\SFML\\Enemyes\\Green.png") ||
        !asteroidBlueTexture.loadFromFile("C:\\Users\\User\\Desktop\\SFML\\Enemyes\\Blue.png") ||
        !asteroidRedTexture.loadFromFile("C:\\Users\\User\\Desktop\\SFML\\Enemyes\\Red.png") ||
        !shipTexture1.loadFromFile("C:\\Users\\User\\Desktop\\SFML\\Skins\\spaceship1.png") ||
        !shipTexture2.loadFromFile("C:\\Users\\User\\Desktop\\SFML\\Skins\\spaceship2.png") ||
        !shipTexture3.loadFromFile("C:\\Users\\User\\Desktop\\SFML\\Skins\\spaceship3.png") ||
        !shipTexture4.loadFromFile("C:\\Users\\User\\Desktop\\SFML\\Skins\\spaceship4.png") ||
        !shipTexture5.loadFromFile("C:\\Users\\User\\Desktop\\SFML\\Skins\\spaceship5.png") ||
        !shipTexture6.loadFromFile("C:\\Users\\User\\Desktop\\SFML\\Skins\\spaceship6.png") ||
        !shipTexture7.loadFromFile("C:\\Users\\User\\Desktop\\SFML\\Skins\\spaceship7.png") ||
        !shipTexture8.loadFromFile("C:\\Users\\User\\Desktop\\SFML\\Skins\\spaceship8.png") ||
        !shipTexture9.loadFromFile("C:\\Users\\User\\Desktop\\SFML\\Skins\\spaceship9.png") ||
        !shipTexture10.loadFromFile("C:\\Users\\User\\Desktop\\SFML\\Skins\\spaceship10.png") ||
        !shopMusic.openFromFile(musicFiles[currentMusicIndex]) ||
        !lockTexture.loadFromFile("C:\\Users\\User\\Desktop\\SFML\\Skins\\lock.png") ||
        !backgroundTexture.loadFromFile("C:\\Users\\User\\Desktop\\SFML\\Backgrounds\\321.png") ||
        !shopbackgroundTexture.loadFromFile("C:\\Users\\User\\Desktop\\SFML\\Backgrounds\\shop.png") ||
        !menuBackgroundTexture.loadFromFile("C:\\Users\\User\\Desktop\\SFML\\Backgrounds\\123.png") ||
        !settingsBackgroundTexture.loadFromFile("C:\\Users\\User\\Desktop\\SFML\\Backgrounds\\123.png") ||
        !leaderboardBackgroundTexture.loadFromFile("C:\\Users\\User\\Desktop\\SFML\\Backgrounds\\123.png") ||
        !authBackgroundTexture.loadFromFile("C:\\Users\\User\\Desktop\\SFML\\Backgrounds\\123.png") ||
        !damageBuffTexture.loadFromFile("C:\\Users\\User\\Desktop\\SFML\\Buffs\\damage.png") ||
        !errorSoundBuffer.loadFromFile("C:\\Users\\User\\Desktop\\SFML\\Sounds\\error.mp3") ||
        !fireRateBuffTexture.loadFromFile("C:\\Users\\User\\Desktop\\SFML\\Buffs\\shotspeed.png") ||
        !doubleShotBuffTexture.loadFromFile("C:\\Users\\User\\Desktop\\SFML\\Buffs\\double_shot.png") ||
        !explosionPointTexture.loadFromFile("C:\\Users\\User\\Desktop\\SFML\\Buffs\\explosion.png") ||
        !asteroidDestroyBuffer.loadFromFile("C:\\Users\\User\\Desktop\\SFML\\Sounds\\explosion_sound.wav") ||
        !bulletTexture2.loadFromFile("C:\\Users\\User\\Desktop\\SFML\\Skins\\bullet2.png") ||
        !healthPackTexture.loadFromFile("C:\\Users\\User\\Desktop\\SFML\\Buffs\\medic_bag.png") ||
        !explosionSoundBuffer.loadFromFile("C:\\Users\\User\\Desktop\\SFML\\Sounds\\explosion_sound.wav") ||
        !shieldBuffTexture.loadFromFile("C:\\Users\\User\\Desktop\\SFML\\Buffs\\shield.png") ||
        !shootBuffer.loadFromFile("C:\\Users\\User\\Desktop\\SFML\\Sounds\\vistrel.wav")) {
        cerr << "Error loading textures or sounds!" << endl;
        return -1;
    }
    errorSound.setBuffer(errorSoundBuffer);
    loadSettings();

    loadSelectedSkin();

    VideoMode desktop = VideoMode::getDesktopMode();
    RenderWindow window(desktop, "Asteroids", Style::Fullscreen);

    Font font;
    if (!font.loadFromFile("C:\\Users\\User\\Desktop\\SFML\\Fonts\\RuneScape-ENA.ttf")) {
        cerr << "Error: font file not found." << endl;
        return -1;
    }

    UserManager userManager;
    if (!showAuthWindow(window, font, userManager)) {
        return 0;
    }

    Music backgroundMusic;
    if (!backgroundMusic.openFromFile(musicFiles[currentMusicIndex])) {
        cerr << "Error: music file not found." << endl;
        return -1;
    }
    backgroundMusic.setLoop(true);
    backgroundMusic.setVolume(musicVolume);
    backgroundMusic.play();

    shootSound.setBuffer(shootBuffer);
    asteroidDestroySound.setBuffer(asteroidDestroyBuffer);
    explosionSound.setBuffer(explosionSoundBuffer);


    shootSound.setVolume(soundVolume);
    asteroidDestroySound.setVolume(soundVolume);
    explosionSound.setVolume(soundVolume);

    loadGlobalHighscores();

    Sprite menuBg(menuBackgroundTexture);
    menuBg.setScale(
        window.getSize().x / float(menuBackgroundTexture.getSize().x),
        window.getSize().y / float(menuBackgroundTexture.getSize().y)
    );

    Text userText("User: " + userManager.getCurrentNickname(), font, 20);
    userText.setFillColor(Color::White);
    userText.setPosition(20, window.getSize().y - 50);

    Text highScoreText("Best (Infinite): " + to_string(userManager.getUserInfiniteModeHighScore()) +
        "\nBest (Time): " + to_string(userManager.getUserTimeModeHighScore()), font, 20);
    highScoreText.setFillColor(Color::Yellow);
    highScoreText.setPosition(20, userText.getPosition().y - 50);

    Text coinsText("Coins: " + to_string(userManager.getUserCoins()), font, 20);
    coinsText.setFillColor(Color::Yellow);
    coinsText.setPosition(20, highScoreText.getPosition().y - 20);


    vector<RectangleShape> menuButtons;
    vector<Text> menuButtonTexts;
    vector<string> options = { "Play", "Skins", "Settings", "Leaderboard", "Exit" };

    for (size_t i = 0; i < options.size(); ++i) {
        RectangleShape button(Vector2f(300, 60));
        button.setFillColor(Color(70, 70, 70, 200));
        button.setOutlineThickness(2);
        button.setOutlineColor(Color::White);
        button.setPosition(
            window.getSize().x / 2 - 150,
            window.getSize().y / 2 - 150 + (i * 100)
        );
        menuButtons.push_back(button);

        Text text(options[i], font, 30);
        text.setFillColor(Color::White);
        FloatRect textRect = text.getLocalBounds();
        text.setOrigin(textRect.left + textRect.width / 2.0f,
            textRect.top + textRect.height / 2.0f);
        text.setPosition(
            button.getPosition().x + button.getSize().x / 2,
            button.getPosition().y + button.getSize().y / 2
        );
        menuButtonTexts.push_back(text);
    }
    Text gameTitleShadow("ASTEROIDS", font, 100);
    gameTitleShadow.setFillColor(Color(70, 70, 70, 200));
    gameTitleShadow.setStyle(Text::Bold);
    gameTitleShadow.setPosition(
        window.getSize().x / 2 - gameTitleShadow.getGlobalBounds().width / 2 + 5,
        55
    );

    Text gameTitle("ASTEROIDS", font, 100);
    gameTitle.setFillColor(Color::White);
    gameTitle.setStyle(Text::Bold);
    gameTitle.setPosition(
        window.getSize().x / 2 - gameTitle.getGlobalBounds().width / 2,
        50
    );
    while (window.isOpen()) {
        Event e;
        while (window.pollEvent(e)) {
            if (e.type == Event::Closed) window.close();
            if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left) {
                for (size_t i = 0; i < menuButtons.size(); ++i) {
                    if (menuButtons[i].getGlobalBounds().contains(e.mouseButton.x, e.mouseButton.y)) {
                        if (options[i] == "Play") gameLoop(window, font, userManager);
                        else if (options[i] == "Skins") showSkinSelection(window, font, backgroundMusic, userManager);
                        else if (options[i] == "Settings") showSettings(window, font, backgroundMusic, shootSound, asteroidDestroySound, explosionSound);
                        else if (options[i] == "Leaderboard") showLeaderboard(window, font);
                        else if (options[i] == "Exit" && confirmExit(window, font, userManager)) window.close();
                    }
                }
            }
        }

        Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));
        for (size_t i = 0; i < menuButtons.size(); ++i) {
            if (menuButtons[i].getGlobalBounds().contains(mousePos)) {
                menuButtons[i].setFillColor(Color(100, 100, 100, 200));
                menuButtonTexts[i].setFillColor(Color::Yellow);
            }
            else {
                menuButtons[i].setFillColor(Color(70, 70, 70, 200));
                menuButtonTexts[i].setFillColor(Color::White);
            }
        }

        highScoreText.setString("Best (Infinite): " + to_string(userManager.getUserInfiniteModeHighScore()) +
            "\nBest (Time): " + to_string(userManager.getUserTimeModeHighScore()));
        coinsText.setString("Coins: " + to_string(userManager.getUserCoins()));
        window.clear();
        window.draw(menuBg);

        window.draw(gameTitleShadow);
        window.draw(gameTitle);

        for (size_t i = 0; i < menuButtons.size(); ++i) {
            window.draw(menuButtons[i]);
            window.draw(menuButtonTexts[i]);
        }

        window.draw(userText);
        window.draw(coinsText);
        window.draw(highScoreText);
        window.display();
    }

    return 0;
}