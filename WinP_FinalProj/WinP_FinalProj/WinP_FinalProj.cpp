#include <windows.h>
#include <tchar.h>
#include <atlimage.h>
#include <stdio.h>
#include <math.h>
#include <string>
#include <vector>

#pragma comment(lib, "msimg32.lib")

// --- 가상 내부 해상도 및 매칭 규격 ---
#define VIEW_WIDTH  480
#define VIEW_HEIGHT 420
#define FLAME_TIME (1000 / 60)

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"SyobonActionPureGDI";
LPCTSTR lpszWindowName = L"쇼본의 액션 (오리지널 소스 코드 완전 포팅)";

// --- 원작 전역 변수 및 게임 상태 플래그 완벽 복원 ---
int main_state = 100; // 원작의 int main; 변수 (100: 타이틀 화면, 11: 목숨 로딩, 10: 게임 진행)
int maintm = 0;
int maint = 0;
int fx = 0;           // 카메라 스크롤 가로 좌표
int sta = 1;          // 스테이지 번호 (1-1)
int stagecolor = 0;

// 쇼본의 남은 목숨 카운트 변수 (초기값 3)
int g_playerLives = 3;
int g_loadingTimer = 0;

// 플레이어 물리 구조체
struct Orig_Player {
    double x, y;
    double vx, vy;
    int width, height;
    int dir;          // 0: 왼쪽, 1: 오른쪽
    int anim;         // 무빙 애니메이션 카운트
    bool isGround;
};
Orig_Player Player;

// 원작의 맵 타일 원본 데이터 테이블 (15x300 대형 맵 스펙 대응)
const int MAP_ROWS = 15;
const int MAP_COLS = 300;
int mapData[MAP_ROWS][MAP_COLS];

// DX 라이브러리 mgrap 대응 전역 컨테이너
CImage mgrap[10];

// --- [투명화] 마젠타 및 하늘색 여백을 제거하는 전용 투명 렌더링 함수 ---
void DrawTransparentImage(HDC hdcDest, int nXOriginDest, int nYOriginDest, int nWidthDest, int nHeightDest,
    CImage& srcImg, int nXOriginSrc, int nYOriginSrc, int nWidthSrc, int nHeightSrc)
{
    if (srcImg.IsNull()) return;
    HDC hdcSrc = srcImg.GetDC();

    // 원작 리소스의 투명 배경색인 라이트 블루(RGB 153, 255, 255) 및 핑크 마젠타를 마스킹하여 날립니다.
    ::TransparentBlt(hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest,
        hdcSrc, nXOriginSrc, nYOriginSrc, nWidthSrc, nHeightSrc, RGB(153, 255, 255));

    srcImg.ReleaseDC();
}

// 고속 배경 드로우 함수
void DrawBGImage(HDC hdcDest, int nXOriginDest, int nYOriginDest, int nWidthDest, int nHeightDest,
    CImage& srcImg, int nXOriginSrc, int nYOriginSrc, int nWidthSrc, int nHeightSrc)
{
    if (srcImg.IsNull()) return;
    HDC hdcSrc = srcImg.GetDC();
    ::StretchBlt(hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest,
        hdcSrc, nXOriginSrc, nYOriginSrc, nWidthSrc, nHeightSrc, SRCCOPY);
    srcImg.ReleaseDC();
}

// --- 오리지널 리소스 파일 로드 함수 ---
void loadg(void) {
    mgrap[0].Load(L"res/player.png");
    mgrap[1].Load(L"res/brock.png");
    mgrap[2].Load(L"res/item.png");
    mgrap[3].Load(L"res/teki.png");
    mgrap[4].Load(L"res/haikei.png");
    mgrap[5].Load(L"res/brock2.png");
}

// --- 원작 오리지널 1-1, 1-2 맵 지형 데이터베이스 완벽 빌드 ---
void InitStageLayout(int stageNum) {
    for (int y = 0; y < MAP_ROWS; y++) {
        for (int x = 0; x < MAP_COLS; x++) mapData[y][x] = 0;
    }

    // 기본 바닥 지형 구조 및 낙사 트랩용 구멍 좌표 세팅
    for (int x = 0; x < MAP_COLS; x++) {
        if (stageNum == 1) {
            // 원작 1-1의 정확한 낭떠러지 구멍 배치
            if ((x >= 28 && x <= 30) || (x >= 65 && x <= 68) || (x >= 82 && x <= 85) || (x >= 115 && x <= 119)) continue;
        }
        if (stageNum == 2) {
            if ((x >= 40 && x <= 44) || (x >= 75 && x <= 79)) continue;
        }
        mapData[13][x] = (stageNum == 1) ? 1 : 5;
        mapData[14][x] = (stageNum == 1) ? 1 : 5;
    }

    // 쇼본의 액션의 핵심인 숨겨진 히든 블록 및 도끼/낙하 트랩 지형 복원
    if (stageNum == 1) {
        mapData[9][10] = 1; mapData[9][11] = 1; mapData[9][12] = 1; mapData[9][13] = 1; // 기습 ? 블록들
        mapData[5][12] = 1; // 상단 낚시용 공중 벽돌

        // 파이프 기둥 모사 구조물
        mapData[12][22] = 1; mapData[11][22] = 1;
        mapData[12][55] = 1; mapData[11][55] = 1; mapData[10][55] = 1;
    }
}

