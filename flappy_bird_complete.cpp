#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <cmath>
using namespace std;
using namespace sf;

// ─── Constants ───────────────────────────────────────────────────────────────
const int    W = 800, H = 600;
const string TITLE    = "Flappy Bird";
const string FONT_F   = "arial.ttf";
const string SND_SCORE= "assets/sounds/score.wav";
const string SND_DIE  = "assets/sounds/die.wav";
const string BG_IMG   = "assets/images/background.png";

// Physics (frame-based — game locked at 60 FPS)
const float  GRAV     = 0.38f;
const float  FLAP     = -7.f;
const float  MAX_FALL = 9.f;

// Pipes
const float  PW       = 70.f;
const float  PGAP     = 175.f;
const float  PSPEED_BASE = 3.f;
const float  PSPEED_MAX  = 7.f;
const float  PSPEED_STEP = 0.15f;   // زيادة بعد كل 5 نقاط
const int    SPAWN_MS = 1700;

// Ground
const float  GROUND_H = 40.f;

// ─── State ───────────────────────────────────────────────────────────────────
enum class State { Menu, Playing, Paused, Over };

// ─── Helpers ─────────────────────────────────────────────────────────────────
Text makeTxt(const Font& f, const string& s, unsigned sz,
             Color fill, float x, float y,
             Color out = Color::Black, float thick = 2.f) {
    Text t(s, f, sz);
    t.setFillColor(fill);
    t.setOutlineColor(out);
    t.setOutlineThickness(thick);
    // توسيط أفقي لو x == -1
    if (x < 0) x = W / 2.f - t.getLocalBounds().width / 2.f;
    t.setPosition(x, y);
    return t;
}

// ─── Bird ────────────────────────────────────────────────────────────────────
class Bird {
    CircleShape  body, eye;
    ConvexShape  beak, wingT, wingB;

    void moveAll(float dx, float dy) {
        body.move(dx,dy); eye.move(dx,dy);
        beak.move(dx,dy); wingT.move(dx,dy); wingB.move(dx,dy);
    }
    void rot(float a) {
        body.setRotation(a); eye.setRotation(a);
        beak.setRotation(a); wingT.setRotation(a); wingB.setRotation(a);
    }

public:
    float vel = 0;
    float wingAnim = 0;   // للرفرفة

    Bird() { build(); }

    void build() {
        float cx = W / 4.f, cy = H / 2.f;

        body.setRadius(16); body.setOrigin(16,16); body.setPosition(cx,cy);
        body.setFillColor({255,210,0}); body.setOutlineThickness(2); body.setOutlineColor({200,160,0});

        eye.setRadius(5); eye.setOrigin(5,5); eye.setPosition(cx+10, cy-5);
        eye.setFillColor(Color::White); eye.setOutlineThickness(1.5f); eye.setOutlineColor(Color::Black);

        beak.setPointCount(3);
        beak.setPoint(0,{0,0}); beak.setPoint(1,{14,4}); beak.setPoint(2,{0,8});
        beak.setFillColor({255,120,0}); beak.setOrigin(0,4); beak.setPosition(cx+16,cy);

        wingT.setPointCount(4);
        wingT.setPoint(0,{0,0}); wingT.setPoint(1,{-10,-8});
        wingT.setPoint(2,{-22,-4}); wingT.setPoint(3,{-12,6});
        wingT.setFillColor({255,170,0}); wingT.setPosition(cx, cy-4);

        wingB.setPointCount(4);
        wingB.setPoint(0,{0,0}); wingB.setPoint(1,{-8,6});
        wingB.setPoint(2,{-18,4}); wingB.setPoint(3,{-10,-4});
        wingB.setFillColor({200,130,0}); wingB.setPosition(cx, cy+4);
    }

    void flap() { vel = FLAP; wingAnim = 1.f; }

