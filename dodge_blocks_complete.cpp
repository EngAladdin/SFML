#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>
#include <sstream>
#include <cmath>
using namespace sf;
using namespace std;

// ─── Constants ───────────────────────────────────────────────────────────────
const int    WINDOW_WIDTH      = 800;
const int    WINDOW_HEIGHT     = 600;
const string WINDOW_TITLE      = "Dodge The Falling Blocks";
const string FONT_FILE         = "assets/arial.ttf";
const string GAMEOVER_SOUND    = "assets/Sounds/gameover.wav";

const float  PLAYER_SPEED      = 300.f;
const float  PLAYER_WIDTH      = 50.f;
const float  PLAYER_HEIGHT     = 50.f;

const float  OBSTACLE_BASE_SPEED  = 200.f;
const float  OBSTACLE_SPEED_STEP  = 5.f;     // زيادة السرعة كل عقبة تعدي
const float  OBSTACLE_MAX_SPEED   = 600.f;
const float  OBSTACLE_WIDTH    = 50.f;
const float  OBSTACLE_HEIGHT   = 50.f;

const float  SPAWN_BASE        = 0.8f;       // ثواني بين كل عقبة
const float  SPAWN_MIN         = 0.25f;      // أسرع فترة ممكنة
const float  SPAWN_SHRINK      = 0.005f;     // تقليل الـ interval مع الوقت

const int    SCORE_PER_DODGE   = 5;          // نقاط لما عقبة تعدي
const float  DELTA_CAP         = 0.05f;

// ─── Game State ──────────────────────────────────────────────────────────────
enum class GameState { Menu, Playing, Paused, GameOver };

// ─── Colors ──────────────────────────────────────────────────────────────────
Color randomObstacleColor() {
    Color palette[] = {
        Color::Green, Color::Cyan, Color::Yellow,
        Color(255,128,0), Color(200,50,255), Color(50,200,255)
    };
    return palette[rand() % 6];
}

// ─── Player ──────────────────────────────────────────────────────────────────
class Player {
public:
    RectangleShape shape;

    Player() { reset(); }

    void reset() {
        shape.setSize(Vector2f(PLAYER_WIDTH, PLAYER_HEIGHT));
        shape.setFillColor(Color::Red);
        shape.setPosition(WINDOW_WIDTH / 2.f - PLAYER_WIDTH / 2.f,
                          WINDOW_HEIGHT - PLAYER_HEIGHT - 10.f);
    }

    void handleInput(float dt) {
        if (Keyboard::isKeyPressed(Keyboard::Left)  ||
            Keyboard::isKeyPressed(Keyboard::A))
            shape.move(-PLAYER_SPEED * dt, 0);
        if (Keyboard::isKeyPressed(Keyboard::Right) ||
            Keyboard::isKeyPressed(Keyboard::D))
            shape.move( PLAYER_SPEED * dt, 0);

        // حدود الشاشة
        if (shape.getPosition().x < 0)
            shape.setPosition(0, shape.getPosition().y);
        if (shape.getPosition().x + PLAYER_WIDTH > WINDOW_WIDTH)
            shape.setPosition(WINDOW_WIDTH - PLAYER_WIDTH, shape.getPosition().y);
    }

    void draw(RenderWindow& w) { w.draw(shape); }
};

// ─── Obstacle ────────────────────────────────────────────────────────────────
class Obstacle {
public:
    RectangleShape shape;
    float speed;

    Obstacle(float x, float spd) : speed(spd) {
        shape.setSize(Vector2f(OBSTACLE_WIDTH, OBSTACLE_HEIGHT));
        shape.setFillColor(randomObstacleColor());
        shape.setPosition(x, -OBSTACLE_HEIGHT);
    }

    void update(float dt) { shape.move(0, speed * dt); }

    bool isOffScreen() const {
        return shape.getPosition().y > WINDOW_HEIGHT;
    }

    bool collides(const RectangleShape& p) const {
        return shape.getGlobalBounds().intersects(p.getGlobalBounds());
    }

