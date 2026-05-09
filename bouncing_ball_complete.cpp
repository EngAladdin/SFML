#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include <sstream>
#include <cstdlib>   // rand, srand
#include <ctime>     // time
using namespace sf;
using namespace std;

// ─── Constants ───────────────────────────────────────────────────────────────
const int    WINDOW_WIDTH   = 800;
const int    WINDOW_HEIGHT  = 600;
const string WINDOW_TITLE   = "Bouncing Ball";
const string SOUND_FILE     = "assets/Sounds/flap.wav";
const string FONT_FILE      = "assets/Fonts/arial.ttf";

const float  BALL_RADIUS    = 25.f;
const float  BALL_START_X   = 375.f;
const float  BALL_START_Y   = 275.f;
const float  INITIAL_SPEED  = 250.f;
const float  SPEED_INCREMENT= 15.f;   // زيادة السرعة كل ارتطام
const float  MAX_SPEED      = 700.f;
const int    SCORE_PER_BOUNCE = 10;

// ─── Game State ───────────────────────────────────────────────────────────────
enum class GameState { Menu, Playing, Paused, GameOver };

// ─── Helper: random color ─────────────────────────────────────────────────────
Color randomBrightColor() {
    // نختار لون من قائمة ألوان زاهية
    Color colors[] = {
        Color::Green, Color::Cyan, Color::Yellow, Color::Magenta,
        Color(255,128,0), Color(128,255,0), Color(0,255,128),
        Color(255,64,64), Color(64,64,255)
    };
    int n = sizeof(colors) / sizeof(colors[0]);
    return colors[rand() % n];
}

// ─── Window ──────────────────────────────────────────────────────────────────
void initWindow(RenderWindow& window) {
    window.create(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), WINDOW_TITLE);
    window.setFramerateLimit(60);
}

// ─── Ball ────────────────────────────────────────────────────────────────────
CircleShape createBall() {
    CircleShape ball(BALL_RADIUS);
    ball.setFillColor(Color::Green);
    ball.setPosition(BALL_START_X, BALL_START_Y);
    return ball;
}

// ─── Sound ───────────────────────────────────────────────────────────────────
bool loadSound(Music& music) {
    return music.openFromFile(SOUND_FILE);
}

// ─── Events ──────────────────────────────────────────────────────────────────
void handleEvents(RenderWindow& window, GameState& state, bool& restart) {
    Event event;
    while (window.pollEvent(event)) {
        if (event.type == Event::Closed)
            window.close();

        if (event.type == Event::KeyPressed) {
            switch (event.key.code) {

                // Escape: إغلاق النافذة في أي وقت
                case Keyboard::Escape:
                    window.close();
                    break;

                // P أو Space: إيقاف / استئناف
                case Keyboard::P:
                case Keyboard::Space:
                    if (state == GameState::Playing)
                        state = GameState::Paused;
                    else if (state == GameState::Paused)
                        state = GameState::Playing;
                    else if (state == GameState::Menu)
                        state = GameState::Playing;
                    break;

                // R: إعادة التشغيل
                case Keyboard::R:
                    if (state == GameState::GameOver || state == GameState::Paused) {
                        restart = true;
                    }
                    break;

                default:
                    break;
            }
        }
    }
}

// ─── Update Ball ─────────────────────────────────────────────────────────────
void updateBall(CircleShape& ball, Vector2f& velocity,
                Music& music, float deltaTime,
                int& score, bool soundEnabled) {

    ball.move(velocity * deltaTime);

    float diameter = BALL_RADIUS * 2.f;
    bool  bounced  = false;

    // حائط يسار
    if (ball.getPosition().x <= 0.f) {
        ball.setPosition(0.f, ball.getPosition().y);
        velocity.x = abs(velocity.x);
        bounced = true;
    }
    // حائط يمين
    if (ball.getPosition().x + diameter >= (float)WINDOW_WIDTH) {
        ball.setPosition((float)WINDOW_WIDTH - diameter, ball.getPosition().y);
        velocity.x = -abs(velocity.x);
        bounced = true;
    }
    // حائط فوق
    if (ball.getPosition().y <= 0.f) {
        ball.setPosition(ball.getPosition().x, 0.f);
        velocity.y = abs(velocity.y);
        bounced = true;
    }
    // حائط تحت
    if (ball.getPosition().y + diameter >= (float)WINDOW_HEIGHT) {
        ball.setPosition(ball.getPosition().x, (float)WINDOW_HEIGHT - diameter);
        velocity.y = -abs(velocity.y);
        bounced = true;
    }

    if (bounced) {
        // نقاط
        score += SCORE_PER_BOUNCE;

        // تسريع تدريجي
        float currentSpeed = sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
        if (currentSpeed < MAX_SPEED) {
            float factor = (currentSpeed + SPEED_INCREMENT) / currentSpeed;
            velocity *= factor;
        }

        // لون عشوائي
        ball.setFillColor(randomBrightColor());

        // صوت
        if (soundEnabled && music.getStatus() != Music::Playing)
            music.play();
    }
}

// ─── UI Helpers ──────────────────────────────────────────────────────────────
Text makeText(const Font& font, const string& str,
              unsigned size, Color color, float x, float y) {
    Text t(str, font, size);
    t.setFillColor(color);
    t.setPosition(x, y);
    return t;
}

