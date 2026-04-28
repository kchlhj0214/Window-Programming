#include <windows.h>
#include <tchar.h>
#include <random>
#include <vector>
#include <math.h>
#include <string>

using namespace std;

// --- 랜덤 엔진 및 설정 ---
random_device rd;
mt19937 g(rd());
uniform_int_distribution<> uid_x(0, 1100);
uniform_int_distribution<> uid_y(0, 800);

#define LEN 1200
#define HEI 900
#define BOARD_SIZE 15
#define CELL_SIZE 40
#define START_X 60
#define START_Y 60
#define MAX_TOTAL_MONSTERS 50
#define INITIAL_MONSTER_COUNT 5
#define MAX_AMMO 50

// --- 구조체 ---
struct Point2D { float x, y; };
struct Bullet { Point2D pos; int dir; bool active; };
struct Monster { Point2D pos; bool active; };

// --- 전역 변수 ---
HINSTANCE g_hInst;
LPCTSTR lpszClass = L"My Window Class";
LPCTSTR lpszWindowName = L"windows program 2";

int board[BOARD_SIZE][BOARD_SIZE];
Point2D playerPos = { START_X + 7 * CELL_SIZE, START_Y + 7 * CELL_SIZE };
vector<Bullet> bullets;
vector<Monster> monsters;
int spawnedMonsterCount = 0;
int remainBullets = 50;

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

// --- WinMain (수정 없음) ---
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASSEX WndClass;
	g_hInst = hInstance;
	WndClass.cbSize = sizeof(WndClass);
	WndClass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	WndClass.lpfnWndProc = (WNDPROC)WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hInstance;
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = lpszClass;
	WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	RegisterClassEx(&WndClass);
	hWnd = CreateWindow(lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW, 0, 0, LEN, HEI, NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	while (GetMessage(&Message, 0, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return Message.wParam;
}

// --- 보조 함수 ---
void SpawnMonster() {
	if (spawnedMonsterCount >= MAX_TOTAL_MONSTERS) return;
	float rx, ry;
	while (true) {
		rx = (float)uid_x(g);
		ry = (float)uid_y(g);
		if (rx < START_X || rx > START_X + BOARD_SIZE * CELL_SIZE ||
			ry < START_Y || ry > START_Y + BOARD_SIZE * CELL_SIZE) break;
	}
	monsters.push_back({ {rx, ry}, true });
	spawnedMonsterCount++;
}

bool CanMove(float nx, float ny) {
	RECT pRect = { (int)nx, (int)ny, (int)nx + CELL_SIZE, (int)ny + CELL_SIZE };
	if (nx < START_X || ny < START_Y || nx + CELL_SIZE > START_X + BOARD_SIZE * CELL_SIZE || ny + CELL_SIZE > START_Y + BOARD_SIZE * CELL_SIZE)
		return false;

	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++) {
			if (board[i][j] == 0) {
				RECT wall = { START_X + j * CELL_SIZE, START_Y + i * CELL_SIZE, START_X + (j + 1) * CELL_SIZE, START_Y + (i + 1) * CELL_SIZE };
				RECT temp;
				if (IntersectRect(&temp, &pRect, &wall)) return false;
			}
		}
	}
	return true;
}

