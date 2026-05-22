#include <windows.h>
#include <tchar.h>
#include <random>
#include <math.h>
#include <algorithm>
#include <atlimage.h>
#include <string>
#include <vector>

#pragma comment (lib, "msimg32.lib")

#define LEN 1000
#define HEI 1000

using namespace std;

// 랜덤 장치
random_device rd;
mt19937 g(rd());
uniform_int_distribution<> disX{ 100, 900 };
uniform_int_distribution<> disY{ 100, 900 };
uniform_int_distribution<> color{ 1, 5 };   // 빨 주 노 초 파 / (색상 숫자가 곧 체력이자 데미지)
uniform_int_distribution<> brick_size{ 0, 2 };  // 정사각형, 가로로 긴 직사각형, 세로로 긴 직사각형
uniform_int_distribution<> move_disX{ -50, 50 };
uniform_int_distribution<> move_disY{ -50, 50 };

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"My Window Class";
LPCTSTR lpszWindowName = L"windows program 2";

enum class Dir { UP, DOWN, LEFT, RIGHT };

struct SpriteInfo {
    int x, y, w, h;
};

struct Effect {
    float x, y;
    int animFrame;
    DWORD lastAnimTime;  // 프레임 전환 시간 체크용
};

// 글로벌 이미지 에셋 변수
CImage img_back, img_effect, img_brick;
CImage img_bullet_red, img_bullet_orange, img_bullet_yellow, img_bullet_green, img_bullet_blue;
// 8프레임
CImage img_pacman_red, img_pacman_orange, img_pacman_yellow, img_pacman_green, img_pacman_blue;
// 8프레임
CImage img_ghost_red, img_ghost_orange, img_ghost_yellow, img_ghost_green, img_ghost_blue;

int g_playerAnimFrame = 0;
DWORD g_lastPlayerAnimTime = 0;

bool g_showDebug = true;
bool g_hKeyPressed = false;

struct Pacman {
    float x, y;
    float j_x, j_y;
    float velocity;
    float size;
    bool jump;
    Dir dir;
    int color;

    RECT GetHitBox() const {
        RECT rc;
        rc.left = (int)x - 50;
        rc.right = (int)x + 50;
        rc.bottom = (int)y + 50;
        rc.top = (int)y - 50;

        return rc;
    }
};

struct Ghost {
    float x, y;     // 현 좌표
    float mid_x, mid_y; // 고정 중심 좌표
    Dir dir;
    int animFrame;
    DWORD lastAnimTime;
    bool isDead;
    float speed;
    int color;

    RECT GetHitBox() const {
        RECT rc;
        rc.left = (int)x - 25;
        rc.right = (int)x + 25;
        rc.bottom = (int)y + 25;
        rc.top = (int)y - 25;

        return rc;
    }
};

struct Bullet {
    float x, y;
    Dir dir;
    float speed;
    int color;

    RECT GetHitBox() const {        // 추후 테스트 하면서 사이즈 수정
        RECT rc;
        rc.left = (int)x - 25;
        rc.right = (int)x + 25;
        rc.bottom = (int)y + 25;
        rc.top = (int)y - 25;

        return rc;
    }
};

struct Brick {
    float x, y;
    int hp;
    int color;
    int size;

    RECT GetHitBox() const {
        RECT rc;
        if(size == 0) {
            rc.left = (int)x - 25;
            rc.right = (int)x + 25;
            rc.bottom = (int)y + 25;
            rc.top = (int)y - 25;
        }
        else if (size == 1) {
            rc.left = (int)x - 50;
            rc.right = (int)x + 50;
            rc.bottom = (int)y + 25;
            rc.top = (int)y - 25;
        }
        else if (size == 2) {
            rc.left = (int)x - 25;
            rc.right = (int)x + 25;
            rc.bottom = (int)y + 50;
            rc.top = (int)y - 50;
        }

        return rc;
    }
};

Pacman g_pacman;
vector<Ghost> g_enemies;
vector<Bullet> g_bullets;
vector<Brick> g_bricks;
vector<Effect> g_effects;

DWORD g_lastShootTime = 0;
const DWORD SHOOT_COOLDOWN = 500;