    void update() {
        vel += GRAV;
        if (vel > MAX_FALL) vel = MAX_FALL;

        // رفرفة الجناح
        if (wingAnim > 0) wingAnim -= 0.08f;

        float wingOff = (wingAnim > 0) ? sinf(wingAnim * 3.14f) * 6.f : 0.f;
        // نحرك الأجنحة للأعلى قليلاً
        wingT.setPosition(wingT.getPosition().x, wingT.getPosition().y - wingOff * 0.5f);

        float a = vel * 4.f;
        if (a > 45) a = 45; if (a < -25) a = -25;
        rot(a);
        moveAll(0, vel);

        // clamp (فوق فقط، الأرض تتحقق منها خارجياً)
        float r = body.getRadius(), py = body.getPosition().y;
        if (py - r < 0) { moveAll(0, r - py); vel = 0; }
    }

    void reset() {
        float cx = W/4.f, cy = H/2.f;
        float dx = cx - body.getPosition().x;
        float dy = cy - body.getPosition().y;
        moveAll(dx, dy); vel = 0; rot(0); wingAnim = 0;
    }

    float getY()      const { return body.getPosition().y; }
    float getRadius() const { return body.getRadius(); }
    FloatRect getBounds() const { return body.getGlobalBounds(); }

    void draw(RenderWindow& w) {
        w.draw(wingB); w.draw(body); w.draw(wingT); w.draw(beak); w.draw(eye);
    }
};

// ─── Pipe ────────────────────────────────────────────────────────────────────
class Pipe {
public:
    RectangleShape tp, bp, tc, bc;
    bool scored = false;

    Pipe(float x, float gy, float spd) : speed(spd) {
        Color g{80,180,60}, cg{60,150,40};
        float bh = max(10.f, H - GROUND_H - gy - PGAP - 20.f);

        tp.setSize({PW, gy-20});    tp.setFillColor(g);  tp.setPosition(x, 0);
        tc.setSize({PW+10, 20});    tc.setFillColor(cg); tc.setPosition(x-5, gy-20);
        bc.setSize({PW+10, 20});    bc.setFillColor(cg); bc.setPosition(x-5, gy+PGAP);
        bp.setSize({PW, bh});       bp.setFillColor(g);  bp.setPosition(x, gy+PGAP+20);
    }

    void update() {
        tp.move(-speed,0); bp.move(-speed,0);
        tc.move(-speed,0); bc.move(-speed,0);
    }

    bool isOffScreen() const { return tp.getPosition().x + PW < 0; }
    bool hasPassed(float bx) const {
        return !scored && tp.getPosition().x + PW < bx;
    }
    bool checkCollision(const FloatRect& b) const {
        return tp.getGlobalBounds().intersects(b) ||
               tc.getGlobalBounds().intersects(b) ||
               bp.getGlobalBounds().intersects(b) ||
               bc.getGlobalBounds().intersects(b);
    }
    void draw(RenderWindow& w) {
        w.draw(tp); w.draw(tc); w.draw(bp); w.draw(bc);
    }

private:
    float speed;
};

// ─── Ground ──────────────────────────────────────────────────────────────────
struct Ground {
    RectangleShape bar;
    // خطوط متحركة للإحساس بالحركة
    vector<RectangleShape> marks;
    float offset = 0;
    float speed  = PSPEED_BASE;

    Ground() {
        bar.setSize({(float)W, GROUND_H});
        bar.setFillColor(Color(210,180,100));
        bar.setPosition(0, H - GROUND_H);
        bar.setOutlineThickness(2);
        bar.setOutlineColor(Color(160,130,60));

        for (int i = 0; i < 20; i++) {
            RectangleShape m(Vector2f(4, GROUND_H - 4));
            m.setFillColor(Color(180,150,80));
            m.setPosition((float)(i * 42), H - GROUND_H + 2);
            marks.push_back(m);
        }
    }

    void update(float spd) {
        speed  = spd;
        offset += speed;
        if (offset > 42) offset -= 42;
        for (auto& m : marks)
            m.setPosition(m.getPosition().x - speed +
                          (m.getPosition().x < 0 ? (float)W + 42 : 0),
                          H - GROUND_H + 2);
    }

    bool hits(FloatRect b) const {
        return b.top + b.height >= H - GROUND_H;
    }

    void draw(RenderWindow& w) {
        w.draw(bar);
        for (auto& m : marks) w.draw(m);
    }
};

