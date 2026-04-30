#include <windows.h>
#include <tchar.h>
#include <random>
#include <vector>
#include <math.h>

#define LEN 800
#define HEI 600
#define BRICK_LEN 60
#define BRICK_HEI 20
#define BRICK_LINE 3
#define BRICK_NUM 10
#define START_X 100
#define START_Y 20
#define PADDLE_LEN 150
#define PADDLE_HEI 10

using namespace std;
random_device rd;
mt19937 g(rd());
uniform_int_distribution<> uid_rgb{ 0, 255 };
uniform_int_distribution<> uid_speed{ 10, 40 };
uniform_int_distribution<> uid_dir{ 0, 1 };

struct Point2D { float x, y; };
struct PADDLE { Point2D pos; int velocity; int dir; };
struct BRICK { Point2D pos; int line; COLORREF color; bool IsFalling; int dir; };
struct BALL { Point2D pos, moving_pos; };	// 현재 좌표(pos)와 x, y축으로 프레임당 얼마만큼 움직이는지(moving_pos) 저장

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"My Window Class";
LPCTSTR lpszWindowName = L"windows program 2";

PADDLE paddle;
BRICK bricks[BRICK_NUM][BRICK_LINE];
BALL ball;
int colorChangedBricks = 0;
int brokenBricks = 0;

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

void init_setting()
{
	// 벽돌 세팅
	for (int i = 0; i < BRICK_NUM; ++i) {
		int dir = uid_dir(g);
		for (int j = 0; j < BRICK_LINE; ++j) {
			bricks[i][j].color = RGB(255, 255, 0);
			bricks[i][j].IsFalling = false;
			bricks[i][j].line = i;
			bricks[i][j].dir = dir;
			bricks[i][j].pos.x = START_X + BRICK_LEN * j;
			bricks[i][j].pos.y = START_Y + BRICK_HEI * i;
		}
	}

	// 패들 세팅
	paddle.pos.x = LEN / 2;
	paddle.pos.y = HEI - 100;
	paddle.velocity = 0;

	// 공 세팅
	ball.pos.x = paddle.pos.x;
	ball.pos.y = paddle.pos.y - (PADDLE_HEI / 2);
	ball.moving_pos.x = 0;
	ball.moving_pos.y = 0;

	colorChangedBricks = 0;
	brokenBricks = 0;
}

void CollisionToPaddle() 
{

}

