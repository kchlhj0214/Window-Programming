#include <windows.h>
#include <tchar.h>
#include <random>
#include <math.h>
#include <algorithm>
#include <atlimage.h>
#include <string>
#pragma comment (lib, "msimg32.lib")

#define LEN 1000
#define HEI 1000
#define CAT_SIZE 128

using namespace std;

// 랜덤 장치
random_device rd;
mt19937 g(rd());
uniform_int_distribution<> disX{ 100, 3000 }; // 가로로 긴 월드 맵 범위 내 배회용
uniform_int_distribution<> disY{ 550, 680 };  // 땅(바닥) 높이 근처 배회용

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"My Window Class";
LPCTSTR lpszWindowName = L"windows program 2";

enum class PlayerState { IDLE, RUN, JUMP, SLIDE };

struct Player {
    float x, y;
    float velocityY;
    PlayerState state;

    RECT GetHitBox() {
        RECT rc;
        if (state == PlayerState::SLIDE) {
            rc.left = (int)x - 45;
            rc.top = (int)y - 40;
            rc.right = (int)x + 45;
            rc.bottom = (int)y;
        }
        else if (state == PlayerState::JUMP) {
            rc.left = (int)x - 25;
            rc.top = (int)y - 80;
            rc.right = (int)x + 25;
            rc.bottom = (int)y;
        }
        else {
            rc.left = (int)x - 25;
            rc.top = (int)y - 95;
            rc.right = (int)x + 25;
            rc.bottom = (int)y;
        }
        return rc;
    }
};

// 글로벌 변수
Player g_player;

float g_cameraX = 0.0f;
float g_moveSpeed = 8.0f;

const float DEADZONE_LEFT = LEN * 0.2f;
const float DEADZONE_RIGHT = LEN * 0.8f;

#define GROUND_Y 750 

// 에셋 변수
CImage img_back, img_far, img_middle, img_tileset;

// TMJ 규격 데이터
const int mapWidth = 16;
const int mapHeight = 8;
const int tileSize = 32; // 화면에 그려질 타일 크기 (32x32)
const int mapPixelWidth = mapWidth * tileSize; // 16 * 32 = 512px

int tileData[128] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11
};

// 알파 채널 투명 처리를 지원하는 그리기 헬퍼 함수
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

void InitGame() {
    img_back.Load(TEXT("back.png"));
    img_far.Load(TEXT("far.png"));
    img_middle.Load(TEXT("middle.png"));
    img_tileset.Load(TEXT("tileset.png"));

    g_player.x = LEN / 2;
    g_player.y = GROUND_Y + 80;
    g_player.velocityY = 0.0f;
    g_player.state = PlayerState::RUN;
}

void UpdateGame() {
    if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
        if (g_player.state == PlayerState::RUN) g_player.state = PlayerState::SLIDE;
    }
    else {
        if (g_player.state == PlayerState::SLIDE) g_player.state = PlayerState::RUN;
    }

    if (g_player.state != PlayerState::SLIDE) {
        if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
            g_player.x -= g_moveSpeed;
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
            if (g_player.x > DEADZONE_RIGHT) {
                g_cameraX += g_moveSpeed;
                g_player.x = DEADZONE_RIGHT;
            }
        }
    }

    if ((GetAsyncKeyState(VK_UP) & 0x8000) && g_player.state == PlayerState::RUN) {
        g_player.state = PlayerState::JUMP;
        g_player.velocityY = -18.0f;
    }

    if (g_player.state == PlayerState::JUMP) {
        g_player.y += g_player.velocityY;
        g_player.velocityY += 1.0f;
        if (g_player.y >= GROUND_Y + 80) {
            g_player.y = GROUND_Y + 80;
            g_player.velocityY = 0.0f;
            g_player.state = PlayerState::RUN;
        }
    }
}

