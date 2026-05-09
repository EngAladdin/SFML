#include <SFML/Graphics.hpp>
#include <string>
#include <sstream>
using namespace std;
using namespace sf;

// ─── Constants ───────────────────────────────────────────────────────────────
const int    WINDOW_WIDTH   = 300;
const int    WINDOW_HEIGHT  = 380;   // +80 للـ HUD في الأسفل
const string WINDOW_TITLE   = "Tic-Tac-Toe";
const string FONT_FILE      = "arial.ttf";
const int    CELL_SIZE      = 100;
const float  LINE_THICK     = 8.f;
const int    TEXT_SIZE      = 80;
const float  TEXT_OFFSET_X  = 22.f;
const float  TEXT_OFFSET_Y  = 8.f;
const int    BOARD_HEIGHT   = 300;   // منطقة الشبكة فقط

enum class Player { None, X, O };

// ─── Helper ──────────────────────────────────────────────────────────────────
Text makeText(const Font& font, const string& str,
              unsigned size, Color color, float x, float y) {
    Text t(str, font, size);
    t.setFillColor(color);
    t.setPosition(x, y);
    return t;
}

// ─── Game Class ──────────────────────────────────────────────────────────────
class TicTacToe {
    RenderWindow window;
    Font         font;
    bool         fontOk;

    Player  board[3][3];
    Player  currentPlayer;
    Player  winner;
    bool    gameOver;
    bool    isDraw;

    // خط الفوز
    int  winLine[3][2];   // [0..2] = { {row,col}, {row,col}, {row,col} }
    bool hasWinLine;

    // نقاط الجلستين
    int scoreX, scoreO, scoreDraw;