void DrawAlphaImage(HDC hdcDest, int nXOriginDest, int nYOriginDest, int nWidthDest, int nHeightDest, CImage& srcImg, int nXOriginSrc, int nYOriginSrc, int nWidthSrc, int nHeightSrc)
{
    if (srcImg.IsNull()) return;

    BLENDFUNCTION bf;
    bf.BlendOp = AC_SRC_OVER;
    bf.BlendFlags = 0;
    bf.SourceConstantAlpha = 255;
    bf.AlphaFormat = AC_SRC_ALPHA;

    HDC hdcSrc = srcImg.GetDC();
    ::GdiAlphaBlend(hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest,
        hdcSrc, nXOriginSrc, nYOriginSrc, nWidthSrc, nHeightSrc, bf);
    srcImg.ReleaseDC();
}

SpriteInfo GetPacmanSprite(int num) {
    int startX = (num - 1) * 16;
    return { startX, 0, 16, 16 };
}

SpriteInfo GetGhostSprite(int num) {
    int startX = (num - 1) * 16;
    return { startX, 0, 16, 16 };
}

SpriteInfo GetEffectSprite(int frame) {
    int startX = frame * 64;
    return { startX, 576, 64, 64 };
}

SpriteInfo GetBrickSprite(int size, int state, int color) {
    int x, y;

    if (size == 0) {
        x = 224 + state * 16;
        y = 96 - color * 16;
        return { x, y - 16, 16, 16 };
    }
    else {
        x = 112 + state * 32;
        y = 96 - color * 16;
        return { x, y - 16, 32, 16 };
    }
}

void SpawnGhosts() {
    
}

void SpawnBricks() {

}

void InitGame() {
    //img_back.Load(TEXT("back.png"));
    
    img_bullet_red.Load(TEXT("red_razer.png"));
    img_bullet_orange.Load(TEXT("orange_razer.png"));
    img_bullet_yellow.Load(TEXT("yellow_razer.png"));
    img_bullet_green.Load(TEXT("green_razer.png"));
    img_bullet_blue.Load(TEXT("blue_razer.png"));

    img_pacman_red.Load(TEXT("PacMan_red.png"));
    img_pacman_orange.Load(TEXT("PacMan_orange.png"));
    img_pacman_yellow.Load(TEXT("PacMan_yellow.png"));
    img_pacman_green.Load(TEXT("PacMan_green.png"));
    img_pacman_blue.Load(TEXT("PacMan_blue.png"));

    img_ghost_red.Load(TEXT("redGhost.png"));
    img_ghost_orange.Load(TEXT("orangeGhost.png"));
    img_ghost_yellow.Load(TEXT("yellowGhost.png"));
    img_ghost_green.Load(TEXT("greenGhost.png"));
    img_ghost_blue.Load(TEXT("blueGhost.png"));

    img_brick.Load(TEXT("bricks.png"));
    img_effect.Load(TEXT("Free_Smoke_Fx_Pixel 05.png"));

    g_player.x = LEN / 2;
    g_player.y = GROUND_Y + 80;
    g_player.velocityY = 0.0f;
    g_player.state = PlayerState::IDLE;

    g_lastShootTime = GetTickCount();
}

