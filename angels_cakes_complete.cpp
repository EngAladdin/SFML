#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>
#include <ctime>
using namespace std;
using namespace sf;

// ─── Constants ───────────────────────────────────────────────────────────────
const int    WINDOW_WIDTH      = 800;
const int    WINDOW_HEIGHT     = 600;
const string WINDOW_TITLE      = "Angel's Cakes Game";
const string FONT_FILE         = "arial.ttf";
const string SOUND_CATCH       = "assets/sounds/die.wav";
const string SOUND_MISS        = "assets/sounds/miss.wav";   // صوت لما تفوتك كيكة
const string PLAYER_FILE       = "assets/images/bird.png";
const string CAKE_FILE         = "assets/images/cloud.png";

const float  PLAYER_SPEED      = 300.f;
const float  CAKE_BASE_SPEED   = 200.f;
const float  CAKE_SPEED_STEP   = 8.f;     // زيادة كل كيكة تتمسك
const float  CAKE_MAX_SPEED    = 600.f;
const float  CAKE_SPAWN_CHANCE = 3.f;     // من 100 كل فريم
const float  DELTA_CAP         = 0.05f;
const int    MAX_LIVES         = 3;
const int    TEXT_SIZE         = 28;

// ─── Game State ──────────────────────────────────────────────────────────────
enum class GameState { Menu, Playing, Paused, GameOver };

// ─── Texture Cache (يحل bug الـ local Texture) ───────────────────────────────
// الـ Texture لازم تعيش طول عمر الـ Sprite
struct TextureCache {
    Texture player, cake;
    bool playerOk = false, cakeOk = false;

    bool load() {
        // fallback مسارات متعددة للـ player
        playerOk = player.loadFromFile(PLAYER_FILE) ||
                   player.loadFromFile("assets/bird.png");
        // fallback للـ cake
        cakeOk   = cake.loadFromFile(CAKE_FILE) ||
                   cake.loadFromFile("assets/cloud.png");
        return true;   // نكمل حتى لو الصور مش موجودة
    }
};

// ─── Helper: نص ──────────────────────────────────────────────────────────────
Text makeText(const Font& f, const string& s, unsigned sz,
              Color fill, float x, float y,
              Color outline = Color::Transparent, float thick = 0) {
    Text t(s, f, sz);
    t.setFillColor(fill);
    if (thick > 0) { t.setOutlineColor(outline); t.setOutlineThickness(thick); }
    t.setPosition(x, y);
    return t;
}

// ─── Player ──────────────────────────────────────────────────────────────────
class Player {
public:
    Sprite sprite;
    bool   hasTexture;

    Player(const TextureCache& tc) {
        hasTexture = tc.playerOk;
        if (hasTexture) {
            sprite.setTexture(tc.player);
            sprite.setScale(0.15f, 0.15f);   // اضبط حسب حجم صورتك
        } else {
            // fallback: مربع أحمر
        }
        reset();
    }

    void reset() {
        sprite.setPosition(WINDOW_WIDTH / 2.f - 25.f, WINDOW_HEIGHT - 100.f);
    }

    void move(float dx, float dt) {
        sprite.move(dx * dt, 0);
        float px = sprite.getPosition().x;
        float pw = hasTexture ? sprite.getGlobalBounds().width : 50.f;
        if (px < 0)               sprite.setPosition(0, sprite.getPosition().y);
        if (px > WINDOW_WIDTH-pw) sprite.setPosition(WINDOW_WIDTH-pw, sprite.getPosition().y);
    }

    FloatRect bounds() const { return sprite.getGlobalBounds(); }

    void draw(RenderWindow& w, const Font& font, bool fontOk) {
        if (hasTexture) { w.draw(sprite); return; }
        // fallback شكل مرئي
        RectangleShape r(Vector2f(50, 50));
        r.setFillColor(Color(220, 80, 80));
        r.setPosition(sprite.getPosition());
        w.draw(r);
        if (fontOk) w.draw(makeText(font, "P", 30, Color::White,
                                    sprite.getPosition().x+10,
                                    sprite.getPosition().y+8));
    }
};

// ─── Cake ─────────────────────────────────────────────────────────────────────
struct Cake {
    Sprite sprite;
    float  speed;

