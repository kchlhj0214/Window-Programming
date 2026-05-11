#include <windows.h>
#include <tchar.h>
#include <random>
#include <vector>
#include <math.h>
#include <algorithm>
#include "resource.h"

#define LEN 1000
#define HEI 1000
#define CELL_SIZE 100
#define BOARD_LEN 6
#define BOARD_HEI 6
#define MAX_BLOCK 20
#define SPEED 10

using namespace std;
random_device rd;
mt19937 g(rd());
uniform_int_distribution<> uid_rgb{ 0, 255 };
uniform_int_distribution<> uid_board_x{ 0, BOARD_LEN - 1 };
uniform_int_distribution<> uid_board_y{ 0, BOARD_HEI - 1 };
uniform_int_distribution<> uid_pos4{ 0, 14 };
uniform_int_distribution<> uid_pos5{ 0, 23 };

struct RECTS { POINT pos; int num; int dir; RECT curPos; POINT target; bool isMoving; };


HINSTANCE g_hInst;
LPCTSTR lpszClass = L"My Window Class";
LPCTSTR lpszWindowName = L"windows program 2";

int board[BOARD_LEN][BOARD_HEI] = {0};
int srtmx, srtmy;
int init_obstacles = 2;
int g_dirX, g_dirY;
vector<RECTS> g_tiles;
int goal_score = 32;
bool game_start = false;

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

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
	WndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
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
void UpdateTileList();

void init_setting()
{
	for (int i = 0; i < 4; ++i) {
		int x = uid_board_x(g);
		int y = uid_board_y(g);
		while (board[x][y] != 0) {
			x = uid_board_x(g);
			y = uid_board_y(g);
		}
		if (i % 2 == 0)
			board[x][y] = 1;
		else
			board[x][y] = 2;
	}
	UpdateTileList();
}

bool CanMove(int dx, int dy)
{
	for (int y = 0; y < BOARD_HEI; y++) {
		for (int x = 0; x < BOARD_LEN; x++) {
			if (board[x][y] <= 0) continue;

			int nx = x + dx;
			int ny = y + dy;

			if (nx >= 0 && nx < BOARD_LEN && ny >= 0 && ny < BOARD_HEI) {
				if (board[nx][ny] == 0 || board[nx][ny] == board[x][y])
					return true;
			}
		}
	}
	return false;
}

void MoveRects(HWND hWnd, int dx, int dy)
{
	if (CanMove(dx, dy) == false) return;

	bool isChanged = false;
	bool merged[BOARD_LEN][BOARD_HEI] = { false };

	int startX, endX, stepX;
	int startY, endY, stepY;

	if (dx > 0) { startX = BOARD_LEN - 1; endX = -1; stepX = -1; }
	else { startX = 0; endX = BOARD_LEN; stepX = 1; }
	if (dy > 0) { startY = BOARD_HEI - 1; endY = -1; stepY = -1; }
	else { startY = 0; endY = BOARD_HEI; stepY = 1; }

	for (int i = startX; i != endX; i += stepX) {
		for (int j = startY; j != endY; j += stepY) {
			if (board[i][j] <= 1) continue;

			int curX = i, curY = j;
			int originX = i, originY = j; // 시작 위치 기억

			while (true) {
				int nextX = curX + dx, nextY = curY + dy;
				if (nextX < 0 || nextX >= BOARD_LEN || nextY < 0 || nextY >= BOARD_HEI) break;

				if (board[nextX][nextY] == 0) {
					board[nextX][nextY] = board[curX][curY];
					board[curX][curY] = 0;
					curX = nextX; curY = nextY;
					isChanged = true;
				}
				else if (board[nextX][nextY] > 1) {
					if (board[nextX][nextY] == board[curX][curY] && !merged[nextX][nextY]) {
						board[nextX][nextY] *= 2;
						board[curX][curY] = 0;
						merged[nextX][nextY] = true;
						isChanged = true;
					}
					break;
				}
				else if (board[nextX][nextY] == 1) break;
			}

			// 해당 위치에 있던 타일을 찾아서 목표 지점 설정
			if (isChanged) {
				for (auto& tile : g_tiles) {
					if (tile.pos.x == originX && tile.pos.y == originY) {
						tile.pos = { curX, curY };
						tile.target = { curX * CELL_SIZE, curY * CELL_SIZE };
						tile.isMoving = true;
						break;
					}
				}
			}
		}
	}

	if (isChanged) {
		SetTimer(hWnd, 1, 10, NULL);
	}

	for (int y = 0; y < BOARD_HEI; y++) {
		for (int x = 0; x < BOARD_LEN; x++) {
			if (board[x][y] == goal_score) {
				MessageBox(hWnd, L"목표점수를 달성하였습니다!", L"게임 종료", MB_OK);
				game_start = false;
				InvalidateRect(hWnd, NULL, FALSE);
				break;
			}
		}
	}
}