// --- 메인 메시지 처리 ---
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static RECT rt;
	switch (uMsg) {
	case WM_CREATE:
		for (int i = 0; i < BOARD_SIZE; i++) {
			for (int j = 0; j < BOARD_SIZE; j++) {
				if (i >= 6 && i <= 8 && j >= 6 && j <= 8) board[i][j] = 1;
				else board[i][j] = 0;
			}
		}
		for (int i = 0; i < INITIAL_MONSTER_COUNT; i++) SpawnMonster();
		SetTimer(hWnd, 0, 10, NULL);
		SetTimer(hWnd, 1, 30, NULL);
		break;

	case WM_KEYDOWN:
		if (remainBullets > 0) {
			if (wParam == VK_LEFT || wParam == VK_RIGHT || wParam == VK_UP || wParam == VK_DOWN) {
				bullets.push_back({ {playerPos.x + 15, playerPos.y + 15}, (int)wParam, true });
				remainBullets--;
			}
		}
		break;

	case WM_TIMER:
		if (wParam == 0) { // 주인공 & 총알
			if (GetKeyState('W') & 0x8000) if (CanMove(playerPos.x, playerPos.y - 1)) playerPos.y -= 1;
			if (GetKeyState('S') & 0x8000) if (CanMove(playerPos.x, playerPos.y + 1)) playerPos.y += 1;
			if (GetKeyState('A') & 0x8000) if (CanMove(playerPos.x - 1, playerPos.y)) playerPos.x -= 1;
			if (GetKeyState('D') & 0x8000) if (CanMove(playerPos.x + 1, playerPos.y)) playerPos.x += 1;

			for (int i = 0; i < (int)bullets.size(); i++) {
				if (!bullets[i].active) continue;
				if (bullets[i].dir == VK_UP) bullets[i].pos.y -= 5;
				else if (bullets[i].dir == VK_DOWN) bullets[i].pos.y += 5;
				else if (bullets[i].dir == VK_LEFT) bullets[i].pos.x -= 5;
				else if (bullets[i].dir == VK_RIGHT) bullets[i].pos.x += 5;

				RECT bRect = { (int)bullets[i].pos.x, (int)bullets[i].pos.y, (int)bullets[i].pos.x + 10, (int)bullets[i].pos.y + 10 };
				for (int r = 0; r < BOARD_SIZE; r++) {
					for (int c = 0; c < BOARD_SIZE; c++) {
						if (board[r][c] == 0) {
							RECT wall = { START_X + c * CELL_SIZE, START_Y + r * CELL_SIZE, START_X + (c + 1) * CELL_SIZE, START_Y + (r + 1) * CELL_SIZE };
							RECT temp;
							if (IntersectRect(&temp, &bRect, &wall)) board[r][c] = 1;
						}
					}
				}
				for (auto& m : monsters) {
					if (!m.active) continue;
					RECT mRect = { (int)m.pos.x, (int)m.pos.y, (int)m.pos.x + 40, (int)m.pos.y + 40 };
					RECT temp;
					if (IntersectRect(&temp, &bRect, &mRect)) {
						m.active = false;
						if (remainBullets < MAX_AMMO) remainBullets++;
						SpawnMonster();
					}
				}
				if (bullets[i].pos.x < -500 || bullets[i].pos.x > 2000 || bullets[i].pos.y < -500 || bullets[i].pos.y > 2000) bullets[i].active = false;
			}
		}
		else if (wParam == 1) { // 몬스터
			for (auto& m : monsters) {
				if (!m.active) continue;
				float dx = playerPos.x - m.pos.x;
				float dy = playerPos.y - m.pos.y;
				float dist = sqrt(dx * dx + dy * dy);
				if (dist > 0) {
					m.pos.x += dx / dist;
					m.pos.y += dy / dist;
				}
			}
		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hDC = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd, &rt);
		HDC memDC = CreateCompatibleDC(hDC);
		HBITMAP hBit = CreateCompatibleBitmap(hDC, rt.right, rt.bottom);
		HBITMAP oldBit = (HBITMAP)SelectObject(memDC, hBit);

		// 배경
		HBRUSH hBgBrush = CreateSolidBrush(RGB(245, 245, 245));
		FillRect(memDC, &rt, hBgBrush);
		DeleteObject(hBgBrush);

		// 보드판
		HBRUSH hWallBrush = CreateSolidBrush(RGB(40, 40, 40));
		HBRUSH hEmptyBrush = CreateSolidBrush(RGB(220, 220, 220));
		HPEN hBoardPen = CreatePen(PS_SOLID, 1, RGB(150, 150, 150));
		SelectObject(memDC, hBoardPen);

		for (int i = 0; i < BOARD_SIZE; i++) {
			for (int j = 0; j < BOARD_SIZE; j++) {
				if (board[i][j] == 0) SelectObject(memDC, hWallBrush);
				else SelectObject(memDC, hEmptyBrush);

				Rectangle(memDC,
					START_X + j * CELL_SIZE,
					START_Y + i * CELL_SIZE,
					START_X + (j + 1) * CELL_SIZE + 1,
					START_Y + (i + 1) * CELL_SIZE + 1);
			}
		}
		DeleteObject(hWallBrush); DeleteObject(hEmptyBrush); DeleteObject(hBoardPen);

		// 텍스트 출력 설정
		SetBkMode(memDC, TRANSPARENT);

		// 1. 남은 총알 수 표시
		TCHAR szAmmo[50];
		_stprintf_s(szAmmo, _T("REMAINING AMMO: %d / %d"), remainBullets, MAX_AMMO);
		SetTextColor(memDC, remainBullets > 10 ? RGB(0, 0, 0) : RGB(255, 0, 0));
		TextOut(memDC, START_X, START_Y + (BOARD_SIZE * CELL_SIZE) + 20, szAmmo, (int)_tcslen(szAmmo));

		// 2. 남은 몬스터 수 표시 (현재 살아있는 수 + 앞으로 더 나올 수 있는 수)
		int activeMonsters = 0;
		for (const auto& m : monsters) if (m.active) activeMonsters++;
		int monstersLeftToSpawn = MAX_TOTAL_MONSTERS - spawnedMonsterCount;

		TCHAR szMonsters[50];
		_stprintf_s(szMonsters, _T("REMAINING MONSTERS: %d / %d"), activeMonsters + monstersLeftToSpawn, MAX_TOTAL_MONSTERS);
		SetTextColor(memDC, RGB(0, 0, 255)); // 몬스터 수는 파란색으로 표시
		TextOut(memDC, START_X, START_Y + (BOARD_SIZE * CELL_SIZE) + 45, szMonsters, (int)_tcslen(szMonsters));

		// 총알 그리기
		HBRUSH hBulletBrush = CreateSolidBrush(RGB(100, 100, 100));
		SelectObject(memDC, hBulletBrush);
		for (auto& b : bullets) {
			if (b.active) {
				Rectangle(memDC, (int)b.pos.x, (int)b.pos.y, (int)b.pos.x + 8, (int)b.pos.y + 8);
			}
		}
		DeleteObject(hBulletBrush);

		// 몬스터 그리기
		HBRUSH mBrush = CreateSolidBrush(RGB(220, 50, 50));
		SelectObject(memDC, mBrush);
		for (auto& m : monsters) {
			if (m.active) {
				Rectangle(memDC, (int)m.pos.x, (int)m.pos.y, (int)m.pos.x + 40, (int)m.pos.y + 40);
			}
		}
		DeleteObject(mBrush);

		// 주인공 그리기
		HBRUSH pBrush = CreateSolidBrush(RGB(100, 200, 240));
		SelectObject(memDC, pBrush);
		RoundRect(memDC, (int)playerPos.x, (int)playerPos.y, (int)playerPos.x + 40, (int)playerPos.y + 40, 15, 15);
		DeleteObject(pBrush);

		BitBlt(hDC, 0, 0, rt.right, rt.bottom, memDC, 0, 0, SRCCOPY);
		SelectObject(memDC, oldBit);
		DeleteObject(hBit);
		DeleteDC(memDC);
		EndPaint(hWnd, &ps);
	}
	break;

	case WM_ERASEBKGND: return 1;
	case WM_DESTROY:
		KillTimer(hWnd, 0); KillTimer(hWnd, 1);
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}