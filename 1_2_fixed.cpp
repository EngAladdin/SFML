#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <string>
using namespace sf;
using namespace std;

const int    WINDOW_WIDTH  = 800;
const int    WINDOW_HEIGHT = 600;
const string WINDOW_TITLE  = "SFML window";
const string IMAGE_FILE    = "C:\\Users\\engal\\Desktop\\app\\assets\\bg.jpg";
const string FONT_FILE     = "C:\\Users\\engal\\Desktop\\app\\assets\\arial.ttf";
const string SOUND_FILE    = "C:\\Users\\engal\\Desktop\\app\\assets\\Ramadan.mp3";
const string TEXT_STRING   = "Hello SFML";
const int    TEXT_SIZE     = 50;
const float  TEXT_X        = 100.f;
const float  TEXT_Y        = 50.f;
const float  IMAGE_SCALE   = 0.1f;

void initWindow(RenderWindow& window) {
    window.create(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), WINDOW_TITLE);
    window.setFramerateLimit(60);          // FIX 1: حد الـ FPS
}

bool loadSprite(Texture& texture, Sprite& sprite) {
    if (!texture.loadFromFile(IMAGE_FILE)) return false;
    sprite.setTexture(texture);
    sprite.setScale(IMAGE_SCALE, IMAGE_SCALE);
    return true;
}

bool loadText(Font& font, Text& text) {
    if (!font.loadFromFile(FONT_FILE)) return false;
    text.setFont(font);
    text.setString(TEXT_STRING);
    text.setCharacterSize(TEXT_SIZE);
    text.setFillColor(Color::Yellow);
    text.setPosition(TEXT_X, TEXT_Y);
    return true;
}

bool loadAndPlaySound(Music& sound) {
    if (!sound.openFromFile(SOUND_FILE)) return false;
    sound.setLoop(true);
    sound.play();
    return true;
}

void handleEvents(RenderWindow& window) {
    Event event;
    while (window.pollEvent(event))
        if (event.type == Event::Closed)
            window.close();
}

void render(RenderWindow& window, Sprite& sprite, Text& text) {
    window.clear();
    window.draw(sprite);
    window.draw(text);
    window.display();
}

int main() {
    RenderWindow window;
    initWindow(window);

    Texture texture;
    Sprite  sprite;
    if (!loadSprite(texture, sprite)) return -1;  // FIX 2: texture لازم تعيش طول عمر الـ sprite

    Font font;
    Text text;
    if (!loadText(font, text)) return -1;          // FIX 3: font لازم تعيش طول عمر الـ text

    Music sound;
    if (!loadAndPlaySound(sound)) return -1;

    while (window.isOpen()) {
        handleEvents(window);
        render(window, sprite, text);
    }
    return 0;
}
