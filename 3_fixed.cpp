#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
using namespace sf;
using namespace std;

const int    WINDOW_WIDTH  = 800;
const int    WINDOW_HEIGHT = 600;
const string WINDOW_TITLE  = "Bouncing Ball";
const string SOUND_FILE    = "C:\\Users\\engal\\Desktop\\app\\assets\\Sounds\\flap.wav";
const float  BALL_RADIUS   = 25.f;
const float  BALL_SPEED    = 300.f;
const float  BALL_START_X  = 375.f;
const float  BALL_START_Y  = 275.f;

void initWindow(RenderWindow& window) {
    window.create(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), WINDOW_TITLE);
    window.setFramerateLimit(60);   // FIX 1: حد الـ FPS
}

CircleShape createBall() {
    CircleShape ball(BALL_RADIUS);
    ball.setFillColor(Color::Green);
    ball.setPosition(BALL_START_X, BALL_START_Y);
    return ball;
}

bool loadSound(Music& music) {
    return music.openFromFile(SOUND_FILE);
}

void handleEvents(RenderWindow& window) {
    Event event;
    while (window.pollEvent(event))
        if (event.type == Event::Closed)
            window.close();
}

void updateBall(CircleShape& ball, Vector2f& velocity, Music& music, float deltaTime) {
    ball.move(velocity * deltaTime);

    // FIX 2: كانت بتستخدم BALL_RADIUS * 2 بس الصح هو diameter للحافة اليمين/أسفل
    float diameter = BALL_RADIUS * 2.f;

    if (ball.getPosition().x <= 0) {
        ball.setPosition(0, ball.getPosition().y);   // FIX 3: منع الغروق في الحائط
        velocity.x = abs(velocity.x);
        music.play();
    }
    if (ball.getPosition().x + diameter >= WINDOW_WIDTH) {
        ball.setPosition(WINDOW_WIDTH - diameter, ball.getPosition().y);
        velocity.x = -abs(velocity.x);
        music.play();
    }
    if (ball.getPosition().y <= 0) {
        ball.setPosition(ball.getPosition().x, 0);
        velocity.y = abs(velocity.y);
        music.play();
    }
    if (ball.getPosition().y + diameter >= WINDOW_HEIGHT) {
        ball.setPosition(ball.getPosition().x, WINDOW_HEIGHT - diameter);
        velocity.y = -abs(velocity.y);
        music.play();
    }
}

void render(RenderWindow& window, CircleShape& ball) {
    window.clear(Color::Black);
    window.draw(ball);
    window.display();
}

int main() {
    RenderWindow window;
    initWindow(window);

    Music music;
    if (!loadSound(music)) return -1;

    CircleShape ball     = createBall();
    Vector2f    velocity = Vector2f(BALL_SPEED, BALL_SPEED);
    Clock       clock;

    while (window.isOpen()) {
        float deltaTime = clock.restart().asSeconds();
        handleEvents(window);
        updateBall(ball, velocity, music, deltaTime);
        render(window, ball);
    }
    return 0;
}
