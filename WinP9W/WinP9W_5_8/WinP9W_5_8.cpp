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

using namespace std;

// 랜덤 장치
random_device rd;
mt19937 g(rd());
uniform_int_distribution<> disX{ 20, LEN - 20 };
uniform_int_distribution<> speed{ 10, 100 };

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"My Window Class";
LPCTSTR lpszWindowName = L"windows program 2";

struct SpriteInfo {
    int x, y, w, h;
};

struct AniInfo {
    int speed, x, y; bool isCatched; SpriteInfo aSprite;
};

CImage img_ani, img_back;
RECT dragRect = { 0, 0, 0, 0 };
RECT g_prevDragRect = { 0, 0, 0, 0 };

bool g_isDrawing = false;
bool g_isMoving = false;
bool g_isInverted = false;
POINT g_lastMousePos = { 0, 0 };

int g_AniFrame = 0;
DWORD g_lastAniTime = 0;

bool g_rBouttonPressed = false;
bool isFalling = false;

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

SpriteInfo GetAniSprite(int num) {     // 프레임 마다 사용할 위치 저장(0부터 세기)
    int startX = num * 32;
    return { startX, 0, 32, 32 };
}

void SpawnAni() {
    AniInfo a;
    for (int i = 0; i < ANINUM; ++i) {
        a.x = disX(g);
        a.y = 0;
        a.isCatched = false;
        a.speed = speed(g);
        a.aSprite = GetAniSprite(0);
        g_anies.push_back(a);
    }
}

void InitGame() {
    img_ani.Load(TEXT("Enemies.png"));
    img_back.Load(TEXT("back.png"));

    isFalling = false;
    SpawnAni();
}

