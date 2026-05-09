#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <string>
#include <sstream>
#include <iomanip>
using namespace sf;
using namespace std;

// ─── Constants ───────────────────────────────────────────────────────────────
const int    WINDOW_WIDTH  = 800;
const int    WINDOW_HEIGHT = 600;
const string WINDOW_TITLE  = "SFML Interactive Demo";

const string IMAGE_FILE    = "assets/bg.jpg";
const string FONT_FILE     = "assets/arial.ttf";
const string SOUND_FILE    = "assets/Ramadan.mp3";

const string TEXT_STRING   = "Hello SFML";
const int    TEXT_SIZE     = 60;

// ─── Helpers ─────────────────────────────────────────────────────────────────
Text makeText(const Font& f, const string& s, unsigned sz,
              Color fill, float x, float y) {
    Text t(s, f, sz);
    t.setFillColor(fill);
    t.setPosition(x, y);
    return t;
}

// ─── Main ────────────────────────────────────────────────────────────────────
int main() {

    // ── Window ───────────────────────────────────────────────────
    RenderWindow window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), WINDOW_TITLE);
    window.setFramerateLimit(60);

    // ── Texture / Background ─────────────────────────────────────
    Texture bgTex;
    Sprite  bgSprite;
    bool    hasImage = bgTex.loadFromFile(IMAGE_FILE) ||
                       bgTex.loadFromFile("bg.jpg");
    if (hasImage) {
        bgSprite.setTexture(bgTex);
        // تمديد الصورة لتملأ النافذة بالكامل
        bgSprite.setScale(
            (float)WINDOW_WIDTH  / bgTex.getSize().x,
            (float)WINDOW_HEIGHT / bgTex.getSize().y
        );
    }

    // ── Font ─────────────────────────────────────────────────────
    Font font;
    bool fontOk = font.loadFromFile(FONT_FILE)                    ||
                  font.loadFromFile("C:/Windows/Fonts/arial.ttf") ||
                  font.loadFromFile("arial.ttf")                  ||
                  font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");

    // ── Sound ────────────────────────────────────────────────────
    Music music;
    bool  soundOk = music.openFromFile(SOUND_FILE) ||
                    music.openFromFile("Ramadan.mp3");
    float volume  = 50.f;
    bool  muted   = false;
    if (soundOk) {
        music.setLoop(true);
        music.setVolume(volume);
        music.play();
    }

    // ── Main Text ────────────────────────────────────────────────
    // نص يتحرك ويتلوّن
    float  textX    = 100.f, textY = 50.f;
    float  textVX   = 120.f, textVY = 80.f;   // سرعة الحركة (pixel/sec)
    float  hue      = 0.f;                     // للتدوير على الألوان
    bool   animated = true;

    // ── Clock & FPS ──────────────────────────────────────────────
    Clock  clock;
    Clock  fpsClock;
    int    frames   = 0;
    float  fps      = 0.f;

    // ─── Game Loop ───────────────────────────────────────────────
    while (window.isOpen()) {

        float dt = clock.restart().asSeconds();
        if (dt > 0.05f) dt = 0.05f;

        // FPS حساب
        frames++;
        float elapsed = fpsClock.getElapsedTime().asSeconds();
        if (elapsed >= 0.5f) {
            fps = frames / elapsed;
            frames = 0;
            fpsClock.restart();
        }

        // ── Events ───────────────────────────────────────────────
        Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == Event::Closed)
                window.close();

            if (ev.type == Event::KeyPressed) {
                switch (ev.key.code) {

                    // خروج
                    case Keyboard::Escape:
                        window.close();
                        break;

                    // إيقاف/تشغيل الحركة
                    case Keyboard::Space:
                        animated = !animated;
                        break;

                    // تحكم بالصوت
                    case Keyboard::Up:
                        if (soundOk && !muted) {
                            volume = min(100.f, volume + 10.f);
                            music.setVolume(volume);
                        }
                        break;
                    case Keyboard::Down:
                        if (soundOk && !muted) {
                            volume = max(0.f, volume - 10.f);
                            music.setVolume(volume);
                        }
                        break;
                    case Keyboard::M:
                        if (soundOk) {
                            muted = !muted;
                            music.setVolume(muted ? 0.f : volume);
                        }
                        break;

                    // إيقاف/تشغيل الموسيقى
                    case Keyboard::P:
                        if (soundOk) {
                            if (music.getStatus() == Music::Playing)
                                music.pause();
                            else
                                music.play();
                        }
                        break;

                    default: break;
                }
            }
        }

        // ── Update ───────────────────────────────────────────────
        if (animated && fontOk) {
            // حركة النص
            textX += textVX * dt;
            textY += textVY * dt;

            // تقدير حجم النص
            float tw = (float)(TEXT_STRING.size() * TEXT_SIZE * 0.55f);
            float th = (float)TEXT_SIZE;

            if (textX < 0)                      { textX = 0;                   textVX =  abs(textVX); }
            if (textX + tw > WINDOW_WIDTH)      { textX = WINDOW_WIDTH - tw;   textVX = -abs(textVX); }
            if (textY < 0)                      { textY = 0;                   textVY =  abs(textVY); }
            if (textY + th > WINDOW_HEIGHT - 80){ textY = WINDOW_HEIGHT-80-th; textVY = -abs(textVY); }

            // تلوين قوس قزح
            hue += 90.f * dt;
            if (hue >= 360.f) hue -= 360.f;
        }

        // تحويل hue → RGB بسيط
        auto hueToColor = [](float h) -> Color {
            h = fmodf(h, 360.f);
            float s = 1.f, v = 1.f;
            int   i = (int)(h / 60.f) % 6;
            float f = h / 60.f - (int)(h / 60.f);
            float p = v * (1 - s);
            float q = v * (1 - f * s);
            float t = v * (1 - (1 - f) * s);
            switch (i) {
                case 0: return Color((Uint8)(v*255),(Uint8)(t*255),(Uint8)(p*255));
                case 1: return Color((Uint8)(q*255),(Uint8)(v*255),(Uint8)(p*255));
                case 2: return Color((Uint8)(p*255),(Uint8)(v*255),(Uint8)(t*255));
                case 3: return Color((Uint8)(p*255),(Uint8)(q*255),(Uint8)(v*255));
                case 4: return Color((Uint8)(t*255),(Uint8)(p*255),(Uint8)(v*255));
                default:return Color((Uint8)(v*255),(Uint8)(p*255),(Uint8)(q*255));
            }
        };

        // ── Render ───────────────────────────────────────────────
        window.clear(Color(30, 30, 50));

        // خلفية
        if (hasImage)
            window.draw(bgSprite);
        else {
            // Gradient بديل لو مفيش صورة
            RectangleShape grad(Vector2f((float)WINDOW_WIDTH, (float)WINDOW_HEIGHT));
            grad.setFillColor(Color(40, 60, 100));
            window.draw(grad);
        }

        // النص الرئيسي
        if (fontOk) {
            Text mainText(TEXT_STRING, font, TEXT_SIZE);
            mainText.setFillColor(hueToColor(hue));
            mainText.setOutlineColor(Color::Black);
            mainText.setOutlineThickness(3.f);
            mainText.setStyle(Text::Bold);
            mainText.setPosition(textX, textY);
            window.draw(mainText);
        }

        // ── HUD Panel (شريط معلومات في الأسفل) ──────────────────
        if (fontOk) {
            // خلفية الشريط
            RectangleShape panel(Vector2f((float)WINDOW_WIDTH, 80.f));
            panel.setFillColor(Color(0, 0, 0, 160));
            panel.setPosition(0.f, (float)WINDOW_HEIGHT - 80.f);
            window.draw(panel);

            // FPS
            ostringstream fpsStr;
            fpsStr << fixed << setprecision(1) << "FPS: " << fps;
            window.draw(makeText(font, fpsStr.str(), 18, Color::Green,
                                 10.f, (float)WINDOW_HEIGHT - 76.f));

            // حالة الصوت
            string volStr;
            if (!soundOk)       volStr = "No Audio File";
            else if (muted)     volStr = "Sound: MUTED";
            else {
                ostringstream vs;
                vs << "Volume: " << (int)volume << "%  "
                   << (music.getStatus() == Music::Playing ? "[Playing]" : "[Paused]");
                volStr = vs.str();
            }
            window.draw(makeText(font, volStr, 18, Color::Cyan,
                                 10.f, (float)WINDOW_HEIGHT - 54.f));

            // حالة الحركة
            string animStr = animated ? "Animation: ON" : "Animation: OFF";
            window.draw(makeText(font, animStr, 18,
                                 animated ? Color::Yellow : Color(150,150,150),
                                 10.f, (float)WINDOW_HEIGHT - 32.f));

            // Controls
            string ctrl = "[Space] Toggle Anim  [Up/Down] Volume  [M] Mute  [P] Pause Music  [Esc] Quit";
            window.draw(makeText(font, ctrl, 13, Color(120,120,120),
                                 (float)WINDOW_WIDTH/2.f - 300.f,
                                 (float)WINDOW_HEIGHT - 20.f));
        }

        window.display();
    }

    return 0;
}
