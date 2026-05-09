#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>       // FIX 1: مطلوب لـ srand
#include <string>
using namespace sf;
using namespace std;

const int    WINDOW_WIDTH    = 800;
const int    WINDOW_HEIGHT   = 600;
const string WINDOW_TITLE    = "Dodge The Falling Blocks";
const string FONT_FILE       = "C:\\Users\\engal\\Desktop\\app\\assets\\arial.ttf";
const string GAMEOVER_SOUND  = "C:\\Users\\engal\\Desktop\\app\\assets\\Ramadan.mp3";
const float  PLAYER_SPEED    = 300.0f;
const float  PLAYER_WIDTH    = 50.0f;
const float  PLAYER_HEIGHT   = 50.0f;
const float  OBSTACLE_SPEED  = 200.0f;
const float  OBSTACLE_WIDTH  = 50.0f;
const float  OBSTACLE_HEIGHT = 50.0f;
const float  SPAWN_INTERVAL  = 0.8f;
const string GAMEOVER_STRING = "Game Over!";
const int    GAMEOVER_SIZE   = 50;
const float  GAMEOVER_X      = 260.f;
const float  GAMEOVER_Y      = 260.f;

class Player {
public:
    RectangleShape shape;

    Player() {
        shape.setSize(Vector2f(PLAYER_WIDTH, PLAYER_HEIGHT));
        shape.setFillColor(Color::Red);
        shape.setPosition(WINDOW_WIDTH / 2.f - PLAYER_WIDTH / 2.f, WINDOW_HEIGHT - PLAYER_HEIGHT - 10.f);
    }

    void handleInput(float deltaTime) {
        if (Keyboard::isKeyPressed(Keyboard::Left))  shape.move(-PLAYER_SPEED * deltaTime, 0);
        if (Keyboard::isKeyPressed(Keyboard::Right)) shape.move( PLAYER_SPEED * deltaTime, 0);
        if (shape.getPosition().x < 0)
            shape.setPosition(0, shape.getPosition().y);
        if (shape.getPosition().x + PLAYER_WIDTH > WINDOW_WIDTH)
            shape.setPosition(WINDOW_WIDTH - PLAYER_WIDTH, shape.getPosition().y);
    }

    void draw(RenderWindow& window) { window.draw(shape); }
};

class Obstacle {
public:
    RectangleShape shape;

    Obstacle(float x, float y) {
        shape.setSize(Vector2f(OBSTACLE_WIDTH, OBSTACLE_HEIGHT));
        shape.setFillColor(Color::Green);
        shape.setPosition(x, y);
    }

    void update(float deltaTime)            { shape.move(0, OBSTACLE_SPEED * deltaTime); }
    bool isOffScreen()                      { return shape.getPosition().y > WINDOW_HEIGHT; }
    bool checkCollision(RectangleShape& p)  { return shape.getGlobalBounds().intersects(p.getGlobalBounds()); }
    void draw(RenderWindow& window)         { window.draw(shape); }
};

void initWindow(RenderWindow& window) {
    window.create(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), WINDOW_TITLE);
    window.setFramerateLimit(60);   // FIX 2: حد الـ FPS
}

bool loadFont(Font& font) {
    return font.loadFromFile(FONT_FILE);
}

void initGameOverText(Text& text, Font& font) {
    text.setFont(font);
    text.setString(GAMEOVER_STRING);
    text.setCharacterSize(GAMEOVER_SIZE);
    text.setFillColor(Color::Red);
    text.setPosition(GAMEOVER_X, GAMEOVER_Y);
}

bool loadSound(Music& music) {
    return music.openFromFile(GAMEOVER_SOUND);
}

void handleEvents(RenderWindow& window) {
    Event event;
    while (window.pollEvent(event))
        if (event.type == Event::Closed)
            window.close();
}

void spawnObstacle(vector<Obstacle>& obstacles, float& spawnTimer, float deltaTime) {
    spawnTimer += deltaTime;
    if (spawnTimer >= SPAWN_INTERVAL) {
        float randomX = (float)(rand() % (WINDOW_WIDTH - (int)OBSTACLE_WIDTH));
        obstacles.push_back(Obstacle(randomX, 0));
        spawnTimer = 0;
    }
}

void render(RenderWindow& window, Player& player,
            vector<Obstacle>& obstacles, Text& gameOverText, bool gameOver) {
    window.clear();
    player.draw(window);
    for (auto& obstacle : obstacles) obstacle.draw(window);
    if (gameOver) window.draw(gameOverText);
    window.display();
}

int main() {
    srand((unsigned)time(0));   // FIX 1: عشوائية حقيقية

    RenderWindow window;
    initWindow(window);

    Font font;
    if (!loadFont(font)) return -1;

    Music gameOverMusic;
    if (!loadSound(gameOverMusic)) return -1;

    Text gameOverText;
    initGameOverText(gameOverText, font);

    Player           player;
    vector<Obstacle> obstacles;
    Clock            clock;
    float            spawnTimer = 0.f;
    bool             gameOver   = false;

    while (window.isOpen()) {
        float deltaTime = clock.restart().asSeconds();

        handleEvents(window);

        if (!gameOver) {
            player.handleInput(deltaTime);
            spawnObstacle(obstacles, spawnTimer, deltaTime);

            // FIX 3: حذف دالة updateObstacles القديمة اللي كانت بتستخدم 1/60 ثابت بدل deltaTime الحقيقي
            for (size_t i = 0; i < obstacles.size(); i++) {
                obstacles[i].update(deltaTime);
                if (obstacles[i].checkCollision(player.shape)) {
                    gameOverMusic.play();
                    gameOver = true;
                }
                if (obstacles[i].isOffScreen()) {
                    obstacles.erase(obstacles.begin() + i);
                    i--;
                }
            }
        }

        render(window, player, obstacles, gameOverText, gameOver);
    }
    return 0;
}