    // ─── Init ────────────────────────────────────────────────────
    void initWindow() {
        window.create(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), WINDOW_TITLE);
        window.setFramerateLimit(60);
    }

    void resetBoard() {
        for (int r = 0; r < 3; r++)
            for (int c = 0; c < 3; c++)
                board[r][c] = Player::None;
        currentPlayer = Player::X;
        winner        = Player::None;
        gameOver      = false;
        isDraw        = false;
        hasWinLine    = false;
    }

    // ─── Events ──────────────────────────────────────────────────
    void handleEvents() {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();

            if (event.type == Event::KeyPressed) {
                if (event.key.code == Keyboard::R)
                    resetBoard();
                if (event.key.code == Keyboard::Escape)
                    window.close();
            }

            if (event.type == Event::MouseButtonPressed &&
                event.mouseButton.button == Mouse::Left)
                handleClick(event.mouseButton.x, event.mouseButton.y);
        }
    }

    void handleClick(int mx, int my) {
        if (gameOver) { resetBoard(); return; }   // كليك بعد نهاية = Restart

        if (my >= BOARD_HEIGHT) return;           // كليك في منطقة HUD

        int row = my / CELL_SIZE;
        int col = mx / CELL_SIZE;
        if (row >= 3 || col >= 3) return;
        if (board[row][col] != Player::None) return;

        board[row][col] = currentPlayer;

        if (checkWin()) {
            winner   = currentPlayer;
            gameOver = true;
            if (winner == Player::X) scoreX++;
            else                     scoreO++;
            return;
        }
        if (checkDraw()) {
            isDraw   = true;
            gameOver = true;
            scoreDraw++;
            return;
        }

        currentPlayer = (currentPlayer == Player::X) ? Player::O : Player::X;
    }

    // ─── Logic ───────────────────────────────────────────────────
    // checkWin تُحدّث winLine كمان
    bool checkWin() {
        // صفوف
        for (int r = 0; r < 3; r++)
            if (board[r][0] != Player::None &&
                board[r][0] == board[r][1] && board[r][1] == board[r][2]) {
                setWinLine(r,0, r,1, r,2); return true;
            }
        // أعمدة
        for (int c = 0; c < 3; c++)
            if (board[0][c] != Player::None &&
                board[0][c] == board[1][c] && board[1][c] == board[2][c]) {
                setWinLine(0,c, 1,c, 2,c); return true;
            }
        // قطر رئيسي
        if (board[0][0] != Player::None &&
            board[0][0] == board[1][1] && board[1][1] == board[2][2]) {
            setWinLine(0,0, 1,1, 2,2); return true;
        }
        // قطر عكسي
        if (board[0][2] != Player::None &&
            board[0][2] == board[1][1] && board[1][1] == board[2][0]) {
            setWinLine(0,2, 1,1, 2,0); return true;
        }
        return false;
    }

    void setWinLine(int r0,int c0, int r1,int c1, int r2,int c2) {
        winLine[0][0]=r0; winLine[0][1]=c0;
        winLine[1][0]=r1; winLine[1][1]=c1;
        winLine[2][0]=r2; winLine[2][1]=c2;
        hasWinLine = true;
    }

    bool checkDraw() {
        for (int r = 0; r < 3; r++)
            for (int c = 0; c < 3; c++)
                if (board[r][c] == Player::None) return false;
        return true;
    }

    // ─── Draw ────────────────────────────────────────────────────
    void drawGrid() {
        for (int i = 1; i < 3; i++) {
            // خطوط أفقية
            RectangleShape h(Vector2f((float)WINDOW_WIDTH, LINE_THICK));
            h.setFillColor(Color(60,60,60));
            h.setPosition(0.f, (float)(CELL_SIZE * i) - LINE_THICK / 2.f);
            window.draw(h);

            // خطوط رأسية
            RectangleShape v(Vector2f(LINE_THICK, (float)BOARD_HEIGHT));
            v.setFillColor(Color(60,60,60));
            v.setPosition((float)(CELL_SIZE * i) - LINE_THICK / 2.f, 0.f);
            window.draw(v);
        }
    }

    void drawSymbols() {
        for (int r = 0; r < 3; r++)
            for (int c = 0; c < 3; c++) {
                if (board[r][c] == Player::X) {
                    Text t("X", font, TEXT_SIZE);
                    t.setFillColor(Color(220, 50, 50));
                    t.setPosition((float)(c*CELL_SIZE)+TEXT_OFFSET_X,
                                  (float)(r*CELL_SIZE)+TEXT_OFFSET_Y);
                    window.draw(t);
                }
                if (board[r][c] == Player::O) {
                    Text t("O", font, TEXT_SIZE);
                    t.setFillColor(Color(50, 100, 220));
                    t.setPosition((float)(c*CELL_SIZE)+TEXT_OFFSET_X,
                                  (float)(r*CELL_SIZE)+TEXT_OFFSET_Y);
                    window.draw(t);
                }
            }
    }

    // تمييز خلايا الفوز بخلفية ذهبية
    void drawWinHighlight() {
        if (!hasWinLine) return;
        for (int i = 0; i < 3; i++) {
            int r = winLine[i][0], c = winLine[i][1];
            RectangleShape cell(Vector2f((float)CELL_SIZE, (float)CELL_SIZE));
            cell.setFillColor(Color(255, 220, 50, 120));   // أصفر شفاف
            cell.setPosition((float)(c * CELL_SIZE), (float)(r * CELL_SIZE));
            window.draw(cell);
        }
    }

    // ─── HUD ─────────────────────────────────────────────────────
    void drawHUD() {
        if (!fontOk) return;

        // خلفية HUD
        RectangleShape bg(Vector2f((float)WINDOW_WIDTH, 80.f));
        bg.setFillColor(Color(30, 30, 30));
        bg.setPosition(0.f, (float)BOARD_HEIGHT);
        window.draw(bg);

        // نقاط
        ostringstream ss;
        ss << "X:" << scoreX << "  Draw:" << scoreDraw << "  O:" << scoreO;
        Text scores(ss.str(), font, 16);
        scores.setFillColor(Color(200, 200, 200));
        scores.setPosition(10.f, (float)BOARD_HEIGHT + 5.f);
        window.draw(scores);

        // حالة اللعبة
        string status;
        Color  statusColor;
        if (!gameOver) {
            status      = (currentPlayer == Player::X) ? "X's Turn" : "O's Turn";
            statusColor = (currentPlayer == Player::X) ? Color(220,80,80) : Color(80,120,220);
        } else if (isDraw) {
            status      = "Draw!  Click to Restart";
            statusColor = Color::Yellow;
        } else {
            string w    = (winner == Player::X) ? "X" : "O";
            status      = w + " Wins!  Click to Restart";
            statusColor = (winner == Player::X) ? Color(220,80,80) : Color(80,120,220);
        }

        Text st(status, font, 20);
        st.setFillColor(statusColor);
        st.setPosition(10.f, (float)BOARD_HEIGHT + 32.f);
        window.draw(st);

        // تلميح
        Text hint("[R] Restart  [Esc] Quit", font, 13);
        hint.setFillColor(Color(100,100,100));
        hint.setPosition(10.f, (float)BOARD_HEIGHT + 60.f);
        window.draw(hint);
    }

    // ─── Render ──────────────────────────────────────────────────
    void render() {
        window.clear(Color(245, 245, 240));
        drawWinHighlight();   // أول عشان يكون تحت الـ grid
        drawGrid();
        drawSymbols();
        drawHUD();
        window.display();
    }

public:
    TicTacToe() : scoreX(0), scoreO(0), scoreDraw(0) {
        initWindow();

        // Font: fallback لمسارات متعددة
        fontOk = font.loadFromFile(FONT_FILE)                    ||
                 font.loadFromFile("C:/Windows/Fonts/arial.ttf") ||
                 font.loadFromFile("assets/arial.ttf")           ||
                 font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");

        resetBoard();
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
