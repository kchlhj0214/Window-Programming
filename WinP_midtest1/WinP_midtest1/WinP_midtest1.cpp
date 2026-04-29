#include <windows.h>		//--- 윈도우 헤더 파일
#include <tchar.h>
#include <random>
#include <vector>
#include <math.h>
#include <string>

using namespace std;

#define marginX 16
#define marginY 39
#define LEN 1200
#define HEI 900
#define BOARD_SIZE 15
#define CELL_SIZE 40
#define START_X 200
#define START_Y 60
#define MAX_TOTAL_MONSTER 50
#define INITIAL_MONSTER_COUNT 5
#define MAX_AMMO 50

struct Point2D { float x, y; };
struct Bullet { Point2D pos; int dir; bool active; };
struct Monster { Point2D pos; bool active; };

random_device rd;
mt19937 g(rd());
uniform_int_distribution<> uid_x{ 0, 1100 };
uniform_int_distribution<> uid_y{ 60, 660 };

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"2026 Window Programming Exam";

int board[BOARD_SIZE][BOARD_SIZE];
Point2D playerPos = { START_X + 7 * CELL_SIZE, START_Y + 7 * CELL_SIZE };
vector <Bullet> bullets;
vector <Monster> monsters;
int spawnedMonsterCount = 0;
int totalBullets = 0;

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

int  WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_  LPSTR lpszCmdParam, _In_  int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASSEX WndClass;
	g_hInst = hInstance;

	WndClass.cbSize = sizeof(WndClass);
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = (WNDPROC)WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hInstance;
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(NULL, IDI_APPLICATION);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = lpszClass;
	WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	RegisterClassEx(&WndClass);

	//--- 크기 변경 가능 (기존 (1024, 768))
	hWnd = CreateWindow(lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW, 0, 0, LEN, HEI, NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&Message, 0, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return Message.wParam;
}

// 몬스터 스폰 함수
void SpawnMonster()
{
	if (spawnedMonsterCount >= MAX_TOTAL_MONSTER) return;
	float rx, ry;
	while (true) {
		rx = (float)uid_x(g);
		ry = (float)uid_y(g);
		if (rx < START_X || rx > START_X + BOARD_SIZE * CELL_SIZE ||
			ry < START_Y || ry > START_Y + BOARD_SIZE * CELL_SIZE)
			break;
	}
	monsters.push_back({ { rx, ry }, true });
	spawnedMonsterCount++;
}