    Cake(const TextureCache& tc, float spd) : speed(spd) {
        if (tc.cakeOk) {
            sprite.setTexture(tc.cake);
            sprite.setScale(0.1f, 0.1f);
        }
        float rx = (float)(rand() % (WINDOW_WIDTH - 60));
        sprite.setPosition(rx, -60.f);
    }

    void update(float dt) { sprite.move(0, speed * dt); }
    bool offScreen() const { return sprite.getPosition().y > WINDOW_HEIGHT; }
    bool collides(FloatRect pr) const {
        return sprite.getGlobalBounds().intersects(pr);
    }
};

void drawCake(RenderWindow& w, Cake& ck, bool hasTexture,
              const Font& font, bool fontOk) {
    if (hasTexture) { w.draw(ck.sprite); return; }
    RectangleShape r(Vector2f(50, 50));
    r.setFillColor(Color(255, 200, 50));
    r.setPosition(ck.sprite.getPosition());
    w.draw(r);
    if (fontOk) w.draw(makeText(font, "C", 28, Color::Black,
                                ck.sprite.getPosition().x+12,
                                ck.sprite.getPosition().y+8));
}

// ─── Render Lives ─────────────────────────────────────────────────────────────
void drawLives(RenderWindow& w, const Font& f, bool fontOk, int lives) {
    if (!fontOk) return;
    string hearts = "";
    for (int i = 0; i < MAX_LIVES; i++)
        hearts += (i < lives) ? " <3 " : " -- ";
    Text t(hearts, f, 22);
    t.setFillColor(Color(220, 60, 60));
    t.setPosition(10.f, 48.f);
    w.draw(t);
}

