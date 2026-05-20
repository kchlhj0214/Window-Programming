#include <windows.h>
#include <tchar.h>
#include <random>
#include <math.h>
#include <algorithm>
#include <atlimage.h>
#include <string>
#include <vector>

#pragma comment (lib, "msimg32.lib")

#define LEN GetSystemMetrics(SM_CXSCREEN)
#define HEI GetSystemMetrics(SM_CYSCREEN)
#define ANINUM 40
#define MONSTER_SIZE 64
#define TILENUM 3

using namespace std;

// 52, 90 // 8프레임

// 랜덤 장치
random_device rd;
mt19937 g(rd());
uniform_int_distribution<> disX{ 20, LEN - 20 };
uniform_int_distribution<> speed{ 10, 100 };
uniform_int_distribution<> tileX{ LEN / 5, (LEN / 4)*3 };
uniform_int_distribution<> tileY{ HEI / 5, (HEI / 4) * 3 };
uniform_int_distribution<> tileW{ 100, 200 };
uniform_int_distribution<> tileH{ 100, 150 };

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"My Window Class";
LPCTSTR lpszWindowName = L"windows program 2";

struct SpriteInfo {
    int x, y, w, h;
};

struct AniInfo {
    int speed, x, y; int isCatched; SpriteInfo aSprite;
};

CImage img_ani, img_back, img_tile;
vector<RECT> tileRect;
vector<RECT> g_prevTileRect;

bool g_isMoving = false;
POINT g_lastMousePos = { 0, 0 };

int g_AniFrame = 0;
DWORD g_lastAniTime = 0;

bool isFalling = false;
int movingIdx = -1;

vector<AniInfo> g_anies;

void DrawAlphaImage(HDC hdcDest, int nXOriginDest, int nYOriginDest, int nWidthDest, int nHeightDest, CImage& srcImg, int nXOriginSrc, int nYOriginSrc, int nWidthSrc, int nHeightSrc)
{
    if (srcImg.IsNull()) return;

    BLENDFUNCTION bf;
    bf.BlendOp = AC_SRC_OVER;       // 배경에 덮어씌우기
    bf.BlendFlags = 0;
    bf.SourceConstantAlpha = 255;   // 불투명도
    bf.AlphaFormat = AC_SRC_ALPHA;  // 투명 배경 인식

    HDC hdcSrc = srcImg.GetDC();
    ::GdiAlphaBlend(hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest,
        hdcSrc, nXOriginSrc, nYOriginSrc, nWidthSrc, nHeightSrc, bf);       // 출력
    srcImg.ReleaseDC();
}

SpriteInfo GetAniSprite(int w, int h) {     // 프레임 마다 사용할 위치 저장(0부터 세기)
    int startY = h * 32;
    int startX = w * 32;
    return { startX, startY, 32, 32 };
}

void SpawnAni() {
    AniInfo a;
    g_anies.clear();
    for (int i = 0; i < ANINUM; ++i) {
        a.x = disX(g);
        a.y = 0;
        a.isCatched = -1;
        a.speed = speed(g);
        a.aSprite = GetAniSprite(0, 90);
        g_anies.push_back(a);
    }
}

void InitGame() {
    img_ani.Load(TEXT("Cat_Ginger.png"));
    img_back.Load(TEXT("back.png")); 
    img_tile.Load(TEXT("tile1.png"));

    isFalling = false;
    SpawnAni();

    tileRect.clear();
    for (int i = 0; i < TILENUM; ++i) {
        int x = tileX(g);
        int y = tileY(g);
        int w = tileW(g);
        int h = tileH(g);
        RECT a = { x, y, x + w, y + h };
        tileRect.push_back(a);
        g_prevTileRect.push_back(a);
    }
}