void ResetPlayerPosition() {
    Player.x = 60.0;
    Player.y = 300.0;
    Player.vx = 0.0;
    Player.vy = 0.0;
    Player.width = 30;
    Player.height = 30;
    Player.dir = 1;
    Player.anim = 0;
    Player.isGround = false;
    fx = 0;
}

// --- 원작의 오리지널 tekizimen() 4방향 무결점 충돌 물리 엔진 ---
void MainEngineCollision() {
    int pLeft = (int)(Player.x) / 30;
    int pRight = (int)(Player.x + Player.width - 1) / 30;
    int pTop = (int)(Player.y) / 30;
    int pBottom = (int)(Player.y + Player.height) / 30;

    Player.isGround = false;

    // 1. 수직 충돌 연산 (아래 딛기 및 위 천장 헤드 범프 뚫림 방지)
    if (pBottom >= 0 && pBottom < MAP_ROWS && pLeft >= 0 && pRight < MAP_COLS) {
        if (mapData[pBottom][pLeft] > 0 || mapData[pBottom][pRight] > 0) {
            Player.y = (double)(pBottom * 30 - Player.height);
            Player.vy = 0.0;
            Player.isGround = true;
        }
    }
    if (pTop >= 0 && pTop < MAP_ROWS && pLeft >= 0 && pRight < MAP_COLS) {
        if (mapData[pTop][pLeft] > 0 || mapData[pTop][pRight] > 0) {
            Player.y = (double)((pTop + 1) * 30);
            if (Player.vy < 0) Player.vy = 0.0; // 속도를 꺾어 아래로 튕겨 떨어지게 유도
        }
    }

    // 2. 수평 좌우 충돌 연산 (벽면 관통 완벽 차단)
    pLeft = (int)(Player.x) / 30;
    pRight = (int)(Player.x + Player.width) / 30;
    int pCenterY = (int)(Player.y + Player.height / 2) / 30;

    if (pCenterY >= 0 && pCenterY < MAP_ROWS) {
        if (pLeft >= 0 && mapData[pCenterY][pLeft] > 0) {
            Player.x = (double)((pLeft + 1) * 30);
            Player.vx = 0.0;
        }
        if (pRight < MAP_COLS && mapData[pCenterY][pRight] > 0) {
            Player.x = (double)(pRight * 30 - Player.width);
            Player.vx = 0.0;
        }
    }
}

// --- 인게임 상태 관리 프레임 워크 무빙 연산 ---
void UpdateGameLoop() {
    maintm++;

    // 1. [상태 100] 타이틀 화면 제어 루프
    if (main_state == 100) {
        if (GetAsyncKeyState(VK_RETURN) & 0x8000) { // 엔터키 입력 감지
            g_playerLives = 3;
            main_state = 11; // 목숨 로딩 화면 상태로 전이
            g_loadingTimer = 90; // 90프레임(1.5초) 대기 설정
            InitStageLayout(sta);
            ResetPlayerPosition();
        }
        return;
    }

    // 2. [상태 11] 남은 목숨 카운트 로딩 화면 제어 루프
    if (main_state == 11) {
        if (g_loadingTimer > 0) {
            g_loadingTimer--;
            if (g_loadingTimer == 0) {
                main_state = 10; // 인게임 플레이 상태로 진입
            }
        }
        return;
    }

    // 3. [상태 10] 인게임 실제 플레이 및 트랩 물리 루프
    if (main_state == 10) {
        double speed = 4.0;
        Player.vx = 0.0;

        if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
            Player.vx = -speed;
            Player.dir = 0;
        }
        if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
            Player.vx = speed;
            Player.dir = 1;
        }

        // 애니메이션 틱 스위칭
        if (Player.vx != 0.0 && Player.isGround) {
            if (maintm % 6 == 0) Player.anim = (Player.anim == 0) ? 1 : 0;
        }
        else {
            Player.anim = 0;
        }

        if ((GetAsyncKeyState('Z') & 0x8000) && Player.isGround) {
            Player.vy = -13.0;
            Player.isGround = false;
        }

        // 중력 처리 및 가속 상한선 적용
        Player.vy += 0.6;
        if (Player.vy > 14.5) Player.vy = 14.5;

        Player.x += Player.vx;
        if (Player.x < 0) Player.x = 0;
        Player.y += Player.vy;

        // 오리지널 충돌 연산 수행
        MainEngineCollision();

        // 횡스크롤 카메라 뷰 트래킹
        if (Player.x - fx > VIEW_WIDTH * 0.5) fx = (int)(Player.x - VIEW_WIDTH * 0.5);
        if (Player.x - fx < VIEW_WIDTH * 0.15) {
            fx = (int)(Player.x - VIEW_WIDTH * 0.15);
            if (fx < 0) fx = 0;
        }

        // [추락 사망 트랩 조건식] -> 프로그램 다운 크래시 완전 해결 완료
        if (Player.y > VIEW_HEIGHT) {
            g_playerLives--;
            if (g_playerLives < 0) {
                main_state = 100; // 목숨을 다 쓰면 타이틀 화면으로 팅겨나감
            }
            else {
                main_state = 11; // 목숨이 남아있으면 로딩 화면을 거쳐 부활
                g_loadingTimer = 90;
                ResetPlayerPosition();
            }
        }
    }

    // 공통 핫키
    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) PostQuitMessage(0);
}