void UpdateGame() {
    DWORD currentTime = GetTickCount();

    if (GetAsyncKeyState('H') & 0x8000) {
        if (!g_hKeyPressed) {
            g_showDebug = !g_showDebug;
            g_hKeyPressed = true;
        }
    }
    else {
        g_hKeyPressed = false;
    }

    if (currentTime - g_lastAutoSpawnTime >= 5000) {
        EnemyType randomType = (disX(g) % 2 == 0) ? EnemyType::WALK : EnemyType::FLY;
        SpawnEnemy(randomType);
        g_lastAutoSpawnTime = currentTime;
    }

    if ((currentTime - g_lastAutoSpawnTime >= SPAWN_COOLDOWN) &&
        (currentTime - g_lastManualSpawnTime >= SPAWN_COOLDOWN)) {

        if (GetAsyncKeyState('1') & 0x8000) {
            SpawnEnemy(EnemyType::WALK);
            g_lastManualSpawnTime = currentTime;
        }
        else if (GetAsyncKeyState('2') & 0x8000) {
            SpawnEnemy(EnemyType::FLY);
            g_lastManualSpawnTime = currentTime;
        }
    }

    if (GetAsyncKeyState(VK_DOWN) & 0x8000 && g_player.state != PlayerState::JUMP) {
        g_player.state = PlayerState::SLIDE;
    }
    else {
        if (g_player.state == PlayerState::SLIDE) {
            g_player.state = PlayerState::IDLE;
        }
    }

    if (g_player.state != PlayerState::SLIDE) {
        bool isMoving = false;
        if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
            g_player.x -= g_moveSpeed;
            isMoving = true;
            if (g_player.x < DEADZONE_LEFT) {
                if (g_cameraX > 0) {
                    g_cameraX -= g_moveSpeed;
                    g_player.x = DEADZONE_LEFT;
                    if (g_cameraX < 0) g_cameraX = 0;
                }
                else if (g_player.x < 32.0f) {
                    g_player.x = 32.0f;
                }
            }
        }
        if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
            g_player.x += g_moveSpeed;
            isMoving = true;
            if (g_player.x > DEADZONE_RIGHT) {
                g_cameraX += g_moveSpeed;
                g_player.x = DEADZONE_RIGHT;
            }
        }

        if (g_player.state != PlayerState::JUMP) {
            g_player.state = isMoving ? PlayerState::RUN : PlayerState::IDLE;
        }
    }

    if ((GetAsyncKeyState(VK_UP) & 0x8000) && g_player.state != PlayerState::JUMP) {
        g_player.state = PlayerState::JUMP;
        g_player.velocityY = -18.0f;
    }

    if (g_player.state == PlayerState::JUMP) {
        g_player.y += g_player.velocityY;
        g_player.velocityY += 1.0f;
        if (g_player.y >= GROUND_Y + 80) {
            g_player.y = GROUND_Y + 80;
            g_player.velocityY = 0.0f;
            g_player.state = PlayerState::IDLE;
        }
    }

    if (currentTime - g_lastPlayerAnimTime > 100) {
        g_playerAnimFrame++;
        g_lastPlayerAnimTime = currentTime;
    }

    // 몬스터 상태 업데이트 및 충돌 검사
    RECT pRealHitBox = g_player.GetHitBox();
    pRealHitBox.left += (int)g_cameraX;
    pRealHitBox.right += (int)g_cameraX;

    for (int i = 0; i < g_enemies.size(); ) {
        g_enemies[i].x -= g_enemies[i].speed;

        if (currentTime - g_enemies[i].lastAnimTime > 150) {
            g_enemies[i].animFrame++;
            g_enemies[i].lastAnimTime = currentTime;
        }

        RECT eHitBox = g_enemies[i].GetHitBox();
        RECT dummy;
        if (IntersectRect(&dummy, &pRealHitBox, &eHitBox)) {
            Effect eff;
            eff.x = g_enemies[i].x;
            eff.y = g_enemies[i].y;
            eff.animFrame = 0;
            eff.lastAnimTime = currentTime;
            g_effects.push_back(eff);

            g_enemies.erase(g_enemies.begin() + i);
            continue;
        }

        if (g_enemies[i].x < -100) {
            g_enemies.erase(g_enemies.begin() + i);
        }
        else {
            i++;
        }
    }

    for (int i = 0; i < g_effects.size(); ) {
        if (currentTime - g_effects[i].lastAnimTime > 70) {
            g_effects[i].animFrame++;
            g_effects[i].lastAnimTime = currentTime;
        }

        if (g_effects[i].animFrame >= 9) {
            g_effects.erase(g_effects.begin() + i);
        }
        else {
            i++;
        }
    }
}

