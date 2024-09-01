#define SN_HELP   _T("Game controls:\nLeft key\nRight key\nUp key\nDown key")
#define SN_TIMER  100
#define SN_CLASS  _T("SNAKE")
#define SN_APP    _T("Snake")
#define SN_WIDTH  1200
#define SN_HEIGHT 900
#define SN_SIZE   30
#define SN_NONE   0
#define SN_BODY   1
#define SN_FOOD   2
#define SN_HEAD   3
#define SN_WALL   4
#define COLS      40
#define ROWS      30

#define ID_GAME_HISTORY 1
#define ID_FILE_OPEN 2
#define ID_EDIT_COPY 3
#define ID_EDIT_TEXT 4
#define ID_MENU 5


int  create_window(HINSTANCE hinst, LPCTSTR cap);
void draw(HDC mdc, int dir, HBRUSH back, HPEN pen);
void draw_text(HDC mdc);
void draw_score(HDC hdc);
void draw_pause(HWND hwnd);
void show_menu(HWND hwnd);
void start(int& dir);
void pos_food(void);
void ReadBestScore();
void WriteBestScore();
void WriteGameHistory(int score, int best_score);
void generateMap2();
void generateMap3();


BYTE g_mat[ROWS][COLS];
bool g_over;
bool g_paused = false;
int g_score;
int g_bestScore;
int snakeMap;
COLORREF snakeColor;


struct point {
    int x, y;
    point(void) {}
    point(int _x, int _y) :x(_x), y(_y) {}
};
std::vector<point> g_snake;

void start(int& dir) {
    dir = VK_UP;
    g_snake.clear();
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j)
            g_mat[i][j] = SN_NONE;
    }

    for (int b = 0; b < 3; ++b) {
        g_snake.push_back(point(COLS / 2, ROWS / 2 + b));
        const point& p = g_snake.back();
        g_mat[p.y][p.x] = SN_BODY;
    }
    ReadBestScore();
    pos_food();
    g_over = false;

    switch (snakeMap) {
    case 0:
        break;
    case 1:
        generateMap2();
        break;
    case 2:
        generateMap3();
        break;
    }
    g_over = false;
}

void draw(HDC mdc, int dir, HBRUSH back, HPEN pen, HBITMAP food, HBRUSH wall) {
    if (g_over) {
        draw_text(mdc);
        return;
    }

    char szBuffer[60];
    TextOut(mdc, 10, 30, szBuffer, wsprintf(szBuffer, "Score: %d", g_score));
    TextOut(mdc, 10, 60, szBuffer, wsprintf(szBuffer, "Best Score: %d", g_bestScore));

    point prev = g_snake.front();
    switch (dir) {
    case VK_LEFT:
        --g_snake.front().x;
        break;
    case VK_RIGHT:
        ++g_snake.front().x;
        break;
    case VK_UP:
        --g_snake.front().y;
        break;
    case VK_DOWN:
        ++g_snake.front().y;
        break;
    }

    if (g_snake.front().x < 0) g_snake.front().x = COLS - 1;
    if (g_snake.front().x >= COLS) g_snake.front().x = 0;
    if (g_snake.front().y < 0) g_snake.front().y = ROWS - 1;
    if (g_snake.front().y >= ROWS) g_snake.front().y = 0;

    point q = g_snake.front();
    if ((g_mat[q.y][q.x] == SN_BODY) || (g_mat[q.y][q.x] == SN_WALL)) {
        if (g_score > g_bestScore) {
            WriteGameHistory(g_score, g_bestScore);
            g_bestScore = g_score;
            WriteBestScore();
        }
        else {
            WriteGameHistory(g_score, g_bestScore);
        }
        g_score = 0;
        g_over = true;
        return;
    }

    point e = g_snake.back();
    std::vector<point>::size_type s;
    for (s = 1; s < g_snake.size(); ++s)
        std::swap(g_snake[s], prev);

    if (g_mat[q.y][q.x] == SN_FOOD) {
        g_score++;
        g_snake.push_back(e);
        g_mat[q.y][q.x] = SN_NONE;
        pos_food();
    }
    g_mat[e.y][e.x] = SN_NONE;
    g_mat[g_snake.front().y][g_snake.front().x] = SN_HEAD;

    for (s = 1; s < g_snake.size(); ++s)
        g_mat[g_snake[s].y][g_snake[s].x] = SN_BODY;

    int     x, y, m;
    RECT    rc;
    HGDIOBJ a, b;
    a = SelectObject(mdc, back);
    b = SelectObject(mdc, pen);
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            switch (g_mat[i][j]) {
            case SN_HEAD: 
                x = j * SN_SIZE;
                y = i * SN_SIZE;
                m = SetROP2(mdc, R2_XORPEN);
                Ellipse(mdc, x, y, x + SN_SIZE, y + SN_SIZE);
                SetROP2(mdc, m);
                break;
            case SN_BODY:
                x = j * SN_SIZE;
                y = i * SN_SIZE;
                RoundRect(mdc, x, y, x + SN_SIZE, y + SN_SIZE, 14, 14);
                break;
            case SN_FOOD: {
                x = j * SN_SIZE;
                y = i * SN_SIZE;
                HDC hdcMem = CreateCompatibleDC(mdc);
                HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, food);

                BITMAP bitmap;
                GetObject(food, sizeof(BITMAP), &bitmap);
                StretchBlt(mdc, x, y, SN_SIZE * 1.1, SN_SIZE * 1.1, hdcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);
                SelectObject(hdcMem, hOldBitmap);
                DeleteDC(hdcMem);
            }
                break;
            case SN_WALL: 
                x = j * SN_SIZE;
                y = i * SN_SIZE;
                SetRect(&rc, x, y, x + SN_SIZE, y + SN_SIZE);
                FillRect(mdc, &rc, wall);
                break;
            }
        }
    }
    SelectObject(mdc, a);
    SelectObject(mdc, b);
}

