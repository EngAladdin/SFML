#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
using namespace std;
using namespace sf;

const int    WINDOW_WIDTH = 800, WINDOW_HEIGHT = 600;
const string WINDOW_TITLE = "Angel's Cakes Game", FONT_FILE = "arial.ttf";
const string SOUND_FILE = "C:\\Users\\engal\\Desktop\\app\\assets\\sounds\\die.wav";
const string PLAYER_FILE = "C:\\Users\\engal\\Desktop\\app\\assets\\images\\bird.png";
const string CAKE_FILE = "C:\\Users\\engal\\Desktop\\app\\assets\\images\\cloud.png";
const float  PLAYER_SPEED = 300.f, CAKE_FALL_SPEED = 200.f;
const int    CAKE_SPAWN_RATE = 3, TEXT_SIZE = 30;

class Player {
    Texture texture;
public:
    Sprite sprite;
    Player() {
        if (!texture.loadFromFile(PLAYER_FILE)) return;
        sprite.setTexture(texture);
        sprite.setPosition(WINDOW_WIDTH / 2.f, WINDOW_HEIGHT - 100.f);
    }
    void move(float deltaX, float deltaTime) {
        sprite.move(deltaX * deltaTime, 0);
        float posX = sprite.getPosition().x;
        if (posX < 0) sprite.setPosition(0, sprite.getPosition().y);
        if (posX > WINDOW_WIDTH - sprite.getGlobalBounds().width)
            sprite.setPosition(WINDOW_WIDTH - sprite.getGlobalBounds().width, sprite.getPosition().y);
    }
    void draw(RenderWindow& window) { window.draw(sprite); }
};

class Cake {
    Texture texture;
public:
    Sprite sprite;
    Cake()
    {
        if (!texture.loadFromFile(CAKE_FILE)) return;
        sprite.setTexture(texture);
        sprite.setPosition((float)(rand() % (WINDOW_WIDTH - 50)), -50.f);
    }
    void update(float deltaTime) { sprite.move(0, CAKE_FALL_SPEED * deltaTime); }
    bool isOffScreen() { return sprite.getPosition().y > WINDOW_HEIGHT; }
    bool checkCollision(Sprite& playerSprite) { return sprite.getGlobalBounds().intersects(playerSprite.getGlobalBounds()); }
    void draw(RenderWindow& window) { window.draw(sprite); }
};

int main() {
    srand((unsigned)time(0));

    RenderWindow window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), WINDOW_TITLE);
    window.setFramerateLimit(60);

    Music music; if (!music.openFromFile(SOUND_FILE)) return -1;
    Font  font;  if (!font.loadFromFile(FONT_FILE))   return -1;

    Text scoreText; scoreText.setFont(font); scoreText.setCharacterSize(TEXT_SIZE);
    scoreText.setFillColor(Color::White); scoreText.setOutlineThickness(2);
    scoreText.setOutlineColor(Color::Black); scoreText.setPosition(10, 10);

    Player player; vector<Cake> cakes; Clock clock; int score = 0;

    while (window.isOpen()) {
        float deltaTime = clock.restart().asSeconds();

        Event event;
        while (window.pollEvent(event)) if (event.type == Event::Closed) window.close();

        if (Keyboard::isKeyPressed(Keyboard::Left))  player.move(-PLAYER_SPEED, deltaTime);
        if (Keyboard::isKeyPressed(Keyboard::Right)) player.move(PLAYER_SPEED, deltaTime);

        if (rand() % 100 < CAKE_SPAWN_RATE) cakes.push_back(Cake());

        for (auto it = cakes.begin(); it != cakes.end();) {
            it->update(deltaTime);
            if (it->checkCollision(player.sprite)) { ++score; music.play(); it = cakes.erase(it); }
            else if (it->isOffScreen()) { it = cakes.erase(it); }
            else { ++it; }
        }

        window.clear();
        player.draw(window);
        for (auto& cake : cakes) cake.draw(window);
        scoreText.setString("Score: " + to_string(score));
        window.draw(scoreText);
        window.display();
    }
    return 0;
}