    void draw(RenderWindow& w) { w.draw(shape); }
};

// ─── Helpers ─────────────────────────────────────────────────────────────────
void initWindow(RenderWindow& window) {
    window.create(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), WINDOW_TITLE);
    window.setFramerateLimit(60);
}

Text makeText(const Font& font, const string& str,
              unsigned size, Color color, float x, float y) {
    Text t(str, font, size);
    t.setFillColor(color);
    t.setPosition(x, y);
    return t;
}

// ─── Render ──────────────────────────────────────────────────────────────────
void render(RenderWindow& window, Player& player,
            vector<Obstacle>& obstacles,
            const Font& font, bool fontOk,
            int score, int highScore,
            GameState state) {

    window.clear(Color(15, 15, 25));

    // ── عناصر اللعبة ────────────────────────────────────────────
    if (state == GameState::Playing || state == GameState::Paused) {
        player.draw(window);
        for (auto& ob : obstacles) ob.draw(window);
    }

    // ── HUD ─────────────────────────────────────────────────────
    if (fontOk) {
        {
            ostringstream ss; ss << "Score: " << score;
            window.draw(makeText(font, ss.str(), 24, Color::White, 10.f, 10.f));
        }
        {
            ostringstream hs; hs << "Best: " << highScore;
            window.draw(makeText(font, hs.str(), 24, Color::Yellow, 10.f, 40.f));
        }
        window.draw(makeText(font, "[Arrows/AD] Move  [P] Pause  [R] Restart  [Esc] Quit",
                             15, Color(120,120,120), 8.f, (float)WINDOW_HEIGHT - 26.f));
    }

    // ── Menu ─────────────────────────────────────────────────────
    if (state == GameState::Menu && fontOk) {
        window.draw(makeText(font, "DODGE THE BLOCKS", 52, Color::Cyan,
                             WINDOW_WIDTH/2.f - 240.f, WINDOW_HEIGHT/2.f - 110.f));
        window.draw(makeText(font, "Press SPACE to Start", 28, Color::White,
                             WINDOW_WIDTH/2.f - 150.f, WINDOW_HEIGHT/2.f + 10.f));
    }

    // ── Paused ───────────────────────────────────────────────────
    if (state == GameState::Paused && fontOk) {
        RectangleShape overlay(Vector2f((float)WINDOW_WIDTH, (float)WINDOW_HEIGHT));
        overlay.setFillColor(Color(0, 0, 0, 160));
        window.draw(overlay);

        window.draw(makeText(font, "PAUSED", 64, Color::Yellow,
                             WINDOW_WIDTH/2.f - 105.f, WINDOW_HEIGHT/2.f - 60.f));
        window.draw(makeText(font, "SPACE to Resume  |  R to Restart", 22, Color::White,
                             WINDOW_WIDTH/2.f - 180.f, WINDOW_HEIGHT/2.f + 30.f));
    }

    // ── Game Over ────────────────────────────────────────────────
    if (state == GameState::GameOver && fontOk) {
        RectangleShape overlay(Vector2f((float)WINDOW_WIDTH, (float)WINDOW_HEIGHT));
        overlay.setFillColor(Color(0, 0, 0, 180));
        window.draw(overlay);

        window.draw(makeText(font, "GAME OVER", 64, Color::Red,
                             WINDOW_WIDTH/2.f - 180.f, WINDOW_HEIGHT/2.f - 80.f));

        ostringstream ss; ss << "Score: " << score;
        window.draw(makeText(font, ss.str(), 32, Color::White,
                             WINDOW_WIDTH/2.f - 80.f, WINDOW_HEIGHT/2.f + 0.f));

        if (score >= highScore && score > 0) {
            window.draw(makeText(font, "NEW BEST!", 26, Color::Yellow,
                                 WINDOW_WIDTH/2.f - 75.f, WINDOW_HEIGHT/2.f + 42.f));
        }

        window.draw(makeText(font, "Press R to Restart", 24, Color(200,200,200),
                             WINDOW_WIDTH/2.f - 120.f, WINDOW_HEIGHT/2.f + 80.f));
    }

    window.display();
}