void UpdateTileList()
{
	g_tiles.clear(); // 기존 리스트 삭제
	for (int i = 0; i < BOARD_LEN; i++) {
		for (int j = 0; j < BOARD_HEI; j++) {
			if (board[i][j] > 1) { // 숫자 타일인 경우만 추가
				RECTS newTile;
				newTile.num = board[i][j];
				newTile.pos = { i, j };

				// 현재 화면 좌표 초기화
				newTile.curPos.left = i * CELL_SIZE;
				newTile.curPos.top = j * CELL_SIZE;
				newTile.curPos.right = newTile.curPos.left + CELL_SIZE;
				newTile.curPos.bottom = newTile.curPos.top + CELL_SIZE;

				newTile.target = { i * CELL_SIZE, j * CELL_SIZE };
				newTile.isMoving = false;

				g_tiles.push_back(newTile);
			}
		}
	}
}

void AddTwo()
{
	int x = uid_board_x(g);
	int y = uid_board_y(g);
	while (board[x][y] != 0) {
		x = uid_board_x(g);
		y = uid_board_y(g);
	}
	board[x][y] = 2;
	UpdateTileList();
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC;
	HPEN hPen, oldPen;
	HBRUSH hBrush, oldBrush;
	static RECT rt;
	static int mx, my;
	static BITMAP bmp2, bmp4, bmp8, bmp16, bmp32, bmp64;
	static HBITMAP hBitmap2, hBitmap4, hBitmap8, hBitmap16, hBitmap32, hBitmap64;

	switch (uMsg) {
	case WM_CREATE:
		hBitmap2 = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP1));
		hBitmap4 = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP2));
		hBitmap8 = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP3));
		hBitmap16 = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP4));
		hBitmap32 = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP5));
		hBitmap64 = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP6));
		GetObject(hBitmap2, sizeof(BITMAP), &bmp2);
		GetObject(hBitmap4, sizeof(BITMAP), &bmp4);
		GetObject(hBitmap8, sizeof(BITMAP), &bmp8);
		GetObject(hBitmap16, sizeof(BITMAP), &bmp16);
		GetObject(hBitmap32, sizeof(BITMAP), &bmp32);
		GetObject(hBitmap64, sizeof(BITMAP), &bmp64);
		init_setting();
		break;
	case WM_SIZE:
		InvalidateRect(hWnd, NULL, FALSE);
		return 0;
	case WM_GETMINMAXINFO:
	{
		LPMINMAXINFO lpmmi = (LPMINMAXINFO)lParam;

		lpmmi->ptMinTrackSize.x = LEN;
		lpmmi->ptMinTrackSize.y = HEI;
	}
	return 0;
	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd, &rt);
		{
			// 백버퍼 생성
			HDC memDC = CreateCompatibleDC(hDC);
			HBITMAP hBit = CreateCompatibleBitmap(hDC, rt.right, rt.bottom);
			HBITMAP oldBit = (HBITMAP)SelectObject(memDC, hBit);

			// 배경 지우기 (더블 버퍼링의 핵심: 하얀색으로 백버퍼를 채움)
			FillRect(memDC, &ps.rcPaint, (HBRUSH)GetStockObject(WHITE_BRUSH));
			//------------------------------------------------------------------
			if(game_start){
				for (int i = 0; i < BOARD_LEN; i++) {
					for (int j = 0; j < BOARD_HEI; j++) {
						RECT cell = { i * CELL_SIZE, j * CELL_SIZE, (i + 1) * CELL_SIZE, (j + 1) * CELL_SIZE };
						Rectangle(memDC, cell.left, cell.top, cell.right, cell.bottom);

						if (board[i][j] == 1) {
							HBRUSH obsBrush = CreateSolidBrush(RGB(255, 0, 0));
							FillRect(memDC, &cell, obsBrush);
							DeleteObject(obsBrush);
						}
					}
				}

				// 2. 비트맵 출력을 위한 메모리 DC 생성
				HDC imgDC = CreateCompatibleDC(hDC);

				for (auto& tile : g_tiles) {
					if (tile.num <= 0) continue;

					HBITMAP hTargetBmp = NULL;
					int bmpW = 0, bmpH = 0;

					if (tile.num == 2) { hTargetBmp = hBitmap2; bmpW = bmp2.bmWidth; bmpH = bmp2.bmHeight; }
					else if (tile.num == 4) { hTargetBmp = hBitmap4; bmpW = bmp4.bmWidth; bmpH = bmp4.bmHeight; }
					else if (tile.num == 8) { hTargetBmp = hBitmap8; bmpW = bmp8.bmWidth; bmpH = bmp8.bmHeight; }
					else if (tile.num == 16) { hTargetBmp = hBitmap16; bmpW = bmp16.bmWidth; bmpH = bmp16.bmHeight; }
					else if (tile.num == 32) { hTargetBmp = hBitmap32; bmpW = bmp32.bmWidth; bmpH = bmp32.bmHeight; }
					else if (tile.num == 64) { hTargetBmp = hBitmap64; bmpW = bmp64.bmWidth; bmpH = bmp64.bmHeight; }

					if (hTargetBmp != NULL) {
						SelectObject(imgDC, hTargetBmp);
						StretchBlt(memDC, tile.curPos.left, tile.curPos.top, tile.curPos.right - tile.curPos.left, tile.curPos.bottom - tile.curPos.top,
							imgDC, 0, 0, bmpW, bmpH, SRCCOPY);
					}
				}

				DeleteDC(imgDC);
			}
			//------------------------------------------------------------------
			// 완성된 백버퍼를 실제 화면으로 한 번에 복사
			BitBlt(hDC, 0, 0, rt.right, rt.bottom, memDC, 0, 0, SRCCOPY);

			SelectObject(memDC, oldBit);
			DeleteObject(hBit);
			DeleteDC(memDC);
		}
		EndPaint(hWnd, &ps);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_GAME_GAMESTART:
			game_start = true;
			init_setting();

			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case ID_GAME_GAMEEND:
			game_start = false;
			MessageBox(hWnd, L"게임이 종료되었습니다", L"게임 종료", MB_OK);

			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case ID_GOALSCORE_32:
			goal_score = 32;

			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case ID_GOALSCORE_64:
			goal_score = 64;

			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case ID_OBSTACLES_2:
		{
			init_obstacles = 2;
			int n = 0;
			for (int i = 0; i < BOARD_LEN; ++i) {
				for (int j = 0; j < BOARD_HEI; ++j) {
					if (board[i][j] == 1) board[i][j] = 0;
					if (board[i][j] == 0) n++;
				}
			}

			for (int i = 0; i < init_obstacles; ++i) {
				int x = uid_board_x(g);
				int y = uid_board_y(g);
				while (board[x][y] != 0) {
					x = uid_board_x(g);
					y = uid_board_y(g);
				}
				board[x][y] = 1;
			}
			InvalidateRect(hWnd, NULL, FALSE);
		}
			break;
		case ID_OBSTACLES_3:
		{
			init_obstacles = 3;
			int n = 0;
			for (int i = 0; i < BOARD_LEN; ++i) {
				for (int j = 0; j < BOARD_HEI; ++j) {
					if (board[i][j] == 1) board[i][j] = 0;
					if (board[i][j] == 0) n++;
				}
			}

			for (int i = 0; i < init_obstacles; ++i) {
				int x = uid_board_x(g);
				int y = uid_board_y(g);
				while (board[x][y] != 0) {
					x = uid_board_x(g);
					y = uid_board_y(g);
				}
				board[x][y] = 1;
			}

			InvalidateRect(hWnd, NULL, FALSE);
		}
			break;
		case ID_OBSTACLES_4:
		{
			init_obstacles = 4;
			int n = 0;
			for (int i = 0; i < BOARD_LEN; ++i) {
				for (int j = 0; j < BOARD_HEI; ++j) {
					if (board[i][j] == 1) board[i][j] = 0;
					if (board[i][j] == 0) n++;
				}
			}

			for (int i = 0; i < init_obstacles; ++i) {
				int x = uid_board_x(g);
				int y = uid_board_y(g);
				while (board[x][y] != 0) {
					x = uid_board_x(g);
					y = uid_board_y(g);
				}
				board[x][y] = 1;
			}

			InvalidateRect(hWnd, NULL, FALSE);
		}
			break;
		}
		break;
	case WM_ERASEBKGND:
		return 1;

	case WM_LBUTTONDOWN:
		mx = LOWORD(lParam);
		my = HIWORD(lParam);
		srtmx = mx;
		srtmy = my;
		break;
	case WM_LBUTTONUP:
	{
		mx = LOWORD(lParam);
		my = HIWORD(lParam);

		int dx = abs(mx - srtmx);
		int dy = abs(my - srtmy);
		if (dx > dy) {
			g_dirY = 0;
			if (mx - srtmx > 0) g_dirX = 1;
			else g_dirX = -1;
		}
		else {
			g_dirX = 0;
			if (my - srtmy > 0) g_dirY = 1;
			else g_dirY = -1;
		}
		MoveRects(hWnd, g_dirX, g_dirY);
	}
		break;
	case WM_TIMER:
		if (wParam == 1) {
			bool allArrived = true;
			for (auto& tile : g_tiles) {
				if (tile.isMoving) {
					allArrived = false;

					// X축 이동
					if (tile.curPos.left < tile.target.x) tile.curPos.left += SPEED;
					else if (tile.curPos.left > tile.target.x) tile.curPos.left -= SPEED;

					// Y축 이동
					if (tile.curPos.top < tile.target.y) tile.curPos.top += SPEED;
					else if (tile.curPos.top > tile.target.y) tile.curPos.top -= SPEED;

					// 목적지 도달 확인 (오차 보정)
					if (abs(tile.curPos.left - tile.target.x) <= SPEED) tile.curPos.left = tile.target.x;
					if (abs(tile.curPos.top - tile.target.y) <= SPEED) tile.curPos.top = tile.target.y;

					// Right, Bottom 갱신
					tile.curPos.right = tile.curPos.left + CELL_SIZE;
					tile.curPos.bottom = tile.curPos.top + CELL_SIZE;

					if (tile.curPos.left == tile.target.x && tile.curPos.top == tile.target.y) {
						tile.isMoving = false;
					}
					if (tile.curPos.left == tile.target.x && tile.curPos.top == tile.target.y) {
						tile.isMoving = false;
					}
					else {
						allArrived = false; // 하나라도 이동 중이면 false
					}
				}
			}

			InvalidateRect(hWnd, NULL, FALSE);
			if (allArrived) {
				KillTimer(hWnd, 1);
				AddTwo();
				UpdateTileList();
			}
		}
		break;
	case WM_DESTROY:
		DeleteObject(hBitmap2);
		DeleteObject(hBitmap4);
		DeleteObject(hBitmap8);
		DeleteObject(hBitmap16);
		DeleteObject(hBitmap32);
		DeleteObject(hBitmap64);
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}