void CollisionToBrick()
{

}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC;
	HPEN hPen, oldPen;
	HBRUSH hBrush, oldBrush;
	TCHAR str[50];
	static RECT rt;
	static int mx, my;

	switch (uMsg) {
	case WM_CREATE:
		break;
	case WM_KEYDOWN:
		if (wParam == 'M') {
			mode = (mode == 0) ? 1 : 0;
			isDragging = false;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;
	case WM_SIZE:
		InvalidateRect(hWnd, NULL, FALSE);
		return 0;
	case WM_GETMINMAXINFO:
	{
		LPMINMAXINFO lpmmi = (LPMINMAXINFO)lParam;

		lpmmi->ptMinTrackSize.x = 800;
		lpmmi->ptMinTrackSize.y = 600;
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

			HBRUSH hBgBrush = CreateSolidBrush(RGB(245, 245, 245));
			FillRect(memDC, &rt, hBgBrush);
			DeleteObject(hBgBrush);

			HBRUSH hBoardBrush = CreateSolidBrush(RGB(255, 255, 255));

			for (int i = 0; i < board_len; ++i) {
				for (int j = 0; j < board_hei; ++j) {
					Rectangle(memDC, START_X + j * CELL_SIZE, START_Y + i * CELL_SIZE, START_X + (j + 1) * CELL_SIZE, START_Y + (i + 1) * CELL_SIZE);
					if (opened_board[i][j] == 0) {
						HBRUSH hBrush = CreateSolidBrush(RGB(200, 200, 200));
						HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
						Rectangle(memDC, START_X + j * CELL_SIZE, START_Y + i * CELL_SIZE, START_X + (j + 1) * CELL_SIZE, START_Y + (i + 1) * CELL_SIZE);
						SelectObject(memDC, oldBrush);
						DeleteObject(hBrush);
					}
					else if (opened_board[i][j] == 1) {
						int x1 = START_X + j * CELL_SIZE;
						int y1 = START_Y + i * CELL_SIZE;
						int x2 = x1 + CELL_SIZE;
						int y2 = y1 + CELL_SIZE;
						int midX = x1 + CELL_SIZE / 2;
						int midY = y1 + CELL_SIZE / 2;
						if (closed_board[i][j] == 1) {
							SetTextColor(memDC, RGB(255, 0, 0));
							SetBkMode(memDC, TRANSPARENT);

							HFONT hFont = CreateFont(CELL_SIZE * 0.75, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
								DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
								DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
							HFONT oldFont = (HFONT)SelectObject(memDC, hFont);

							UINT oldAlign = SetTextAlign(memDC, TA_CENTER | TA_BASELINE);

							TextOut(memDC, midX, midY + (int)(CELL_SIZE * 0.25), L"B", 1);

							SetTextAlign(memDC, oldAlign);
							SelectObject(memDC, oldFont);
							DeleteObject(hFont);
						}
						else if (closed_board[i][j] == 2) {
							HBRUSH hItemBrush = CreateSolidBrush(RGB(255, 255, 0));
							HBRUSH oldItemBrush = (HBRUSH)SelectObject(memDC, hItemBrush);

							POINT pts[4] = { { midX, y1 }, { x2, midY }, { midX, y2 }, { x1, midY } };

							Polygon(memDC, pts, 4);

							SelectObject(memDC, oldItemBrush);
							DeleteObject(hItemBrush);
						}
						else if (closed_board[i][j] > 2) {
							int val = closed_board[i][j] - 3;
							int remainder = val % 4;
							int colorIdx = val / 4;

							HBRUSH hBrush = CreateSolidBrush(pie_color[colorIdx]);
							HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, hBrush);

							if (remainder == 0) Pie(memDC, x1, y1, x2, y2, midX, y1, x1, midY);
							else if (remainder == 1) Pie(memDC, x1, y1, x2, y2, x2, midY, midX, y1);
							else if (remainder == 2) Pie(memDC, x1, y1, x2, y2, midX, y2, x2, midY);
							else if (remainder == 3) Pie(memDC, x1, y1, x2, y2, x1, midY, midX, y2);

							SelectObject(memDC, oldBrush);
							DeleteObject(hBrush);
							/*if ((closed_board[i][j] - 3) % 4 == 0) {
								HBRUSH hBrush = CreateSolidBrush(pie_color[(closed_board[i][j] - 3) / 4]);
								Pie(memDC, START_X + j * CELL_SIZE, START_Y + i * CELL_SIZE, START_X + (j + 1) * CELL_SIZE, START_Y + (i + 1) * CELL_SIZE,
									START_X + (j + 1) * (CELL_SIZE / 2), START_Y + i * CELL_SIZE, START_X + j * CELL_SIZE, START_Y + (i + 1) * (CELL_SIZE / 2));
								DeleteObject(hBrush);
							}
							else if((closed_board[i][j] - 3) % 4 == 1) {
								HBRUSH hBrush = CreateSolidBrush(pie_color[(closed_board[i][j] - 3) / 4]);
								Pie(memDC, START_X + j * CELL_SIZE, START_Y + i * CELL_SIZE, START_X + (j + 1) * CELL_SIZE, START_Y + (i + 1) * CELL_SIZE,
									START_X + (j + 1) * CELL_SIZE, START_Y + (i + 1) * (CELL_SIZE / 2), START_X + (j + 1) * (CELL_SIZE / 2), START_Y + i * CELL_SIZE);
								DeleteObject(hBrush);
							}
							else if ((closed_board[i][j] - 3) % 4 == 2) {
								HBRUSH hBrush = CreateSolidBrush(pie_color[(closed_board[i][j] - 3) / 4]);
								Pie(memDC, START_X + j * CELL_SIZE, START_Y + i * CELL_SIZE, START_X + (j + 1) * CELL_SIZE, START_Y + (i + 1) * CELL_SIZE,
									START_X + (j + 1) * (CELL_SIZE / 2), START_Y + (i + 1) * CELL_SIZE, START_X + (j + 1) * CELL_SIZE, START_Y + (i + 1) * (CELL_SIZE / 2));
								DeleteObject(hBrush);
							}
							else if ((closed_board[i][j] - 3) % 4 == 3) {
								HBRUSH hBrush = CreateSolidBrush(pie_color[(closed_board[i][j] - 3) / 4]);
								Pie(memDC, START_X + j * CELL_SIZE, START_Y + i * CELL_SIZE, START_X + (j + 1) * CELL_SIZE, START_Y + (i + 1) * CELL_SIZE,
									START_X + j * CELL_SIZE, START_Y + (i + 1) * (CELL_SIZE / 2), START_X + (j + 1) * (CELL_SIZE / 2), START_Y + (i + 1) * CELL_SIZE);
								DeleteObject(hBrush);
							}*/
						}

					}
				}
			}

			if (mode == 0 && isDragging) {
				HPEN hBluePen = CreatePen(PS_SOLID, 2, RGB(0, 0, 255));
				HPEN oldPen = (HPEN)SelectObject(memDC, hBluePen);
				HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, GetStockObject(NULL_BRUSH));

				Rectangle(memDC, startPT.x, startPT.y, endPT.x, endPT.y);

				SelectObject(memDC, oldBrush);
				SelectObject(memDC, oldPen);
				DeleteObject(hBluePen);
			}

			if (show_score) {
				SetBkMode(memDC, TRANSPARENT);
				SetTextColor(memDC, RGB(0, 0, 0));
				SetTextAlign(memDC, TA_TOP);
				_stprintf_s(str, L"Score : %d", score);
				TextOut(memDC, START_X, START_Y + board_hei * CELL_SIZE + 20, str, _tcslen(str));
			}

			if (IsHint) {
				for (int i = 0; i < board_len; ++i) {
					for (int j = 0; j < board_hei; ++j) {
						Rectangle(memDC, START_X + j * CELL_SIZE, START_Y + i * CELL_SIZE, START_X + (j + 1) * CELL_SIZE, START_Y + (i + 1) * CELL_SIZE);

						int x1 = START_X + j * CELL_SIZE;
						int y1 = START_Y + i * CELL_SIZE;
						int x2 = x1 + CELL_SIZE;
						int y2 = y1 + CELL_SIZE;
						int midX = x1 + CELL_SIZE / 2;
						int midY = y1 + CELL_SIZE / 2;
						if (closed_board[i][j] == 0) {
							HBRUSH hBrush = CreateSolidBrush(RGB(230, 230, 230));
							HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
							Rectangle(memDC, START_X + j * CELL_SIZE, START_Y + i * CELL_SIZE, START_X + (j + 1) * CELL_SIZE, START_Y + (i + 1) * CELL_SIZE);
							SelectObject(memDC, oldBrush);
							DeleteObject(hBrush);
						}
						else if (closed_board[i][j] == 1) {
							SetTextColor(memDC, RGB(255, 0, 0));
							SetBkMode(memDC, TRANSPARENT);

							HFONT hFont = CreateFont(CELL_SIZE * 0.75, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
								DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
								DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
							HFONT oldFont = (HFONT)SelectObject(memDC, hFont);

							UINT oldAlign = SetTextAlign(memDC, TA_CENTER | TA_BASELINE);

							TextOut(memDC, midX, midY + (int)(CELL_SIZE * 0.25), L"B", 1);

							SetTextAlign(memDC, oldAlign);
							SelectObject(memDC, oldFont);
							DeleteObject(hFont);
						}
						else if (closed_board[i][j] == 2) {
							HBRUSH hItemBrush = CreateSolidBrush(RGB(255, 255, 0));
							HBRUSH oldItemBrush = (HBRUSH)SelectObject(memDC, hItemBrush);

							POINT pts[4] = { { midX, y1 }, { x2, midY }, { midX, y2 }, { x1, midY } };

							Polygon(memDC, pts, 4);

							SelectObject(memDC, oldItemBrush);
							DeleteObject(hItemBrush);
						}
						else if (closed_board[i][j] > 2) {
							int val = closed_board[i][j] - 3;
							int remainder = val % 4;
							int colorIdx = val / 4;

							HBRUSH hBrush = CreateSolidBrush(pie_color[colorIdx]);
							HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, hBrush);

							if (remainder == 0) Pie(memDC, x1, y1, x2, y2, midX, y1, x1, midY);
							else if (remainder == 1) Pie(memDC, x1, y1, x2, y2, x2, midY, midX, y1);
							else if (remainder == 2) Pie(memDC, x1, y1, x2, y2, midX, y2, x2, midY);
							else if (remainder == 3) Pie(memDC, x1, y1, x2, y2, x1, midY, midX, y2);

							SelectObject(memDC, oldBrush);
							DeleteObject(hBrush);
						}
					}
				}
			}

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
		case ID_MENU_GAMESTART:
			init_setting();

			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case ID_MENU_GAMEEND:
			if (!game_end)
				game_end = true;

			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case ID_MENU_HINT:
			IsHint = true;
			SetTimer(hWnd, 0, 1000, NULL);

			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case ID_MENU_SCORE:
			show_score = !show_score;

			InvalidateRect(hWnd, NULL, FALSE);
			break;
		}
		break;
	case WM_ERASEBKGND:
		return 1;

	case WM_LBUTTONDOWN:
		if (game_end) break;

		isDragging = true;
		startPT.x = LOWORD(lParam);
		startPT.y = HIWORD(lParam);
		endPT = startPT;

		if (mode == 1) {
			goto OPEN_CELL;
		}
		break;

	case WM_MOUSEMOVE:
		if (isDragging) {
			endPT.x = LOWORD(lParam);
			endPT.y = HIWORD(lParam);

			if (mode == 1) {
			OPEN_CELL:
				int j = (endPT.x - START_X) / CELL_SIZE;
				int i = (endPT.y - START_Y) / CELL_SIZE;
				if (i >= 0 && i < board_hei && j >= 0 && j < board_len) {
					if (opened_board[i][j] == 0) {
						opened_board[i][j] = 1;
						OpenCellEvent(hWnd, i, j, score);
					}
				}
			}
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;

	case WM_LBUTTONUP:
		if (isDragging) {
			if (mode == 0) {
				int left = min(startPT.x, endPT.x);
				int top = min(startPT.y, endPT.y);
				int right = max(startPT.x, endPT.x);
				int bottom = max(startPT.y, endPT.y);

				for (int i = 0; i < board_hei; i++) {
					for (int j = 0; j < board_len; j++) {
						int cellX1 = START_X + j * CELL_SIZE;
						int cellY1 = START_Y + i * CELL_SIZE;
						int cellX2 = cellX1 + CELL_SIZE;
						int cellY2 = cellY1 + CELL_SIZE;

						if (!(cellX2 < left || cellX1 > right || cellY2 < top || cellY1 > bottom)) {
							if (opened_board[i][j] == 0) {
								opened_board[i][j] = 1;
								OpenCellEvent(hWnd, i, j, score);
							}
						}
					}
				}
			}
			isDragging = false;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;

	case WM_TIMER:
		GetClientRect(hWnd, &rt);

		if (wParam == 0) {
			KillTimer(hWnd, 0);

			IsHint = false;
			InvalidateRect(hWnd, NULL, FALSE);
		}

		InvalidateRect(hWnd, NULL, FALSE);
		UpdateWindow(hWnd);
		break;
	case WM_DESTROY:
		KillTimer(hWnd, 1);
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}