void generateMap2() {
    for (int i = 0; i < 10; ++i) {
        g_mat[8][i] = SN_WALL;
    }
    for (int i = 5; i < 25; ++i) {
        g_mat[i][5] = SN_WALL;
    }
    for (int i = 10; i < 15; ++i) {
        g_mat[i][15] = SN_WALL;
    }
    for (int i = 20; i < 30; ++i) {
        g_mat[20][i] = SN_WALL;
    }
    for (int i = 10; i < 25; ++i) {
        g_mat[i][30] = SN_WALL;
    }
}

void generateMap3() {
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            if ((i % 5 == 0) && (j % 5 == 0)) {
                g_mat[i][j] = SN_WALL;
            }
        }
    }
}

void pos_food(void) {
    int x, y;
    do {
        x = rand() % COLS;
        y = rand() % ROWS;
        if (g_mat[y][x] == SN_NONE) {
            g_mat[y][x] = SN_FOOD;
            break;
        }
    } while (1);
}

void ReadBestScore() {
    std::ifstream file("best_score.txt");
    if (file.is_open()) {
        file >> g_bestScore;
        file.close();
    }
    else {
        g_bestScore = 0;
    }
}

void WriteBestScore() {
    std::ofstream file("best_score.txt");
    if (file.is_open()) {
        file << g_bestScore;
        file.close();
    }
}

void WriteGameHistory(int score, int best_score) {
    std::ofstream file("game_history.txt", std::ios::app); 
    if (file.is_open()) {
        time_t now = time(0);
        tm ltm;
        localtime_s(&ltm, &now);

        file << "Date: " << 1900 + ltm.tm_year << "-"
            << 1 + ltm.tm_mon << "-"
            << ltm.tm_mday << " "
            << 1 + ltm.tm_hour << ":"
            << 1 + ltm.tm_min << ":"
            << 1 + ltm.tm_sec << "\n";

        file << "Score: " << score << "\n";
        if (score > best_score) {
            file << "New Best Score!\n";
        }
        file << "-------------------\n";
        file.close();
    }
}


void draw_text(HDC mdc) {
    const TCHAR s1[] = _T("END OF GAME");
    const int   n1 = sizeof(s1) / sizeof(s1[0]) - 1;
    SIZE sz;
    GetTextExtentPoint32(mdc, s1, n1, &sz);
    TextOut(mdc, (SN_WIDTH - sz.cx) / 2, (SN_HEIGHT - sz.cy) / 2 - sz.cy * 2, s1, n1);

    const TCHAR s2[] = _T("START OVER ENTER KEY");
    const int   n2 = sizeof(s2) / sizeof(s2[0]) - 1;
    GetTextExtentPoint32(mdc, s2, n2, &sz);
    TextOut(mdc, (SN_WIDTH - sz.cx) / 2, (SN_HEIGHT - sz.cy) / 2 + sz.cy, s2, n2);
}

void draw_score(HDC hdc) {
    char szBuffer[60];
    TextOut(hdc, 10, 30, szBuffer, wsprintf(szBuffer, "Score: %d", g_score));
    TextOut(hdc, 10, 60, szBuffer, wsprintf(szBuffer, "Best Score: %d", g_bestScore));
}

void draw_pause(HWND hwnd) {
    //hPauseButtons[0] = CreateWindow("BUTTON", "OK", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
    //    100, 50, 80, 30, hwnd, (HMENU)ID_DIALOG_OK,
    //    GetModuleHandle(NULL), NULL);

    //hPauseButtons[1] = CreateWindow("BUTTON", "OdasdasdsadK", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
    //    200, 150, 80, 30, hwnd, (HMENU)ID_DIALOG_CANCEL,
    //    GetModuleHandle(NULL), NULL);

    //hPauseButtons[2] = CreateWindow("BUTTON", "OK", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
    //    100, 150, 80, 30, hwnd, (HMENU)ID_DIALOG_3,
    //    GetModuleHandle(NULL), NULL);
}

void show_menu(HWND hwnd) {
    HMENU hMenubar = CreateMenu();
    HMENU hMenu = CreateMenu();

    AppendMenu(hMenu, MF_STRING, ID_MENU, "Menu");

    AppendMenu(hMenubar, MF_POPUP, (UINT_PTR)hMenu, "File");

    SetMenu(hwnd, hMenubar);
}