// ─── Main ─────────────────────────────────────────────────────────────────────
int main() {
    srand((unsigned)time(nullptr));

    RenderWindow window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), WINDOW_TITLE);
    window.setFramerateLimit(60);

    // ── Assets ───────────────────────────────────────────────────
    TextureCache tc; tc.load();

    Font font;
    bool fontOk = font.loadFromFile(FONT_FILE)                    ||
                  font.loadFromFile("C:/Windows/Fonts/arial.ttf") ||
                  font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");

    Music catchSound, missSound;
    bool  catchOk = catchSound.openFromFile(SOUND_CATCH);
    bool  missOk  = missSound.openFromFile(SOUND_MISS);

    // ── Game variables ────────────────────────────────────────────
    Player           player(tc);
    vector<Cake>     cakes;
    Clock            clock;
    int              score      = 0;
    int              highScore  = 0;
    int              lives      = MAX_LIVES;
    float            cakeSpeed  = CAKE_BASE_SPEED;
    GameState        state      = GameState::Menu;
    bool             restartReq = false;

    auto resetGame = [&]() {
        if (score > highScore) highScore = score;
        player.reset();
        cakes.clear();
        score     = 0;
        lives     = MAX_LIVES;
        cakeSpeed = CAKE_BASE_SPEED;
        state     = GameState::Playing;
        clock.restart();
    };

    // ── Game Loop ─────────────────────────────────────────────────
    while (window.isOpen()) {

        if (restartReq) { resetGame(); restartReq = false; }

        float dt = clock.restart().asSeconds();
        if (dt > DELTA_CAP) dt = DELTA_CAP;

        // Events
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();

            if (event.type == Event::KeyPressed) {
                switch (event.key.code) {
                    case Keyboard::Escape: window.close(); break;

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
                            restartReq = true;
                        break;

                    default: break;
                }
            }
        }

        // ── Update ───────────────────────────────────────────────
        if (state == GameState::Playing) {

            if (Keyboard::isKeyPressed(Keyboard::Left)  ||
                Keyboard::isKeyPressed(Keyboard::A))
                player.move(-PLAYER_SPEED, dt);
            if (Keyboard::isKeyPressed(Keyboard::Right) ||
                Keyboard::isKeyPressed(Keyboard::D))
                player.move( PLAYER_SPEED, dt);

            // Spawn
            if (rand() % 100 < (int)CAKE_SPAWN_CHANCE)
                cakes.emplace_back(tc, cakeSpeed);

            // Update كل الكيك
            for (auto it = cakes.begin(); it != cakes.end(); ) {
                it->update(dt);

                if (it->collides(player.bounds())) {
                    // اتمسكت الكيكة
                    score++;
                    cakeSpeed += CAKE_SPEED_STEP;
                    if (cakeSpeed > CAKE_MAX_SPEED) cakeSpeed = CAKE_MAX_SPEED;
                    if (catchOk && catchSound.getStatus() != Music::Playing)
                        catchSound.play();
                    it = cakes.erase(it);

                } else if (it->offScreen()) {
                    // فاتت من غير ما تتمسك
                    lives--;
                    if (missOk && missSound.getStatus() != Music::Playing)
                        missSound.play();
                    it = cakes.erase(it);

                    if (lives <= 0) {
                        if (score > highScore) highScore = score;
                        state = GameState::GameOver;
                        break;
                    }
                } else {
                    ++it;
                }
            }
        }

        // ── Render ───────────────────────────────────────────────
        window.clear(Color(30, 30, 50));

        if (state == GameState::Playing || state == GameState::Paused) {
            player.draw(window, font, fontOk);
            for (auto& ck : cakes)
                drawCake(window, ck, tc.cakeOk, font, fontOk);
        }

        // HUD
        if (fontOk) {
            // Score
            window.draw(makeText(font, "Score: " + to_string(score),
                                 TEXT_SIZE, Color::White, 10.f, 10.f,
                                 Color::Black, 2.f));
            // High Score
            window.draw(makeText(font, "Best: " + to_string(highScore),
                                 20, Color::Yellow,
                                 (float)WINDOW_WIDTH - 130.f, 10.f));
            // Lives
            drawLives(window, font, fontOk, lives);
            // Speed
            ostringstream sp; sp << "Speed: " << (int)cakeSpeed;
            window.draw(makeText(font, sp.str(), 18, Color(180,180,180),
                                 10.f, (float)WINDOW_HEIGHT - 28.f));
            // Hint
            window.draw(makeText(font, "[Arrows/AD] Move  [P] Pause  [R] Restart  [Esc] Quit",
                                 14, Color(100,100,100),
                                 (float)WINDOW_WIDTH/2.f - 190.f,
                                 (float)WINDOW_HEIGHT - 24.f));
        }

        // ── Overlays ─────────────────────────────────────────────
        auto darkOverlay = [&](Uint8 alpha) {
            RectangleShape ov(Vector2f((float)WINDOW_WIDTH, (float)WINDOW_HEIGHT));
            ov.setFillColor(Color(0,0,0,alpha));
            window.draw(ov);
        };

        if (state == GameState::Menu && fontOk) {
            window.draw(makeText(font, "ANGEL'S CAKES", 54, Color::Cyan,
                                 WINDOW_WIDTH/2.f-220.f, WINDOW_HEIGHT/2.f-110.f));
            window.draw(makeText(font, "Catch the falling cakes!", 24, Color::White,
                                 WINDOW_WIDTH/2.f-170.f, WINDOW_HEIGHT/2.f-30.f));
            window.draw(makeText(font, "Press SPACE to Start", 26, Color(220,220,100),
                                 WINDOW_WIDTH/2.f-160.f, WINDOW_HEIGHT/2.f+30.f));
        }

        if (state == GameState::Paused && fontOk) {
            darkOverlay(150);
            window.draw(makeText(font, "PAUSED", 64, Color::Yellow,
                                 WINDOW_WIDTH/2.f-105.f, WINDOW_HEIGHT/2.f-60.f));
            window.draw(makeText(font, "SPACE to Resume  |  R to Restart", 22,
                                 Color::White, WINDOW_WIDTH/2.f-185.f,
                                 WINDOW_HEIGHT/2.f+30.f));
        }

        if (state == GameState::GameOver && fontOk) {
            darkOverlay(170);
            window.draw(makeText(font, "GAME OVER", 64, Color::Red,
                                 WINDOW_WIDTH/2.f-175.f, WINDOW_HEIGHT/2.f-90.f));
            window.draw(makeText(font, "Score: " + to_string(score), 32,
                                 Color::White, WINDOW_WIDTH/2.f-90.f,
                                 WINDOW_HEIGHT/2.f-10.f));
            if (score >= highScore && score > 0)
                window.draw(makeText(font, "NEW BEST!", 26, Color::Yellow,
                                     WINDOW_WIDTH/2.f-80.f, WINDOW_HEIGHT/2.f+30.f));
            window.draw(makeText(font, "Press R to Restart", 24, Color(200,200,200),
                                 WINDOW_WIDTH/2.f-125.f, WINDOW_HEIGHT/2.f+70.f));
        }

        window.display();
    }

    return 0;
}
