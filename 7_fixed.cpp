#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
using namespace std;
using namespace sf;

const int    W = 800, H = 600;
const string TITLE = "Flappy Bird", FONT = "arial.ttf";
const string SND = "C:\\Users\\engal\\Desktop\\app\\assets\\sounds\\score.wav";
const string BG = "C:\\Users\\engal\\Desktop\\app\\assets\\images\\bird.png";
const float  GRAV = 0.25f, FLAP = -6.f, MAX_FALL = 8.f;
const float  PW = 70.f, PGAP = 180.f, PSPEED = 3.f;
const int    SPAWN = 1800;

class Bird {
    CircleShape body, eye;
    ConvexShape beak, wingT, wingB;
    void moveAll(float dx, float dy) { body.move(dx, dy); eye.move(dx, dy); beak.move(dx, dy); wingT.move(dx, dy); wingB.move(dx, dy); }
    void clamp() {
        float r = body.getRadius(), py = body.getPosition().y;
        if (py - r < 0) { moveAll(0, r - py);      vel = 0; }
        if (py + r > H) { moveAll(0, H - r - py);  vel = 0; }
    }
    void rot(float a) { body.setRotation(a); eye.setRotation(a); beak.setRotation(a); wingT.setRotation(a); wingB.setRotation(a); }
public:
    float vel = 0;
    Bird() {
        float cx = W / 4.f, cy = H / 2.f;
        body.setRadius(16); body.setOrigin(16, 16); body.setPosition(cx, cy);
        body.setFillColor({ 255,210,0 }); body.setOutlineThickness(2); body.setOutlineColor({ 200,160,0 });
        eye.setRadius(5); eye.setOrigin(5, 5); eye.setPosition(cx + 10, cy - 5);
        eye.setFillColor(Color::White); eye.setOutlineThickness(1.5f); eye.setOutlineColor(Color::Black);
        beak.setPointCount(3); beak.setPoint(0, { 0,0 }); beak.setPoint(1, { 14,4 }); beak.setPoint(2, { 0,8 });
        beak.setFillColor({ 255,120,0 }); beak.setOrigin(0, 4); beak.setPosition(cx + 16, cy);
        wingT.setPointCount(4); wingT.setPoint(0, { 0,0 }); wingT.setPoint(1, { -10,-8 }); wingT.setPoint(2, { -22,-4 }); wingT.setPoint(3, { -12,6 });
        wingT.setFillColor({ 255,170,0 }); wingT.setPosition(cx, cy - 4);
        wingB.setPointCount(4); wingB.setPoint(0, { 0,0 }); wingB.setPoint(1, { -8,6 }); wingB.setPoint(2, { -18,4 }); wingB.setPoint(3, { -10,-4 });
        wingB.setFillColor({ 200,130,0 }); wingB.setPosition(cx, cy + 4);
    }
    void flap() { vel = FLAP; }
    void update() {
        vel += GRAV; if (vel > MAX_FALL) vel = MAX_FALL;
        float a = vel * 4.f; if (a > 45) a = 45; if (a < -25) a = -25;
        rot(a); moveAll(0, vel); clamp();
    }
    void reset() { float cx = W / 4.f, cy = H / 2.f, dx = cx - body.getPosition().x, dy = cy - body.getPosition().y; moveAll(dx, dy); vel = 0; rot(0); }
    FloatRect getBounds() const { return body.getGlobalBounds(); }
    void draw(RenderWindow& w) { w.draw(wingB); w.draw(body); w.draw(wingT); w.draw(beak); w.draw(eye); }
};