void RenderFrame(HDC hDC, RECT& rt) {
    HDC memDC = CreateCompatibleDC(hDC);
    HBITMAP hBit = CreateCompatibleBitmap(hDC, rt.right, rt.bottom);
    HBITMAP oldBit = (HBITMAP)SelectObject(memDC, hBit);

    HBRUSH hSky = CreateSolidBrush(RGB(42, 34, 54));
    FillRect(memDC, &rt, hSky);
    DeleteObject(hSky);

    float scaleY = (float)rt.bottom / 240.0f;

    if (!img_back.IsNull()) {
        int imgW = img_back.GetWidth(); int imgH = img_back.GetHeight();
        int drawW = (int)(imgW * scaleY); int drawH = rt.bottom;
        int scrollX = (int)(g_cameraX * 0.2f) % drawW;
        if (scrollX < 0) scrollX += drawW;
        for (int x = -scrollX; x < rt.right + drawW; x += drawW) {
            DrawAlphaImage(memDC, x, 0, drawW, drawH, img_back, 0, 0, imgW, imgH);
        }
    }
    if (!img_far.IsNull()) {
        int imgW = img_far.GetWidth(); int imgH = img_far.GetHeight();
        int drawW = (int)(imgW * scaleY); int drawH = rt.bottom;
        int scrollX = (int)(g_cameraX * 0.5f) % drawW;
        if (scrollX < 0) scrollX += drawW;
        for (int x = -scrollX; x < rt.right + drawW; x += drawW) {
            DrawAlphaImage(memDC, x, 0, drawW, drawH, img_far, 0, 0, imgW, imgH);
        }
    }
    if (!img_middle.IsNull()) {
        int imgW = img_middle.GetWidth(); int imgH = img_middle.GetHeight();
        int drawW = (int)(imgW * scaleY); int drawH = rt.bottom;
        int scrollX = (int)(g_cameraX * 1.0f) % drawW;
        if (scrollX < 0) scrollX += drawW;
        for (int x = -scrollX; x < rt.right + drawW; x += drawW) {
            DrawAlphaImage(memDC, x, 0, drawW, drawH, img_middle, 0, 0, imgW, imgH);
        }
    }

    int scaledTileSize = (int)(tileSize * scaleY);
    int scaledMapPixelWidth = mapWidth * scaledTileSize;
    int tileScrollX = (int)g_cameraX % scaledMapPixelWidth;
    if (tileScrollX < 0) tileScrollX += scaledMapPixelWidth;

    int tilesetWidth = 0, tilesetHeight = 0, tilesetColCount = 8;
    if (!img_tileset.IsNull()) {
        tilesetWidth = img_tileset.GetWidth();
        tilesetHeight = img_tileset.GetHeight();
        tilesetColCount = tilesetWidth / 32;
        if (tilesetColCount <= 0) tilesetColCount = 8;
    }

    int requiredRows = 6 + ((rt.bottom - GROUND_Y) / scaledTileSize) + 1;

    for (int loop = -2; loop <= 5; ++loop) {
        int baseOffsetX = loop * scaledMapPixelWidth - tileScrollX;
        for (int y = 0; y < requiredRows; ++y) {
            for (int x = 0; x < mapWidth; ++x) {
                int sampleY = min(y, mapHeight - 1);
                int tileIdx = tileData[sampleY * mapWidth + x];
                if (tileIdx > 0) {
                    int drawX = baseOffsetX + (x * scaledTileSize);
                    int drawY = GROUND_Y + ((y - 6) * scaledTileSize);
                    if (drawY >= rt.bottom) continue;

                    int localIdx = tileIdx - 1;
                    int srcX = (localIdx % tilesetColCount) * 32;
                    int srcY = (localIdx / tilesetColCount) * 32;

                    if (!img_tileset.IsNull() && srcX >= 0 && srcX + 32 <= tilesetWidth && srcY >= 0 && srcY + 32 <= tilesetHeight) {
                        DrawAlphaImage(memDC, drawX, drawY, scaledTileSize, scaledTileSize, img_tileset, srcX, srcY, 32, 32);
                    }
                }
            }
        }
    }

    if (g_showDebug) {
        HPEN hGuidePen = CreatePen(PS_DOT, 1, RGB(200, 200, 200));
        HPEN oldPen = (HPEN)SelectObject(memDC, hGuidePen);
        MoveToEx(memDC, (int)DEADZONE_LEFT, 0, NULL); LineTo(memDC, (int)DEADZONE_LEFT, rt.bottom);
        MoveToEx(memDC, (int)DEADZONE_RIGHT, 0, NULL); LineTo(memDC, (int)DEADZONE_RIGHT, rt.bottom);
        SelectObject(memDC, oldPen);
        DeleteObject(hGuidePen);
    }

    // 플레이어 애니메이션
    SpriteInfo pSprite;
    switch (g_player.state) {
    case PlayerState::IDLE:  pSprite = GetMarioSprite(9);  break;
    case PlayerState::RUN:   pSprite = GetMarioSprite(10 + (g_playerAnimFrame % 3)); break;
    case PlayerState::JUMP:  pSprite = GetMarioSprite(14); break;
    case PlayerState::SLIDE: pSprite = GetMarioSprite(15); break;
    }

    int pDrawW = 70;
    int pDrawH = 70;
    int pDrawX = (int)g_player.x - pDrawW / 2;
    int pDrawY = (g_player.state == PlayerState::SLIDE) ? (int)g_player.y - pDrawH : (int)g_player.y - pDrawH;

    if (!img_mario.IsNull()) {
        DrawAlphaImage(memDC, pDrawX, pDrawY, pDrawW, pDrawH, img_mario, pSprite.x, pSprite.y, pSprite.w, pSprite.h);
    }

    if (g_showDebug) {
        RECT hitBox = g_player.GetHitBox();
        HPEN hRedPen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
        HPEN oldPen = (HPEN)SelectObject(memDC, hRedPen);
        HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, GetStockObject(NULL_BRUSH));

        Rectangle(memDC, hitBox.left, hitBox.top, hitBox.right, hitBox.bottom);

        SelectObject(memDC, oldPen);
        SelectObject(memDC, oldBrush);
        DeleteObject(hRedPen);
    }

    // 몬스터 렌더링
    for (const auto& enemy : g_enemies) {
        SpriteInfo eSprite;
        int eDrawW = 150;
        int eDrawH = 150;

        if (enemy.type == EnemyType::WALK) {
            int walkFrames[] = { 20, 21 };
            eSprite = GetEnemySprite(walkFrames[enemy.animFrame % 2]);
        }
        else {
            int flyFrames[] = { 15, 16, 17 };
            eSprite = GetEnemySprite(flyFrames[enemy.animFrame % 3]);
        }

        int eScreenX = (int)(enemy.x - g_cameraX);
        int eDrawX = eScreenX - eDrawW / 2;
        int eDrawY = (int)enemy.y - eDrawH;

        if (!img_enemies.IsNull()) {
            DrawAlphaImage(memDC, eDrawX, eDrawY, eDrawW, eDrawH, img_enemies, eSprite.x, eSprite.y, eSprite.w, eSprite.h);
        }

        if (g_showDebug) {
            HPEN hGreenPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
            HPEN oldPen = (HPEN)SelectObject(memDC, hGreenPen);
            HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, GetStockObject(NULL_BRUSH));

            RECT eHitBox = enemy.GetHitBox();
            int ebLeft = eHitBox.left - (int)g_cameraX;
            int ebRight = eHitBox.right - (int)g_cameraX;
            Rectangle(memDC, ebLeft, eHitBox.top, ebRight, eHitBox.bottom);

            SelectObject(memDC, oldPen);
            SelectObject(memDC, oldBrush);
            DeleteObject(hGreenPen);
        }
    }

    // 폭발 이펙트 렌더링
    for (const auto& eff : g_effects) {
        SpriteInfo effSprite = GetEffectSprite(eff.animFrame);

        int effDrawW = 300;
        int effDrawH = 300;
        int effScreenX = (int)(eff.x - g_cameraX);
        int effDrawX = effScreenX - effDrawW / 2;
        int effDrawY = (int)eff.y - effDrawH + 100;

        if (!img_effect.IsNull()) {
            DrawAlphaImage(memDC, effDrawX, effDrawY, effDrawW, effDrawH, img_effect,
                effSprite.x, effSprite.y, effSprite.w, effSprite.h);
        }
    }

    BitBlt(hDC, 0, 0, rt.right, rt.bottom, memDC, 0, 0, SRCCOPY);
    SelectObject(memDC, oldBit);
    DeleteObject(hBit);
    DeleteDC(memDC);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
    HWND hWnd; MSG msg; WNDCLASSEX WndClass;
    g_hInst = hInstance;
    WndClass.cbSize = sizeof(WndClass);
    WndClass.style = CS_HREDRAW | CS_VREDRAW;
    WndClass.lpfnWndProc = (WNDPROC)WndProc;
    WndClass.cbClsExtra = 0; WndClass.cbWndExtra = 0;
    WndClass.hInstance = hInstance;
    WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    WndClass.lpszMenuName = NULL; WndClass.lpszClassName = lpszClass; WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    RegisterClassEx(&WndClass);

    hWnd = CreateWindow(lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW, 0, 0, LEN, HEI, NULL, (HMENU)NULL, hInstance, NULL);
    ShowWindow(hWnd, nCmdShow); UpdateWindow(hWnd);

    RECT rt; GetClientRect(hWnd, &rt);
    while (true) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) break;
            TranslateMessage(&msg); DispatchMessage(&msg);
        }
        else {
            UpdateGame();
            HDC hDC = GetDC(hWnd);
            RenderFrame(hDC, rt);
            ReleaseDC(hWnd, hDC);
            Sleep(10);
        }
    }
    return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static RECT rt;
    switch (uMsg) {
    case WM_CREATE: InitGame(); break;
    case WM_SIZE: GetClientRect(hWnd, &rt); return 0;
    case WM_GETMINMAXINFO:
    {
        LPMINMAXINFO lpmmi = (LPMINMAXINFO)lParam;
        lpmmi->ptMinTrackSize.x = LEN; lpmmi->ptMinTrackSize.y = HEI;
    }
    return 0;
    case WM_ERASEBKGND: return 1;
    case WM_DESTROY: PostQuitMessage(0); break;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}