// 주인공 이동 가능 여부 판단 함수
bool CanMove(float nx, float ny)
{
	RECT pRect = { (int)nx, (int)ny, (int)nx + CELL_SIZE, (int)ny + CELL_SIZE };
	if (nx < START_X || ny < START_Y || nx + CELL_SIZE > START_X + BOARD_SIZE * CELL_SIZE || ny + CELL_SIZE > START_Y + BOARD_SIZE * CELL_SIZE)
		return false;

	for (int i = 0; i < BOARD_SIZE; ++i) {
		for (int j = 0; j < BOARD_SIZE; ++j) {
			if (board[i][j] == 0) {
				RECT wall = { START_X + j * CELL_SIZE, START_Y + i * CELL_SIZE, START_X + (j + 1) * CELL_SIZE, START_Y + (i + 1) * CELL_SIZE };
				RECT temp;
				if (IntersectRect(&temp, &pRect, &wall)) return false;
			}
		}
	}
	return true;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	static RECT rt;
	static bool IsP = false;

	switch (iMessage) {
	case WM_CREATE:
		for (int i = 0; i < BOARD_SIZE; ++i) {
			for (int j = 0; j < BOARD_SIZE; ++j)
				board[i][j] = 15;
		}
		for (int i = 0; i < INITIAL_MONSTER_COUNT; ++i)
			SpawnMonster();
		SetTimer(hWnd, 0, 10, NULL);	// 주인공 타이머
		SetTimer(hWnd, 1, 30, NULL);	// 몬스터 타이머
		break;

	case WM_KEYDOWN:
		if (wParam == VK_LEFT || wParam == VK_RIGHT || wParam == VK_UP || wParam == VK_DOWN) {
			if (totalBullets < MAX_AMMO) {
				bullets.push_back({ {playerPos.x + 15, playerPos.y + 15}, (int)wParam, true });
				totalBullets++;
			}
		}
		else if (wParam == 'Q') {
			KillTimer(hWnd, 0);
			KillTimer(hWnd, 1);
			PostQuitMessage(0);
		}
		else if (wParam == 'P') {
			if (IsP == false) {
				IsP = true;
				KillTimer(hWnd, 1);
			}
			else {
				IsP = false;
				SetTimer(hWnd, 1, 30, NULL);
			}
		}
		break;

	case WM_TIMER:
		if (wParam == 0) {	// 주인공 & 총알 구현
			if (GetKeyState('W') & 0x8000) if (CanMove(playerPos.x, playerPos.y - 1)) playerPos.y -= 1;		// 키다운으로 구현하면 1틱 이동 후 연속된 이동까지 약간의 딜레이가 생김
			if (GetKeyState('S') & 0x8000) if (CanMove(playerPos.x, playerPos.y + 1)) playerPos.y += 1;		// 이를 방지하기 위해 '키가 눌려있는가' 만을 판단하는 코드 사용
			if (GetKeyState('A') & 0x8000) if (CanMove(playerPos.x - 1, playerPos.y)) playerPos.x -= 1;
			if (GetKeyState('D') & 0x8000) if (CanMove(playerPos.x + 1, playerPos.y)) playerPos.x += 1;

			for (int i = 0; i < (int)bullets.size(); ++i) {
				if (!bullets[i].active) continue;
				if (bullets[i].dir == VK_UP) bullets[i].pos.y -= 5;
				else if (bullets[i].dir == VK_DOWN) bullets[i].pos.y += 5;
				else if (bullets[i].dir == VK_LEFT) bullets[i].pos.x -= 5;
				else if (bullets[i].dir == VK_RIGHT) bullets[i].pos.x += 5;

				// 총알이 지나간 보드의 색 변경
				RECT bRect = { (int)bullets[i].pos.x, (int)bullets[i].pos.y, (int)bullets[i].pos.x + 10, (int)bullets[i].pos.y + 10 };
				RECT pRect = { (int)playerPos.x, (int)playerPos.y, (int)playerPos.x + CELL_SIZE, (int)playerPos.y + CELL_SIZE };
				for (int r = 0; r < BOARD_SIZE; ++r) {
					for (int c = 0; c < BOARD_SIZE; ++c) {
						if (board[r][c] <= 15 && board[r][c] > 1) {		// 단순히 2, 1, 0의 상태로 만들면 총알이 한번만 지나가더라도 프레임 변경 후 다시 지나간걸로 취급되어 총알 하나로 검정색이 되어버림. 적당히 15 넣으니 2번 맞고 색이 변했음
							RECT wall = { START_X + c * CELL_SIZE, START_Y + r * CELL_SIZE, START_X + (c + 1) * CELL_SIZE, START_Y + (r + 1) * CELL_SIZE };
							RECT temp;
							if (IntersectRect(&temp, &bRect, &wall) && !IntersectRect(&temp, &pRect, &wall)) {		// !IntersectRect(&temp, &pRect, &wall) 넣은 이유 : 플레이어가 밟고 있는 보드가 검정색이 되어 끼는 현상 방지
								board[r][c] -= 1;
							}
						}
						else if (board[r][c] == 1) {
							RECT wall = { START_X + c * CELL_SIZE, START_Y + r * CELL_SIZE, START_X + (c + 1) * CELL_SIZE, START_Y + (r + 1) * CELL_SIZE };
							RECT temp;
							if (IntersectRect(&temp, &bRect, &wall) && !IntersectRect(&temp, &pRect, &wall)) board[r][c] = 0;
						}
					}
				}
				// 몬스터와 총알 충돌 판별
				for (auto& m : monsters) {
					if (!m.active) continue;
					RECT mRect = { (int)m.pos.x, (int)m.pos.y, (int)m.pos.x + 40, (int)m.pos.y + 40 };
					RECT temp;
					if (IntersectRect(&temp, &bRect, &mRect)) {
						m.active = false;
						SpawnMonster();
					}
				}
				if (bullets[i].pos.x < -500 || bullets[i].pos.x > 2000 || bullets[i].pos.y < -500 || bullets[i].pos.y > 2000) {
					bullets[i].active = false;
					totalBullets--;
				}
			}
		}
		else if (wParam == 1) {		// 몬스터 구현
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

		HBRUSH hBgBrush = CreateSolidBrush(RGB(245, 245, 245));
		FillRect(memDC, &rt, hBgBrush);
		DeleteObject(hBgBrush);

		HBRUSH hWallBrush = CreateSolidBrush(RGB(0, 0, 0));
		HBRUSH hBrokenWallBrush = CreateSolidBrush(RGB(128, 128, 128));
		HBRUSH hEmptyWallBrush = CreateSolidBrush(RGB(255, 255, 255));
		HPEN hBoardPen = CreatePen(PS_SOLID, 1, RGB(120, 120, 120));
		SelectObject(memDC, hBoardPen);

		// 보드 그리기
		for (int i = 0; i < BOARD_SIZE; ++i) {
			for (int j = 0; j < BOARD_SIZE; ++j) {
				if (board[i][j] == 0) SelectObject(memDC, hWallBrush);
				else if (board[i][j] < 15 && board[i][j] > 0) SelectObject(memDC, hBrokenWallBrush);
				else if (board[i][j] == 15) SelectObject(memDC, hEmptyWallBrush);

				Rectangle(memDC, START_X + j * CELL_SIZE, START_Y + i * CELL_SIZE, START_X + (j + 1) * CELL_SIZE, START_Y + (i + 1) * CELL_SIZE);
			}
		}
		DeleteObject(hWallBrush);
		DeleteObject(hBrokenWallBrush);
		DeleteObject(hEmptyWallBrush);
		DeleteObject(hBoardPen);

		// 현재 화면에 출력된 총알 수와 남은 몬스터 수 출력. 안해도 됨
		SetBkMode(memDC, TRANSPARENT);

		// 총알 수 출력
		TCHAR szAmmo[50];
		_stprintf_s(szAmmo, _T("PRINTING AMMO : %d / %d"), totalBullets, MAX_AMMO);
		SetTextColor(memDC, RGB(0, 0, 255));
		TextOut(memDC, START_X, START_Y + (BOARD_SIZE * CELL_SIZE) + 20, szAmmo, (int)_tcslen(szAmmo));

		// 몬스터 수 출력
		int activeMonsters = 0;
		for (const auto& m : monsters) if (m.active) activeMonsters++;
		int monsterLeft = MAX_TOTAL_MONSTER - spawnedMonsterCount;

		TCHAR szMonsters[50];
		_stprintf_s(szMonsters, _T("REMAINING MONSTERS : %d / %d"), activeMonsters + monsterLeft, MAX_TOTAL_MONSTER);
		SetTextColor(memDC, RGB(0, 0, 255));
		TextOut(memDC, START_X, START_Y + (BOARD_SIZE * CELL_SIZE) + 45, szMonsters, (int)_tcslen(szMonsters));

		// 총알 그리기
		HBRUSH hBulletBrush = CreateSolidBrush(RGB(255, 255, 0));
		SelectObject(memDC, hBulletBrush);
		for (auto& b : bullets) {
			if (b.active) {
				Ellipse(memDC, (int)b.pos.x, (int)b.pos.y, (int)b.pos.x + 8, (int)b.pos.y + 8);
			}
		}
		DeleteObject(hBulletBrush);

		// 몬스터 그리기
		HBRUSH mBrush = CreateSolidBrush(RGB(255, 0, 0));
		SelectObject(memDC, mBrush);
		for (auto& m : monsters) {
			if (m.active) {
				Rectangle(memDC, (int)m.pos.x, (int)m.pos.y, (int)m.pos.x + 40, (int)m.pos.y + 40);
			}
		}
		DeleteObject(mBrush);

		// 주인공 그리기
		HBRUSH pBrush = CreateSolidBrush(RGB(0, 0, 255));
		SelectObject(memDC, pBrush);
		RoundRect(memDC, (int)playerPos.x, (int)playerPos.y, (int)playerPos.x + 40, (int)playerPos.y + 40, 20, 20);
		DeleteObject(pBrush);

		BitBlt(hDC, 0, 0, rt.right, rt.bottom, memDC, 0, 0, SRCCOPY);
		SelectObject(memDC, oldBit);
		DeleteObject(hBit);
		DeleteDC(memDC);
		EndPaint(hWnd, &ps);
	}
	break;

	case WM_ERASEBKGND:
		return 1;

	case WM_DESTROY:
		KillTimer(hWnd, 0);
		KillTimer(hWnd, 1);
		PostQuitMessage(0);
		return 0;
	}
	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}