void UpdateGame() {
    DWORD currentTime = GetTickCount();

    static DWORD lastFrameTime = 0;
    if (currentTime - lastFrameTime > 100) {
        g_AniFrame++;
        lastFrameTime = currentTime;
    }

    for (int k = 0; k < TILENUM; ++k) {
        int deltaX = tileRect[k].left - g_prevTileRect[k].left;
        int deltaY = tileRect[k].top - g_prevTileRect[k].top;

        for (size_t i = 0; i < g_anies.size(); ++i) {
            static DWORD lastMoveTime[ANINUM] = { 0 };

            if(isFalling){
                if (g_anies[i].isCatched == -1) {
                    if (currentTime - lastMoveTime[i] > (DWORD)g_anies[i].speed) {
                        g_anies[i].y += 2;
                        lastMoveTime[i] = currentTime;
                    }

                    if (g_anies[i].y > HEI) {
                        g_anies[i].y = 0;
                        g_anies[i].x = disX(g);
                        g_anies[i].speed = speed(g);
                    }

                    if ((tileRect[k].right - tileRect[k].left != 0) && (tileRect[k].bottom - tileRect[k].top != 0)) {
                        //POINT pt = { g_anies[i].x, g_anies[i].y };
                        RECT hitbox = { g_anies[i].x - 28, g_anies[i].y - 28, g_anies[i].x + 28, g_anies[i].y + 28 };
                        RECT temp;
                        RECT temptile = { tileRect[k].left, tileRect[k].top, tileRect[k].right, tileRect[k].top + 4 };
                        if (IntersectRect(&temp, &hitbox, &temptile)) {
                            g_anies[i].isCatched = k;
                        }
                        /*if (((g_anies[i].x - (MONSTER_SIZE / 2) <= tileRect[k].right && g_anies[i].x - (MONSTER_SIZE / 2) >= tileRect[k].left) ||
                            (g_anies[i].x + (MONSTER_SIZE / 2) <= tileRect[k].right && g_anies[i].x + (MONSTER_SIZE / 2) >= tileRect[k].left)) &&
                            (g_anies[i].y + (MONSTER_SIZE / 2) <= tileRect[k].top + 2 && g_anies[i].y + (MONSTER_SIZE / 2) >= tileRect[k].top - 2)) {
                            g_anies[i].isCatched = k;
                        }*/
                        else {
                            g_anies[i].isCatched = -1;
                        }
                    }
                }
                else {
                    if(g_anies[i].isCatched != -1 && g_anies[i].isCatched == movingIdx){
                        g_anies[i].y += deltaY;

                        if ((g_anies[i].y + (MONSTER_SIZE / 2) > tileRect[k].top) && (k == g_anies[i].isCatched)) {
                            g_anies[i].y = tileRect[k].top - (MONSTER_SIZE / 2);
                        }
                        else {
                            if (currentTime - lastMoveTime[i] > (DWORD)g_anies[i].speed) {
                                g_anies[i].y += 2;
                                lastMoveTime[i] = currentTime;
                            }
                        }
                    }
                }
            }
        }

        g_prevTileRect[k] = tileRect[k];
    }
}