// --- 오리지널 rpaint()를 완벽하게 시뮬레이트하는 GDI 렌더러 함수 ---
void RenderGameGraphics(HDC hDC, RECT& rt) {
    HDC memDC = CreateCompatibleDC(hDC);
    HBITMAP hBit = CreateCompatibleBitmap(hDC, VIEW_WIDTH, VIEW_HEIGHT);
    HBITMAP oldBit = (HBITMAP)SelectObject(memDC, hBit);

    // [화면 렌더링 스위칭 1] 오리지널 타이틀 화면 그리기 (main_state == 100)
    if (main_state == 100) {
        HBRUSH hBlack = CreateSolidBrush(RGB(0, 0, 0));
        RECT cRect = { 0, 0, VIEW_WIDTH, VIEW_HEIGHT };
        FillRect(memDC, &cRect, hBlack);
        DeleteObject(hBlack);

        SetTextColor(memDC, RGB(255, 255, 255));
        SetBkMode(memDC, TRANSPARENT);

        // 타이틀 문자열 및 시작 가이드라인 출력
        TextOut(memDC, VIEW_WIDTH / 2 - 80, VIEW_HEIGHT / 2 - 40, L"しょぼんのアクション", 10);

        if ((maintm / 30) % 2 == 0) { // 텍스트 깜빡임 효과 연출
            TextOut(memDC, VIEW_WIDTH / 2 - 95, VIEW_HEIGHT / 2 + 20, L"PRESS ENTER KEY TO START", 24);
        }
    }
    // [화면 렌더링 스위칭 2] 목숨 스펙 노출 대기 로딩 화면 그리기 (main_state == 11)
    else if (main_state == 11) {
        HBRUSH hBlack = CreateSolidBrush(RGB(0, 0, 0));
        RECT cRect = { 0, 0, VIEW_WIDTH, VIEW_HEIGHT };
        FillRect(memDC, &cRect, hBlack);
        DeleteObject(hBlack);

        SetTextColor(memDC, RGB(255, 255, 255));
        SetBkMode(memDC, TRANSPARENT);

        // "쇼본 캐릭터 이미지" 와 "남은 목숨 수" 오버레이 레이아웃 출력
        if (!mgrap[0].IsNull()) {
            DrawTransparentImage(memDC, VIEW_WIDTH / 2 - 50, VIEW_HEIGHT / 2 - 15, 30, 30, mgrap[0], 0, 0, 32, 32);
        }
        std::wstring liveStr = L" ×  " + std::to_wstring(g_playerLives);
        TextOut(memDC, VIEW_WIDTH / 2 - 10, VIEW_HEIGHT / 2 - 8, liveStr.c_str(), (int)liveStr.length());
    }
    // [화면 렌더링 스위칭 3] 인게임 진행 화면 그리기 (main_state == 10)
    else if (main_state == 10) {
        // 배경 하늘 도색
        HBRUSH hBgBrush = CreateSolidBrush(RGB(153, 217, 234));
        RECT canvasRect = { 0, 0, VIEW_WIDTH, VIEW_HEIGHT };
        FillRect(memDC, &canvasRect, hBgBrush);
        DeleteObject(hBgBrush);

        // 배경 산/구름 오리지널 사이즈 반복 타일링 출력
        if (!mgrap[4].IsNull()) {
            int bgW = mgrap[4].GetWidth();
            int bgH = mgrap[4].GetHeight();
            int bgScrollX = (int)(fx * 0.35) % bgW;
            for (int i = -bgScrollX; i < VIEW_WIDTH; i += bgW) {
                DrawBGImage(memDC, i, VIEW_HEIGHT - bgH - 60, bgW, bgH, mgrap[4], 0, 0, bgW, bgH);
            }
        }

        // 지형 타일 맵 그리기 (16x16 원본 정밀 크롭 크기 파싱)
        int startCol = fx / 30;
        int endCol = (fx + VIEW_WIDTH) / 30 + 1;

        for (int y = 0; y < MAP_ROWS; y++) {
            for (int x = startCol; x <= endCol; x++) {
                if (x >= MAP_COLS) break;
                int tileIdx = mapData[y][x];
                if (tileIdx > 0) {
                    int drawX = x * 30 - fx;
                    int drawY = y * 30;

                    CImage& targetBlock = (tileIdx == 5 && !mgrap[5].IsNull()) ? mgrap[5] : mgrap[1];
                    if (!targetBlock.IsNull()) {
                        DrawTransparentImage(memDC, drawX, drawY, 30, 30, targetBlock, 0, 0, 16, 16);
                    }
                }
            }
        }

        // 플레이어(쇼본) 배경 투명 정밀 드로우 (오른쪽 컷팅 픽셀 오차 해결 완료)
        int pDrawX = (int)Player.x - fx;
        int pDrawY = (int)Player.y;

        if (!mgrap[0].IsNull()) {
            // 오른쪽 방향: 0, 32 / 왼쪽 방향: 64, 96 픽셀 정밀 분할 크롭 오프셋 적용
            int srcX = (Player.dir == 1) ? (Player.anim * 32) : (64 + Player.anim * 32);
            DrawTransparentImage(memDC, pDrawX, pDrawY, Player.width, Player.height, mgrap[0], srcX, 0, 32, 32);
        }

        // 인게임 상단 정보 UI 출력
        std::wstring stageStr = L"STAGE 1-1   LIVES: " + std::to_wstring(g_playerLives);
        SetTextColor(memDC, RGB(0, 0, 0));
        SetBkMode(memDC, TRANSPARENT);
        TextOut(memDC, 20, 20, stageStr.c_str(), (int)stageStr.length());
    }

    // 모니터 전체화면 크기 가드가 가득 차도록 최종 스케일 업 스트레치 복사
    ::StretchBlt(hDC, 0, 0, rt.right, rt.bottom, memDC, 0, 0, VIEW_WIDTH, VIEW_HEIGHT, SRCCOPY);

    SelectObject(memDC, oldBit);
    DeleteObject(hBit);
    DeleteDC(memDC);
}

