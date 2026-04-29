#include <windows.h>
#include <tchar.h>
#include <random>
#include <vector>
#include <math.h>
#include "resource.h"

#define LEN 800
#define HEI 600
#define pie_num 5
#define board_len 10
#define board_hei 10
#define mine_num 20
#define item_num 10
#define CELL_SIZE 40
#define START_X 10
#define START_Y 10

using namespace std;
random_device rd;
mt19937 g(rd());
uniform_int_distribution<> uid_borad_len{ 0, board_len - 1 };
uniform_int_distribution<> uid_borad_hei{ 0, board_hei - 1 };
uniform_int_distribution<> uid_rgb{ 0, 255 };
uniform_int_distribution<> uid_speed{ 10, 30 };
uniform_int_distribution<> uid_color{ 2, 5 };

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"My Window Class";
LPCTSTR lpszWindowName = L"windows program 2";

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

int closed_board[board_len][board_hei] = {0};
int opened_board[board_len][board_hei] = {0};
vector <COLORREF> pie_color;
bool game_end = false;
int score = 0;
bool show_score = false;
int mode = 0;
bool isDragging = false;
POINT startPT, endPT;
bool IsHint = false;

void init_setting()
{
	game_end = false;
	score = 0;
	int x, y;

	for (int i = 0; i < board_hei; ++i) {
		for (int j = 0; j < board_len; ++j) {
			closed_board[i][j] = 0;
			opened_board[i][j] = 0;
		}
	}

	for (int i = 0; i < pie_num * 4; ++i) {		// 파이 생성
		do {
			x = uid_borad_len(g);
			y = uid_borad_hei(g);
		} while (closed_board[x][y] != 0);
		closed_board[x][y] = i + 3;
	}
	pie_color.clear();
	for (int i = 0; i < pie_num; ++i)
		pie_color.push_back(RGB(uid_rgb(g), uid_rgb(g), uid_rgb(g)));

	for (int i = 0; i < mine_num; ++i) {		// 지뢰 생성
		do {
			x = uid_borad_len(g);
			y = uid_borad_hei(g);
		} while (closed_board[x][y] != 0);
		closed_board[x][y] = 1;
	}

	for (int i = 0; i < item_num; ++i) {		// 아이템 생성
		do {
			x = uid_borad_len(g);
			y = uid_borad_hei(g);
		} while (closed_board[x][y] != 0);
		closed_board[x][y] = 2;
	}
}

void CheckCompleteCircle(int lastI, int lastJ, int& score) {
	int val = closed_board[lastI][lastJ];
	if (val <= 2) return;

	int targetGroup = (val - 3) / 4;
	int openedCount = 0;

	for (int i = 0; i < board_len; ++i) {
		for (int j = 0; j < board_hei; ++j) {
			int currentVal = closed_board[i][j];
			if (currentVal > 2 && (currentVal - 3) / 4 == targetGroup) {
				if (opened_board[i][j] == 1) {
					openedCount++;
				}
			}
		}
	}

	if (openedCount == 4)
		score++;
}

void OpenCellEvent(HWND hWnd, int i, int j, int& score) {
	switch (closed_board[i][j]) {
	case 1:
		game_end = true;
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case 2:
	{
		vector<int> temp(pie_num + 1);

		for (int i = 0; i < board_len; ++i) {		// 현재 열린 파이 번호 저장
			for (int j = 0; j < board_hei; ++j) {
				if (opened_board[i][j] == 1 && closed_board[i][j] > 2) {
					temp[(closed_board[i][j] - 3) / 4] = 1;
				}
			}
		}

		for (int i = 0; i < board_len; ++i) {
			for (int j = 0; j < board_hei; ++j) {
				for (int k = 0; k < pie_num; ++k) {
					if (opened_board[i][j] == 0 && closed_board[i][j] > 2 && k == ((closed_board[i][j] - 3) / 4) && temp[k] == 1) {
						opened_board[i][j] = 1;
						CheckCompleteCircle(i, j, score);
					}
				}
			}
		}
	}
	break;

	default:
		if (closed_board[i][j] > 2) {
			CheckCompleteCircle(i, j, score);
		}
		break;
	}
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