void RenderFrame(HDC hDC, RECT& rt) {
    HDC memDC = CreateCompatibleDC(hDC);
    HBITMAP hBit = CreateCompatibleBitmap(hDC, rt.right, rt.bottom);
    HBITMAP oldBit = (HBITMAP)SelectObject(memDC, hBit);

    // 1. 기본 배경 숲 테마 어두운 보라색 도색
    HBRUSH hSky = CreateSolidBrush(RGB(42, 34, 54));
    FillRect(memDC, &rt, hSky);
    DeleteObject(hSky);

    // ★ 배경 원본 세로(240px)를 전체 화면 높이(1000px)에 가득 채우기 위한 세로 증폭 스케일 비율 자동 연산
    // 1000 / 240 = 약 4.16배 확대
    float scaleY = (float)rt.bottom / 240.0f;

    // 2. back.png 렌더링 (Parallax: 0.2)
    if (!img_back.IsNull()) {
        int imgW = img_back.GetWidth();
        int imgH = img_back.GetHeight();

        // 가로 길이는 원본 비율을 보존하면서 스케일링
        int drawW = (int)(imgW * scaleY);
        int drawH = rt.bottom;

        int scrollX = (int)(g_cameraX * 0.2f) % drawW;
        if (scrollX < 0) scrollX += drawW;

        for (int x = -scrollX; x < rt.right + drawW; x += drawW) {
            DrawAlphaImage(memDC, x, 0, drawW, drawH, img_back, 0, 0, imgW, imgH);
        }
    }

    // 3. far.png 렌더링 (Parallax: 0.5)
    if (!img_far.IsNull()) {
        int imgW = img_far.GetWidth();
        int imgH = img_far.GetHeight();

        int drawW = (int)(imgW * scaleY);
        int drawH = rt.bottom;

        int scrollX = (int)(g_cameraX * 0.5f) % drawW;
        if (scrollX < 0) scrollX += drawW;

        for (int x = -scrollX; x < rt.right + drawW; x += drawW) {
            DrawAlphaImage(memDC, x, 0, drawW, drawH, img_far, 0, 0, imgW, imgH);
        }
    }

    // 4. middle.png 렌더링 (Parallax: 1.0)
    if (!img_middle.IsNull()) {
        int imgW = img_middle.GetWidth();
        int imgH = img_middle.GetHeight();

        int drawW = (int)(imgW * scaleY);
        int drawH = rt.bottom;

        int scrollX = (int)(g_cameraX * 1.0f) % drawW;
        if (scrollX < 0) scrollX += drawW;

        for (int x = -scrollX; x < rt.right + drawW; x += drawW) {
            DrawAlphaImage(memDC, x, 0, drawW, drawH, img_middle, 0, 0, imgW, imgH);
        }
    }

    // 5. tileset.png 연동 타일맵 무한 루프 그리기 (배경 비율에 맞춘 스케일 보정)
        // ★ [수정] 배경 확대 비율(scaleY)을 타일 크기에도 똑같이 적용합니다.
    int scaledTileSize = (int)(tileSize * scaleY);
    int scaledMapPixelWidth = mapWidth * scaledTileSize;

    // 스케일이 적용된 맵 너비를 기준으로 스크롤 연산
    int tileScrollX = (int)g_cameraX % scaledMapPixelWidth;
    if (tileScrollX < 0) tileScrollX += scaledMapPixelWidth;

    // 타일셋 이미지 검사 및 가로 타일 개수 계산
    int tilesetWidth = 0;
    int tilesetHeight = 0;
    int tilesetColCount = 8;

    if (!img_tileset.IsNull()) {
        tilesetWidth = img_tileset.GetWidth();
        tilesetHeight = img_tileset.GetHeight();
        tilesetColCount = tilesetWidth / 32; // 원본 이미지에서 자를 때는 원래 크기(32) 사용
        if (tilesetColCount <= 0) tilesetColCount = 8;
    }

    // 변경된 타일 크기(scaledTileSize) 기준으로 바닥까지 필요한 줄 수 계산
    int requiredRows = 6 + ((rt.bottom - GROUND_Y) / scaledTileSize) + 1;

    // 화면 가로를 채우기 위한 무한 루프 반복 (-2부터 5까지)
    for (int loop = -2; loop <= 5; ++loop) {
        int baseOffsetX = loop * scaledMapPixelWidth - tileScrollX;

        for (int y = 0; y < requiredRows; ++y) {
            for (int x = 0; x < mapWidth; ++x) {

                int sampleY = min(y, mapHeight - 1);
                int tileIdx = tileData[sampleY * mapWidth + x];

                // 빈 공간(0)이 아닐 때만 렌더링
                if (tileIdx > 0) {
                    // ★ [수정] 좌표 계산 시 변형된 타일 크기(scaledTileSize)를 적용합니다.
                    int drawX = baseOffsetX + (x * scaledTileSize);
                    int drawY = GROUND_Y + ((y - 6) * scaledTileSize);

                    if (drawY >= rt.bottom) continue;

                    // Tiled의 GID(1부터 시작)를 0부터 시작하는 인덱스로 변환
                    int localIdx = tileIdx - 1;

                    // 원본 이미지(tileset.png)에서 뜯어낼 때는 원래 크기인 32를 기준으로 계산
                    int srcX = (localIdx % tilesetColCount) * 32;
                    int srcY = (localIdx / tilesetColCount) * 32;

                    bool bImageDrawn = false;

                    if (!img_tileset.IsNull() &&
                        srcX >= 0 && srcX + 32 <= tilesetWidth &&
                        srcY >= 0 && srcY + 32 <= tilesetHeight)
                    {
                        // ★ [수정] DrawAlphaImage의 목적지 크기(Width, Height)에 scaledTileSize를 넣어 확대 출력합니다.
                        DrawAlphaImage(memDC, drawX, drawY, scaledTileSize, scaledTileSize, img_tileset, srcX, srcY, 32, 32);
                        bImageDrawn = true;
                    }

                    // [디버그 안전장치] 이미지를 그리지 못했을 때도 확대된 크기로 상자 출력
                    if (!bImageDrawn) {
                        HBRUSH tBrush = CreateSolidBrush((tileIdx == 8) ? RGB(139, 69, 19) : RGB(80, 40, 10));
                        RECT tRect = { drawX, drawY, drawX + scaledTileSize, drawY + scaledTileSize };
                        FillRect(memDC, &tRect, tBrush);
                        DeleteObject(tBrush);
                    }
                }
            }
        }
    }

    // 데드존 가이드 라인
    HPEN hGuidePen = CreatePen(PS_DOT, 1, RGB(200, 200, 200));
    HPEN oldPen = (HPEN)SelectObject(memDC, hGuidePen);
    MoveToEx(memDC, (int)DEADZONE_LEFT, 0, NULL); LineTo(memDC, (int)DEADZONE_LEFT, rt.bottom);
    MoveToEx(memDC, (int)DEADZONE_RIGHT, 0, NULL); LineTo(memDC, (int)DEADZONE_RIGHT, rt.bottom);

    // 7. 플레이어 및 히트박스 렌더링
    RECT hitBox = g_player.GetHitBox();
    COLORREF charColor = RGB(0, 0, 255);
    if (g_player.state == PlayerState::JUMP) charColor = RGB(255, 165, 0);
    if (g_player.state == PlayerState::SLIDE) charColor = RGB(0, 255, 0);

    HBRUSH pBrush = CreateSolidBrush(charColor);
    RECT rcDraw;
    if (g_player.state == PlayerState::SLIDE) {
        rcDraw = { (int)g_player.x - 45, (int)g_player.y - 45, (int)g_player.x + 45, (int)g_player.y };
    }
    else {
        rcDraw = { (int)g_player.x - 25, (int)g_player.y - 100, (int)g_player.x + 25, (int)g_player.y };
    }
    FillRect(memDC, &rcDraw, pBrush);
    DeleteObject(pBrush);

    HPEN hRedPen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
    SelectObject(memDC, hRedPen);
    HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, GetStockObject(NULL_BRUSH));
    Rectangle(memDC, hitBox.left, hitBox.top, hitBox.right, hitBox.bottom);

    SelectObject(memDC, oldPen);
    SelectObject(memDC, oldBrush);
    DeleteObject(hRedPen);
    DeleteObject(hGuidePen);

    BitBlt(hDC, 0, 0, rt.right, rt.bottom, memDC, 0, 0, SRCCOPY);
    SelectObject(memDC, oldBit);
    DeleteObject(hBit);
    DeleteDC(memDC);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
    HWND hWnd;
    MSG msg; 
    WNDCLASSEX WndClass;
    g_hInst = hInstance;
    WndClass.cbSize = sizeof(WndClass); 
    WndClass.style = CS_HREDRAW | CS_VREDRAW;
    WndClass.lpfnWndProc = (WNDPROC)WndProc;
    WndClass.cbClsExtra = 0; WndClass.cbWndExtra = 0;
    WndClass.hInstance = hInstance; 
    WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    WndClass.lpszMenuName = NULL; 
    WndClass.lpszClassName = lpszClass; 
    WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
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
    case WM_CREATE:
        InitGame();
        break;
    case WM_SIZE:
        GetClientRect(hWnd, &rt);
        return 0;
    case WM_GETMINMAXINFO:
    {
        LPMINMAXINFO lpmmi = (LPMINMAXINFO)lParam;

        lpmmi->ptMinTrackSize.x = LEN;
        lpmmi->ptMinTrackSize.y = HEI;
    }
    return 0;
    case WM_LBUTTONDOWN:
    {
        int mx = LOWORD(lParam);
        int my = HIWORD(lParam);
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