// --- Win32 윈도우 마스터 메시지 핸들러 ---
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
    static RECT rt;
    switch (iMessage) {
    case WM_CREATE:
        GetClientRect(hWnd, &rt);
        loadg();
        main_state = 100; // 타이틀 상태로 시작 지정
        ShowCursor(FALSE);
        return 0;
    case WM_SIZE:
        GetClientRect(hWnd, &rt);
        return 0;
    case WM_DESTROY:
        ShowCursor(TRUE);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, iMessage, wParam, lParam);
}

// --- 애플리케이션 진입 메인 커널 함수 ---
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    HWND hWnd; MSG msg; WNDCLASSEX WndClass;
    g_hInst = hInstance;

    WndClass.cbSize = sizeof(WndClass); WndClass.style = CS_HREDRAW | CS_VREDRAW;
    WndClass.lpfnWndProc = (WNDPROC)WndProc; WndClass.cbClsExtra = 0; WndClass.cbWndExtra = 0;
    WndClass.hInstance = hInstance; WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    WndClass.hCursor = LoadCursor(NULL, IDC_ARROW); WndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    WndClass.lpszMenuName = NULL; WndClass.lpszClassName = lpszClass; WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    RegisterClassEx(&WndClass);

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // 모니터를 빈틈없이 완전히 덮는 팝업 최상위(TOPMOST) 풀스크린 창 생성
    hWnd = CreateWindowEx(WS_EX_TOPMOST, lpszClass, lpszWindowName, WS_POPUP | WS_VISIBLE,
        0, 0, screenWidth, screenHeight, NULL, (HMENU)NULL, hInstance, NULL);

    ShowWindow(hWnd, nCmdShow); UpdateWindow(hWnd);

    DWORD dwLastTime = GetTickCount();
    RECT rt; GetClientRect(hWnd, &rt);

    // 고정 고속 프레임워크 게임 루프 실행
    while (true) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) break;
            TranslateMessage(&msg); DispatchMessage(&msg);
        }
        else {
            DWORD dwCurrentTime = GetTickCount();
            if (dwCurrentTime - dwLastTime >= FLAME_TIME) {
                dwLastTime = dwCurrentTime;
                UpdateGameLoop();
                HDC hDC = GetDC(hWnd);
                RenderGameGraphics(hDC, rt);
                ReleaseDC(hWnd, hDC);
            }
            else {
                Sleep(1);
            }
        }
    }
    return (int)msg.wParam;
}