// ─── Render ──────────────────────────────────────────────────────────────────
void render(RenderWindow& window, CircleShape& ball,
            const Font& font, int score, int highScore,
            GameState state) {

    window.clear(Color(20, 20, 30));   // خلفية داكنة مريحة للعين

    // ── HUD ─────────────────────────────────────────────────────
    {
        ostringstream ss;
        ss << "Score: " << score;
        Text scoreTxt = makeText(font, ss.str(), 24, Color::White, 10.f, 10.f);
        window.draw(scoreTxt);

        ostringstream hs;
        hs << "Best: " << highScore;
        Text hsTxt = makeText(font, hs.str(), 24, Color::Yellow, 10.f, 40.f);
        window.draw(hsTxt);

        Text hint = makeText(font, "[P] Pause  [R] Restart  [Esc] Quit",
                             16, Color(150,150,150), 10.f, (float)WINDOW_HEIGHT - 28.f);
        window.draw(hint);
    }

    // ── الكرة ───────────────────────────────────────────────────
    if (state == GameState::Playing || state == GameState::Paused)
        window.draw(ball);

    // ── شاشة Menu ───────────────────────────────────────────────
    if (state == GameState::Menu) {
        Text title = makeText(font, "BOUNCING BALL", 60, Color::Cyan,
                              WINDOW_WIDTH/2.f - 210.f, WINDOW_HEIGHT/2.f - 100.f);
        Text sub   = makeText(font, "Press SPACE to Start", 28, Color::White,
                              WINDOW_WIDTH/2.f - 140.f, WINDOW_HEIGHT/2.f + 10.f);
        window.draw(title);
        window.draw(sub);
    }

    // ── شاشة Paused ─────────────────────────────────────────────
    if (state == GameState::Paused) {
        // شبه شفافية (rectangle داكن)
        RectangleShape overlay(Vector2f((float)WINDOW_WIDTH, (float)WINDOW_HEIGHT));
        overlay.setFillColor(Color(0, 0, 0, 150));
        window.draw(overlay);

        Text pauseTxt = makeText(font, "PAUSED", 64, Color::Yellow,
                                 WINDOW_WIDTH/2.f - 110.f, WINDOW_HEIGHT/2.f - 60.f);
        Text resumeTxt = makeText(font, "Press SPACE to Resume  |  R to Restart", 22,
                                  Color::White,
                                  WINDOW_WIDTH/2.f - 200.f, WINDOW_HEIGHT/2.f + 30.f);
        window.draw(pauseTxt);
        window.draw(resumeTxt);
    }

    // ── شاشة Game Over (مش مستخدمة حالياً لكن جاهزة) ──────────
    if (state == GameState::GameOver) {
        RectangleShape overlay(Vector2f((float)WINDOW_WIDTH, (float)WINDOW_HEIGHT));
        overlay.setFillColor(Color(0, 0, 0, 180));
        window.draw(overlay);

        Text goTxt  = makeText(font, "GAME OVER", 64, Color::Red,
                               WINDOW_WIDTH/2.f - 165.f, WINDOW_HEIGHT/2.f - 70.f);
        ostringstream ss; ss << "Final Score: " << score;
        Text scoreTxt = makeText(font, ss.str(), 32, Color::White,
                                 WINDOW_WIDTH/2.f - 110.f, WINDOW_HEIGHT/2.f + 10.f);
        Text restart  = makeText(font, "Press R to Restart", 24, Color(200,200,200),
                                 WINDOW_WIDTH/2.f - 115.f, WINDOW_HEIGHT/2.f + 60.f);
        window.draw(goTxt);
        window.draw(scoreTxt);
        window.draw(restart);
    }

    window.display();
}

// ─── Main ────────────────────────────────────────────────────────────────────
int main() {
    srand((unsigned)time(nullptr));

    RenderWindow window;
    initWindow(window);

    // Font
    Font font;
    bool fontLoaded = font.loadFromFile(FONT_FILE);
    if (!fontLoaded) {
        // fallback: حاول تحميل أي خط موجود على النظام
        fontLoaded = font.loadFromFile("C:/Windows/Fonts/arial.ttf") ||
                     font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
        if (!fontLoaded) {
            // اللعبة هتشتغل بدون نصوص لو مفيش فونت
        }
    }

    // Sound (اختيارية – لو الملف مش موجود اللعبة تكمل)
    Music music;
    bool soundEnabled = loadSound(music);

    // Game variables
    CircleShape ball     = createBall();
    Vector2f    velocity = Vector2f(INITIAL_SPEED, INITIAL_SPEED);
    Clock       clock;
    int         score     = 0;
    int         highScore = 0;
    GameState   state     = GameState::Menu;
    bool        restart   = false;

    // ── Game Loop ────────────────────────────────────────────────
    while (window.isOpen()) {

        // إعادة تهيئة كاملة لو طلب المستخدم Restart
        if (restart) {
            ball     = createBall();
            velocity = Vector2f(INITIAL_SPEED, INITIAL_SPEED);
            if (score > highScore) highScore = score;
            score    = 0;
            state    = GameState::Playing;
            restart  = false;
            clock.restart();
        }

        float deltaTime = clock.restart().asSeconds();
        // تحديد deltaTime بسقف أمان لتجنب القفز عند تأخر الإطار
        if (deltaTime > 0.05f) deltaTime = 0.05f;

        handleEvents(window, state, restart);

        if (state == GameState::Playing)
            updateBall(ball, velocity, music, deltaTime, score, soundEnabled);

        render(window, ball, font, score, highScore, state);
    }

    if (score > highScore) highScore = score;
    return 0;
}