// ─── Main ────────────────────────────────────────────────────────────────────
int main() {
    srand((unsigned)time(nullptr));

    RenderWindow window;
    initWindow(window);

    // Font (fallback مسارات متعددة)
    Font font;
    bool fontOk = font.loadFromFile(FONT_FILE)               ||
                  font.loadFromFile("C:/Windows/Fonts/arial.ttf") ||
                  font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");

    // Sound (اختياري)
    Music gameOverMusic;
    bool  soundOk = gameOverMusic.openFromFile(GAMEOVER_SOUND);

    // ── Game state variables ─────────────────────────────────────
    Player           player;
    vector<Obstacle> obstacles;
    Clock            clock;
    float            spawnTimer    = 0.f;
    float            spawnInterval = SPAWN_BASE;
    float            obstacleSpeed = OBSTACLE_BASE_SPEED;
    int              score         = 0;
    int              highScore     = 0;
    GameState        state         = GameState::Menu;
    bool             restartFlag   = false;

    auto resetGame = [&]() {
        if (score > highScore) highScore = score;
        player.reset();
        obstacles.clear();
        spawnTimer    = 0.f;
        spawnInterval = SPAWN_BASE;
        obstacleSpeed = OBSTACLE_BASE_SPEED;
        score         = 0;
        state         = GameState::Playing;
        clock.restart();
    };

    // ── Game Loop ────────────────────────────────────────────────
    while (window.isOpen()) {

        if (restartFlag) { resetGame(); restartFlag = false; }

        float dt = clock.restart().asSeconds();
        if (dt > DELTA_CAP) dt = DELTA_CAP;

        // Events
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();

            if (event.type == Event::KeyPressed) {
                switch (event.key.code) {

                    case Keyboard::Escape:
                        window.close();
                        break;

                    case Keyboard::Space:
                        if      (state == GameState::Menu)    resetGame();
                        else if (state == GameState::Playing) state = GameState::Paused;
                        else if (state == GameState::Paused)  state = GameState::Playing;
                        break;

                    case Keyboard::P:
                        if      (state == GameState::Playing) state = GameState::Paused;
                        else if (state == GameState::Paused)  state = GameState::Playing;
                        break;

                    case Keyboard::R:
                        if (state == GameState::GameOver || state == GameState::Paused)
                            restartFlag = true;
                        break;

                    default: break;
                }
            }
        }

        // Update
        if (state == GameState::Playing) {
            player.handleInput(dt);

            // Spawn عقبات
            spawnTimer += dt;
            // تقليل interval مع الوقت
            spawnInterval -= SPAWN_SHRINK * dt;
            if (spawnInterval < SPAWN_MIN) spawnInterval = SPAWN_MIN;

            if (spawnTimer >= spawnInterval) {
                float rx = (float)(rand() % (int)(WINDOW_WIDTH - OBSTACLE_WIDTH));
                obstacles.push_back(Obstacle(rx, obstacleSpeed));
                spawnTimer = 0.f;
            }

            // Update عقبات
            for (int i = (int)obstacles.size() - 1; i >= 0; --i) {
                obstacles[i].update(dt);

                // تصادم
                if (obstacles[i].collides(player.shape)) {
                    if (soundOk) gameOverMusic.play();
                    if (score > highScore) highScore = score;
                    state = GameState::GameOver;
                    break;
                }

                // عقبة عدت → نقطة + زيادة سرعة
                if (obstacles[i].isOffScreen()) {
                    score += SCORE_PER_DODGE;
                    obstacleSpeed += OBSTACLE_SPEED_STEP;
                    if (obstacleSpeed > OBSTACLE_MAX_SPEED)
                        obstacleSpeed = OBSTACLE_MAX_SPEED;
                    obstacles.erase(obstacles.begin() + i);
                }
            }
        }

        render(window, player, obstacles, font, fontOk, score, highScore, state);
    }

    return 0;
}
