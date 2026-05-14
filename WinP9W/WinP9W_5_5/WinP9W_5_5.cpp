#include <windows.h>
#include <tchar.h>
#include <random>
#include <vector>
#include <math.h>
#include <algorithm>
#include <atlimage.h>


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

int board[BOARD_LEN][BOARD_HEI] = { 0 };
int srtmx, srtmy;
int init_obstacles = 2;
int g_dirX, g_dirY;
vector<RECTS> g_tiles;
int goal_score = 32;
bool game_start = false;
bool isMoving = false;

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


LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC;
	HPEN hPen, oldPen;
	HBRUSH hBrush, oldBrush;
	static RECT rt;
	static int mx, my;
	static CImage img_background, img_cat, img_mouse;
	static HBITMAP hBitmap2, hBitmap4, hBitmap8, hBitmap16, hBitmap32, hBitmap64;

	switch (uMsg) {
	case WM_CREATE:
		img_background.Load(TEXT("map"));
		img_cat.Load(TEXT("Cat_Grey_White"));
		img_mouse.Load(TEXT("MouseIdle"));
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
			if (game_start) {
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
		if (!isMoving) {
			mx = LOWORD(lParam);
			my = HIWORD(lParam);
			srtmx = mx;
			srtmy = my;
		}
		break;
	case WM_LBUTTONUP:
	{
		if (!isMoving) {
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
			isMoving = true;
			MoveRects(hWnd, g_dirX, g_dirY);
		}
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
				isMoving = false;
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