// ─── Background (parallax بسيط) ───────────────────────────────────────────────
struct Background {
    Texture tex;
    Sprite  s1, s2;
    bool    loaded = false;
    float   spd    = 0.5f;

    Background() {
        loaded = tex.loadFromFile(BG_IMG) ||
                 tex.loadFromFile("assets/images/bird.png");
        if (loaded) {
            tex.setRepeated(true);
            s1.setTexture(tex);
            s2.setTexture(tex);
            s1.setScale((float)W / tex.getSize().x, (float)(H-GROUND_H) / tex.getSize().y);
            s2.setScale(s1.getScale());
            s1.setPosition(0, 0);
            s2.setPosition((float)W, 0);
        }
    }

    void update() {
        if (!loaded) return;
        s1.move(-spd, 0); s2.move(-spd, 0);
        if (s1.getPosition().x + W < 0) s1.setPosition(s2.getPosition().x + W, 0);
        if (s2.getPosition().x + W < 0) s2.setPosition(s1.getPosition().x + W, 0);
    }

    void draw(RenderWindow& w) {
        if (loaded) { w.draw(s1); w.draw(s2); }
        else {
            // تدرج سماء بسيط
            RectangleShape sky(Vector2f((float)W, (float)(H-GROUND_H)));
            sky.setFillColor(Color(113,197,207));
            sky.setPosition(0,0);
            w.draw(sky);
        }
    }
};