void UpdateGame() {
    DWORD currentTime = GetTickCount();

    static DWORD lastFrameTime = 0;
    if (currentTime - lastFrameTime > 100) {
        g_AniFrame++;
        lastFrameTime = currentTime;
    }

    int deltaX = dragRect.left - g_prevDragRect.left;
    int deltaY = dragRect.top - g_prevDragRect.top;

    for (size_t i = 0; i < g_anies.size(); ++i) {
        static DWORD lastMoveTime[ANINUM] = { 0 };

        if (!g_anies[i].isCatched) {
            if (currentTime - lastMoveTime[i] > (DWORD)g_anies[i].speed) {
                g_anies[i].y += 2;
                lastMoveTime[i] = currentTime;
            }

            if (g_anies[i].y > HEI) {
                g_anies[i].y = 0;
                g_anies[i].x = disX(g);
                g_anies[i].speed = speed(g);
            }

            if ((dragRect.right - dragRect.left != 0) && (dragRect.bottom - dragRect.top != 0)) {
                POINT pt = { g_anies[i].x, g_anies[i].y };
                if (PtInRect(&dragRect, pt)) {
                    g_anies[i].isCatched = true;
                }
            }
        }
        else {
            g_anies[i].x += deltaX;
            g_anies[i].y += deltaY;

            if (g_anies[i].y + (MONSTER_SIZE / 2) > dragRect.bottom) {
                g_anies[i].y = dragRect.bottom - (MONSTER_SIZE / 2);
            }
            else {
                if (currentTime - lastMoveTime[i] > (DWORD)g_anies[i].speed) {
                    g_anies[i].y += 2;
                    lastMoveTime[i] = currentTime;
                }
            }
        }
    }

    g_prevDragRect = dragRect;
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

    if (!img_ani.IsNull()) {
        for (size_t i = 0; i < g_anies.size(); ++i) {
            int frameIdx = 0;

            if (g_anies[i].isCatched) {
                frameIdx = 4 + (g_AniFrame % 4); // 포획: 4 ~ 7 프레임
            }
            else {
                frameIdx = 0 + (g_AniFrame % 4); // 일반: 0 ~ 3 프레임
            }

            SpriteInfo sprite = GetAniSprite(frameIdx);
            int drawX = g_anies[i].x - (MONSTER_SIZE / 2);
            int drawY = g_anies[i].y - (MONSTER_SIZE / 2);

            DrawAlphaImage(memDC, drawX, drawY, MONSTER_SIZE, MONSTER_SIZE, img_ani, sprite.x, sprite.y, sprite.w, sprite.h);
        }
    }

    if (dragRect.right - dragRect.left != 0) {
        HPEN hDragPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
        HPEN oldPen = (HPEN)SelectObject(memDC, hDragPen);
        HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, GetStockObject(NULL_BRUSH));

        Rectangle(memDC, dragRect.left, dragRect.top, dragRect.right, dragRect.bottom);

        SelectObject(memDC, oldPen);
        SelectObject(memDC, oldBrush);
        DeleteObject(hDragPen);
    }

    if (g_isInverted && (dragRect.right - dragRect.left > 0)) {
        BitBlt(memDC, dragRect.left, dragRect.top,
            dragRect.right - dragRect.left, dragRect.bottom - dragRect.top,
            memDC, dragRect.left, dragRect.top, DSTINVERT);
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
        if ((dragRect.right - dragRect.left > 0) && PtInRect(&dragRect, mousePos)) {
            g_isMoving = true;
            g_lastMousePos = mousePos;
        }
        else {
            g_isDrawing = true;
            g_isInverted = false;
            for (auto& ani : g_anies) ani.isCatched = false;

            dragRect.left = mousePos.x;
            dragRect.top = mousePos.y;
            dragRect.right = mousePos.x;
            dragRect.bottom = mousePos.y;
        }
        break;

    case WM_MOUSEMOVE:
        if (g_isDrawing) {
            dragRect.right = min(max((int)mousePos.x, 0), LEN);
            dragRect.bottom = min(max((int)mousePos.y, 0), HEI);
        }
        else if (g_isMoving) {
            int moveX = mousePos.x - g_lastMousePos.x;
            int moveY = mousePos.y - g_lastMousePos.y;

            int rectWidth = dragRect.right - dragRect.left;
            int rectHeight = dragRect.bottom - dragRect.top;

            dragRect.left += moveX;
            dragRect.right += moveX;

            if (dragRect.left < 0) {
                dragRect.left = 0;
                dragRect.right = rectWidth;
            }
            if (dragRect.right > LEN) {
                dragRect.right = LEN;
                dragRect.left = LEN - rectWidth;
            }

            dragRect.top += moveY;
            dragRect.bottom += moveY;

            if (dragRect.top < 0) {
                dragRect.top = 0;
                dragRect.bottom = rectHeight;
            }
            if (dragRect.bottom > HEI) {
                dragRect.bottom = HEI;
                dragRect.top = HEI - rectHeight;
            }

            g_lastMousePos = mousePos;
        }
        break;

    case WM_LBUTTONUP:
        if (g_isDrawing) {
            g_isDrawing = false;

            RECT normalized;
            normalized.left = min(dragRect.left, dragRect.right);
            normalized.right = max(dragRect.left, dragRect.right);
            normalized.top = min(dragRect.top, dragRect.bottom);
            normalized.bottom = max(dragRect.top, dragRect.bottom);
            dragRect = normalized;
        }
        if (g_isMoving) {
            g_isMoving = false;
        }
        break;

    case WM_RBUTTONDOWN:
        if ((dragRect.right - dragRect.left > 0) && PtInRect(&dragRect, mousePos)) {
            g_isInverted = !g_isInverted;
        }
        break;

    case WM_KEYDOWN:
        if (wParam == 'D') {
            dragRect = { 0, 0, 0, 0 };
            g_prevDragRect = { 0, 0, 0, 0 };
            g_isInverted = false;
            for (auto& ani : g_anies) ani.isCatched = false;
        }
        else if (wParam == 'R') {
            dragRect = { 0, 0, 0, 0 };
            g_prevDragRect = { 0, 0, 0, 0 };
            g_isInverted = false;
            SpawnAni();
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