void RenderFrame(HDC hDC, RECT& rt) {
    HDC memDC = CreateCompatibleDC(hDC);
    HBITMAP hBit = CreateCompatibleBitmap(hDC, rt.right, rt.bottom);
    HBITMAP oldBit = (HBITMAP)SelectObject(memDC, hBit);

    if (!img_back.IsNull()) {
        DrawAlphaImage(memDC, 0, 0, rt.right, rt.bottom, img_back, 0, 0, img_back.GetWidth(), img_back.GetHeight());
    }
    else {
        HBRUSH hBlank = CreateSolidBrush(RGB(42, 34, 54));
        FillRect(memDC, &rt, hBlank);
        DeleteObject(hBlank);
    }

    for (int i = 0; i < TILENUM; ++i) {
        if (!img_tile.IsNull()) {
            DrawAlphaImage(memDC, tileRect[i].left, tileRect[i].top, tileRect[i].right - tileRect[i].left, tileRect[i].bottom - tileRect[i].top, img_tile, 0, 0, 16, 16);
        }
    }

    if (!img_ani.IsNull()) {
        for (size_t i = 0; i < g_anies.size(); ++i) {
            int frameIdxW = 0;
            int frameIdxH = 0;

            if (g_anies[i].isCatched == -1) {
                frameIdxH = 90;
                frameIdxW = 0 + (g_AniFrame % 8);
            }
            else {
                frameIdxH = 52;
                frameIdxW = 0 + (g_AniFrame % 8);
            }

            SpriteInfo sprite = GetAniSprite(frameIdxW, frameIdxH);
            int drawX = g_anies[i].x - (MONSTER_SIZE / 2);
            int drawY = g_anies[i].y - (MONSTER_SIZE / 2);

            DrawAlphaImage(memDC, drawX, drawY, MONSTER_SIZE, MONSTER_SIZE, img_ani, sprite.x, sprite.y, sprite.w, sprite.h);
        }
    }

    for (int i = 0; i < TILENUM; ++i) {
        if (tileRect[i].right - tileRect[i].left != 0) {
            HPEN hDragPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
            HPEN oldPen = (HPEN)SelectObject(memDC, hDragPen);
            HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, GetStockObject(NULL_BRUSH));

            Rectangle(memDC, tileRect[i].left, tileRect[i].top, tileRect[i].right, tileRect[i].bottom);

            SelectObject(memDC, oldPen);
            SelectObject(memDC, oldBrush);
            DeleteObject(hDragPen);
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
    POINT mousePos;
    mousePos.x = LOWORD(lParam);
    mousePos.y = HIWORD(lParam);

    switch (uMsg) {
    case WM_CREATE:
        InitGame();
        break;

    case WM_SIZE:
        GetClientRect(hWnd, &rt);
        return 0;

    case WM_LBUTTONDOWN:
        for (int i = 0; i < TILENUM; ++i) {
            if ((tileRect[i].right - tileRect[i].left > 0) && PtInRect(&tileRect[i], mousePos)) {
                g_isMoving = true;
                g_lastMousePos = mousePos;
                movingIdx = i;
            }
        }
        break;

    case WM_MOUSEMOVE:
        if (g_isMoving) {
            int moveX = mousePos.x - g_lastMousePos.x;
            int moveY = mousePos.y - g_lastMousePos.y;

            int rectWidth = tileRect[movingIdx].right - tileRect[movingIdx].left;
            int rectHeight = tileRect[movingIdx].bottom - tileRect[movingIdx].top;

            tileRect[movingIdx].left += moveX;
            tileRect[movingIdx].right += moveX;

            if (tileRect[movingIdx].left < 0) {
                tileRect[movingIdx].left = 0;
                tileRect[movingIdx].right = rectWidth;
            }
            if (tileRect[movingIdx].right > LEN) {
                tileRect[movingIdx].right = LEN;
                tileRect[movingIdx].left = LEN - rectWidth;
            }

            tileRect[movingIdx].top += moveY;
            tileRect[movingIdx].bottom += moveY;

            if (tileRect[movingIdx].top < 0) {
                tileRect[movingIdx].top = 0;
                tileRect[movingIdx].bottom = rectHeight;
            }
            if (tileRect[movingIdx].bottom > HEI) {
                tileRect[movingIdx].bottom = HEI;
                tileRect[movingIdx].top = HEI - rectHeight;
            }

            g_lastMousePos = mousePos;

            for (int i = 0; i < ANINUM; ++i) {
                RECT hitbox = { g_anies[i].x - 28, g_anies[i].y - 28, g_anies[i].x + 28, g_anies[i].y + 28 };
                RECT temp;
                RECT temptile = { tileRect[g_anies[i].isCatched].left, tileRect[g_anies[i].isCatched].top, tileRect[g_anies[i].isCatched].right, tileRect[g_anies[i].isCatched].top + 4 };
                if (g_anies[i].isCatched != -1) {
                    if (!IntersectRect(&temp, &hitbox, &temptile)) {
                        g_anies[i].isCatched = -1;
                    }
                }
            }
        }
        break;

    case WM_LBUTTONUP:
        if (g_isMoving) {
            g_isMoving = false;
        }
        break;

    case WM_RBUTTONDOWN:
        for (int i = 0; i < TILENUM; ++i) {
            if ((tileRect[i].right - tileRect[i].left > 0) && PtInRect(&tileRect[i], mousePos)) {
                tileRect[i].left = 0;
                tileRect[i].right = 0;
                tileRect[i].top = 0;
                tileRect[i].bottom = 0;
            }
        }
        break;

    case WM_KEYDOWN:
        if (wParam == 'D') {
            tileRect.clear();
            for (int i = 0; i < TILENUM; ++i) {
                int x = tileX(g);
                int y = tileY(g);
                int w = tileW(g);
                int h = tileH(g);
                RECT a = { x, y, x + w, y + h };
                tileRect.push_back(a);
            }
        }
        else if (wParam == 'P') {
            isFalling = true;
        }
        else if (wParam == 'Q') {
            PostQuitMessage(0);
        } 
        break;

    case WM_ERASEBKGND:
        return 1;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}