class Pipe {
public:
    RectangleShape tp, bp, tc, bc;
    bool scored = false;
    Pipe(float x, float gy) {
        Color g{ 80,180,60 }, cg{ 60,150,40 };
        float bh = max(10.f, H - gy - PGAP - 20);
        tp.setSize({ PW, gy - 20 }); tp.setFillColor(g); tp.setPosition(x, 0);
        tc.setSize({ PW + 10, 20 });  tc.setFillColor(cg); tc.setPosition(x - 5, gy - 20);
        bc.setSize({ PW + 10, 20 });  bc.setFillColor(cg); bc.setPosition(x - 5, gy + PGAP);
        bp.setSize({ PW, bh });     bp.setFillColor(g);  bp.setPosition(x, gy + PGAP + 20);
    }
    void move() { tp.move(-PSPEED, 0); bp.move(-PSPEED, 0); tc.move(-PSPEED, 0); bc.move(-PSPEED, 0); }
    bool isOffScreen() { return tp.getPosition().x + PW < 0; }
    bool hasPassed(float bx) { return !scored && tp.getPosition().x + PW < bx; }
    bool checkCollision(const FloatRect& b) { return tp.getGlobalBounds().intersects(b) || tc.getGlobalBounds().intersects(b) || bp.getGlobalBounds().intersects(b) || bc.getGlobalBounds().intersects(b); }
    void draw(RenderWindow& w) { w.draw(tp); w.draw(tc); w.draw(bp); w.draw(bc); }
};

int main() {
    srand((unsigned)time(0));
    RenderWindow window(VideoMode(W, H), TITLE);
    window.setFramerateLimit(60);

    Texture bgt; Sprite bg;
    if (!bgt.loadFromFile(BG)) return -1;
    bg.setTexture(bgt); bg.setScale((float)W / bgt.getSize().x, (float)H / bgt.getSize().y);

    Music music; if (!music.openFromFile(SND)) return -1;
    Font font;   if (!font.loadFromFile(FONT)) return -1;

    Text scoreText; scoreText.setFont(font); scoreText.setCharacterSize(30);
    scoreText.setFillColor(Color::White); scoreText.setOutlineThickness(2); scoreText.setOutlineColor(Color::Black);
    scoreText.setPosition(10, 10);

    Bird bird; vector<Pipe> pipes; Clock spawnClock;
    int score = 0; bool over = false;

    while (window.isOpen()) {
        Event e;
        while (window.pollEvent(e)) {
            if (e.type == Event::Closed) window.close();
            if (e.type == Event::KeyPressed) {
                if ((e.key.code == Keyboard::Space || e.key.code == Keyboard::Up) && !over) bird.flap();
                if (e.key.code == Keyboard::R && over) { over = false; score = 0; pipes.clear(); spawnClock.restart(); bird.reset(); }
            }
        }
        if (!over) {
            bird.update();
            if (spawnClock.getElapsedTime().asMilliseconds() > SPAWN) {
                spawnClock.restart();
                pipes.push_back(Pipe((float)W, (float)(60 + rand() % (int)(H - PGAP - 120))));
            }
            for (size_t i = 0; i < pipes.size(); ++i) {
                pipes[i].move();
                if (pipes[i].hasPassed(bird.getBounds().left + bird.getBounds().width)) { pipes[i].scored = true; ++score; music.play(); }
                if (pipes[i].checkCollision(bird.getBounds())) over = true;
                if (pipes[i].isOffScreen()) { pipes.erase(pipes.begin() + i); --i; }
            }
        }
        window.clear({ 113,197,207 }); window.draw(bg);
        for (auto& p : pipes) p.draw(window);
        bird.draw(window);
        scoreText.setString("Score: " + to_string(score)); window.draw(scoreText);
        if (over) {
            RectangleShape ov({ (float)W,(float)H }); ov.setFillColor({ 0,0,0,120 }); window.draw(ov);
            Text t1("Game Over!", font, 60); t1.setFillColor(Color::White); t1.setOutlineThickness(3); t1.setOutlineColor(Color::Black);
            t1.setPosition(W / 2.f - t1.getLocalBounds().width / 2.f, H / 2.f - 70); window.draw(t1);
            Text t2("Score: " + to_string(score), font, 40); t2.setFillColor(Color::Yellow); t2.setOutlineThickness(2); t2.setOutlineColor(Color::Black);
            t2.setPosition(W / 2.f - t2.getLocalBounds().width / 2.f, H / 2.f); window.draw(t2);
            Text t3("Press R to Restart", font, 28); t3.setFillColor(Color::White); t3.setOutlineThickness(2); t3.setOutlineColor(Color::Black);
            t3.setPosition(W / 2.f - t3.getLocalBounds().width / 2.f, H / 2.f + 60); window.draw(t3);
        }
        window.display();
    }
    return 0;
}