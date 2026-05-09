#include <SFML/Graphics.hpp>
#include <string>
using namespace std;
using namespace sf;

const int    WINDOW_WIDTH  = 300;
const int    WINDOW_HEIGHT = 300;
const string WINDOW_TITLE  = "Tic-Tac-Toe";
const string FONT_FILE     = "arial.ttf";
const int    CELL_SIZE     = 100;
const float  LINE_THICK    = 10.f;
const int    TEXT_SIZE     = 80;
const float  TEXT_OFFSET_X = 25.f;
const float  TEXT_OFFSET_Y = 10.f;

enum class Player { None, X, O };

class TicTacToe {
    RenderWindow window;
    Font         font;
    Player       board[3][3];
    Player       currentPlayer;
    bool         gameOver;       // FIX 1: متغير يمنع اللعب بعد الفوز/التعادل

    void initWindow() {
        window.create(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), WINDOW_TITLE);
        window.setFramerateLimit(60);   // FIX 2: حد الـ FPS
    }

    void initFont() {
        if (!font.loadFromFile(FONT_FILE)) window.close();
    }

    void initBoard() {
        for (int row = 0; row < 3; row++)
            for (int col = 0; col < 3; col++)
                board[row][col] = Player::None;
    }

    void handleEvents() {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) window.close();
            if (event.type == Event::MouseButtonPressed) handleMouseClick(event);
        }
    }

    void handleMouseClick(Event& event) {
        if (event.mouseButton.button != Mouse::Left) return;
        if (gameOver) return;  

        int row = event.mouseButton.y / CELL_SIZE;
        int col = event.mouseButton.x / CELL_SIZE;
        if (row >= 3 || col >= 3) return;
        if (board[row][col] != Player::None) return;

        board[row][col] = currentPlayer;

        // FIX 3: التحقق من الفوز بعد كل حركة مباشرةً قبل تبديل اللاعب
        if (checkWin()) {
            string winner = (currentPlayer == Player::X) ? "Player X" : "Player O";  // FIX 4: كان معكوس
            cout << winner << " wins!" << endl;
            gameOver = true;
            return;
        }
        if (checkDraw()) {
            cout << "It's a draw!" << endl;
            gameOver = true;
            return;
        }

        currentPlayer = (currentPlayer == Player::X) ? Player::O : Player::X;
    }

    void drawGrid() {
        for (int i = 1; i < 3; i++) {
            RectangleShape hLine(Vector2f((float)WINDOW_WIDTH, LINE_THICK));
            hLine.setFillColor(Color::Black);
            hLine.setPosition(0.f, (float)(CELL_SIZE * i) - LINE_THICK / 2.f);  // FIX 5: توسيط الخطوط
            window.draw(hLine);

            RectangleShape vLine(Vector2f(LINE_THICK, (float)WINDOW_HEIGHT));
            vLine.setFillColor(Color::Black);
            vLine.setPosition((float)(CELL_SIZE * i) - LINE_THICK / 2.f, 0.f);
            window.draw(vLine);
        }
    }

    void drawSymbol(const string& symbol, Color color, int row, int col) {
        Text text(symbol, font, TEXT_SIZE);
        text.setFillColor(color);
        text.setPosition((float)(col * CELL_SIZE) + TEXT_OFFSET_X,
                         (float)(row * CELL_SIZE) + TEXT_OFFSET_Y);
        window.draw(text);
    }

    void render() {
        window.clear(Color::White);
        drawGrid();
        for (int row = 0; row < 3; row++)
            for (int col = 0; col < 3; col++) {
                if (board[row][col] == Player::X) drawSymbol("X", Color::Red,  row, col);
                if (board[row][col] == Player::O) drawSymbol("O", Color::Blue, row, col);
            }
        window.display();
    }

    bool checkWin() {
        for (int i = 0; i < 3; i++) {
            if (board[i][0] != Player::None && board[i][0] == board[i][1] && board[i][1] == board[i][2]) return true;
            if (board[0][i] != Player::None && board[0][i] == board[1][i] && board[1][i] == board[2][i]) return true;
        }
        if (board[0][0] != Player::None && board[0][0] == board[1][1] && board[1][1] == board[2][2]) return true;
        if (board[0][2] != Player::None && board[0][2] == board[1][1] && board[1][1] == board[2][0]) return true;
        return false;
    }

    bool checkDraw() {
        for (int row = 0; row < 3; row++)
            for (int col = 0; col < 3; col++)
                if (board[row][col] == Player::None) return false;
        return true;
    }

public:
    TicTacToe() {
        initWindow();
        initFont();
        initBoard();
        currentPlayer = Player::X;
        gameOver      = false;
    }

    void run() {
        while (window.isOpen()) {
            handleEvents();
            render();
        }
    }
};

int main() {
    TicTacToe game;
    game.run();
    return 0;
}