// ─── Main ────────────────────────────────────────────────────────────────────
int main() {
    srand((unsigned)time(nullptr));

    RenderWindow window(VideoMode(W, H), TITLE);
    window.setFramerateLimit(60);

    // Font
    Font font;
    bool fontOk = font.loadFromFile(FONT_F)                         ||
                  font.loadFromFile("C:/Windows/Fonts/arial.ttf")   ||
                  font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");

    // Sound
    Music sndScore, sndDie;
    bool scoreOk = sndScore.openFromFile(SND_SCORE);
    bool dieOk   = sndDie.openFromFile(SND_DIE);

    // Objects
    Background bg;
    Bird        bird;
    Ground      ground;
    vector<Pipe> pipes;
    Clock        spawnClock;

    // Game vars
    int   score     = 0;
    int   highScore = 0;
    float pipeSpeed = PSPEED_BASE;
    State state     = State::Menu;

    auto resetGame = [&]() {
        if (score > highScore) highScore = score;
        bird.reset();
        pipes.clear();
        score      = 0;
        pipeSpeed  = PSPEED_BASE;
        state      = State::Playing;
        spawnClock.restart();
    };

    // ── Game Loop ────────────────────────────────────────────────
    while (window.isOpen()) {

        // Events
        Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == Event::Closed)
                window.close();

            if (ev.type == Event::KeyPressed) {
                switch (ev.key.code) {

                    case Keyboard::Escape:
                        window.close();
                        break;

                    case Keyboard::Space:
                    case Keyboard::Up:
                        if (state == State::Menu)    resetGame();
                        else if (state == State::Playing) bird.flap();
                        else if (state == State::Paused)  state = State::Playing;
                        break;

                    case Keyboard::P:
                        if      (state == State::Playing) state = State::Paused;
                        else if (state == State::Paused)  state = State::Playing;
                        break;

                    case Keyboard::R:
                        if (state == State::Over || state == State::Paused)
                            resetGame();
                        break;

                    default: break;
                }
            }
        }

        // ── Update ───────────────────────────────────────────────
        bg.update();

        if (state == State::Playing) {

            bird.update();
            ground.update(pipeSpeed);

            // Spawn أنبوبة جديدة
            if (spawnClock.getElapsedTime().asMilliseconds() > SPAWN_MS) {
                spawnClock.restart();
                float minY = 60.f;
                float maxY = H - GROUND_H - PGAP - 60.f;
                float gy   = minY + (float)(rand() % (int)(maxY - minY));
                pipes.emplace_back((float)W, gy, pipeSpeed);
            }

            // Update Pipes
            for (int i = (int)pipes.size() - 1; i >= 0; --i) {
                pipes[i].update();

                // تصادم بأنبوبة
                if (pipes[i].checkCollision(bird.getBounds())) {
                    if (dieOk) sndDie.play();
                    if (score > highScore) highScore = score;
                    state = State::Over;
                    break;
                }

                // تصادم بالأرض
                if (ground.hits(bird.getBounds())) {
                    if (dieOk) sndDie.play();
                    if (score > highScore) highScore = score;
                    state = State::Over;
                    break;
                }

                // عدي الأنبوبة
                if (pipes[i].hasPassed(bird.getBounds().left + bird.getBounds().width)) {
                    pipes[i].scored = true;
                    score++;
                    // تسريع تدريجي كل 5 نقاط
                    if (score % 5 == 0) {
                        pipeSpeed += PSPEED_STEP;
                        if (pipeSpeed > PSPEED_MAX) pipeSpeed = PSPEED_MAX;
                    }
                    if (scoreOk && sndScore.getStatus() != Music::Playing)
                        sndScore.play();
                }

                if (pipes[i].isOffScreen())
                    pipes.erase(pipes.begin() + i);
            }

            // تصادم بالأرض (لو مفيش أنابيب)
            if (state == State::Playing && ground.hits(bird.getBounds())) {
                if (dieOk) sndDie.play();
                if (score > highScore) highScore = score;
                state = State::Over;
            }
        }

        // ── Render ───────────────────────────────────────────────
        window.clear(Color(113,197,207));
        bg.draw(window);
        for (auto& p : pipes) p.draw(window);
        bird.draw(window);
        ground.draw(window);

        // HUD
        if (fontOk) {
            // Score
            window.draw(makeTxt(font, to_string(score), 52,
                                Color::White, -1.f, 20.f));
            // High Score
            window.draw(makeTxt(font, "Best: " + to_string(highScore),
                                22, Color::Yellow, 10.f, 10.f));
            // Speed indicator
            ostringstream sp;
            sp << "Speed: x" << (int)((pipeSpeed / PSPEED_BASE) * 10) / 10.0;
            window.draw(makeTxt(font, sp.str(), 18,
                                Color(220,220,220), 10.f, (float)H - GROUND_H - 26.f,
                                Color::Black, 1.5f));
            // Controls hint
            window.draw(makeTxt(font, "[Space/Up] Flap  [P] Pause  [R] Restart  [Esc] Quit",
                                14, Color(80,80,80),
                                (float)W/2.f - 220.f, (float)H - GROUND_H + 12.f,
                                Color::Transparent, 0.f));
        }

        // ── Overlays ─────────────────────────────────────────────
        auto overlay = [&](Uint8 alpha) {
            RectangleShape ov(Vector2f((float)W, (float)H));
            ov.setFillColor(Color(0,0,0,alpha));
            window.draw(ov);
        };

        if (state == State::Menu && fontOk) {
            overlay(100);
            window.draw(makeTxt(font, "FLAPPY BIRD", 60, Color::Yellow, -1.f, H/2.f-120.f));
            window.draw(makeTxt(font, "Press SPACE to Start", 28, Color::White, -1.f, H/2.f+10.f));
            if (highScore > 0)
                window.draw(makeTxt(font, "Best: "+to_string(highScore), 22,
                                    Color::Yellow, -1.f, H/2.f+55.f));
        }

        if (state == State::Paused && fontOk) {
            overlay(140);
            window.draw(makeTxt(font, "PAUSED", 64, Color::Yellow, -1.f, H/2.f-60.f));
            window.draw(makeTxt(font, "SPACE to Resume  |  R to Restart",
                                22, Color::White, -1.f, H/2.f+30.f));
        }

        if (state == State::Over && fontOk) {
            overlay(150);
            window.draw(makeTxt(font, "GAME OVER", 60, Color::Red, -1.f, H/2.f-100.f));
            window.draw(makeTxt(font, "Score: "+to_string(score), 40,
                                Color::White, -1.f, H/2.f-20.f));
            if (score >= highScore && score > 0)
                window.draw(makeTxt(font, "NEW BEST!", 28, Color::Yellow, -1.f, H/2.f+35.f));
            window.draw(makeTxt(font, "Press R to Restart", 26,
                                Color(220,220,220), -1.f, H/2.f+75.f));
        }

        window.display();
    }

    return 0;
}
