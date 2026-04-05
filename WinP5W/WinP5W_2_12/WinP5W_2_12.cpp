#include <windows.h>
#include <tchar.h>
#include <random>
#include <vector>
#include <cmath>

using namespace std;
random_device rd;
mt19937 g(rd());
uniform_int_distribution<> uid{ 0, 19 };
uniform_int_distribution<> uid_rgb{ 0, 255 };
uniform_int_distribution<> uid_drawBoard{ 0, 39 };
uniform_int_distribution<> uid_size{ -3, 3 };
#define LEN 1000
#define HEI 1000

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
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
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

typedef struct {
	int shape;		// 0 삼각형 / 1 별 / 2 사각형 / 3 원
	int size;
	int x;
	int y;
	int color;
}PLAYER;

static PLAYER player1, player2, goal;
static int board_state[40][40];			// 0 빈공간 / 1 플레이어1 / 2 플레이어2 / 3 장애물 / 4 목적지 / 5~8 모양변경 / 9~11 크기증가 / 12~14 크기감소 / 15~34 색변경 // 35 기본크기
static vector<COLORREF> colors(20);
static int initShape1 = -1;
static int initShape2 = -1;

void initGameSetting()
{
	for (int i = 0; i < 40; ++i) {
		for (int j = 0; j < 40; ++j)
			board_state[i][j] = 0;
	}

	for (int i = 0; i < 20; ++i)		// 색 초기값
		colors[i] = (RGB(uid_rgb(g), uid_rgb(g), uid_rgb(g)));

	player1.shape = uid_drawBoard(g) % 4;
	player1.size = 0;
	player1.x = 0;
	player1.y = 0;
	player1.color = uid_drawBoard(g) % 20;
	initShape1 = player1.shape;

	player2.shape = uid_drawBoard(g) % 4;
	player2.size = 0;
	player2.x = 39;
	player2.y = 0;
	player2.color = uid_drawBoard(g) % 20;
	initShape2 = player2.shape;

	goal.shape = uid_drawBoard(g) % 4;
	goal.size = uid_size(g);
	goal.x = 20;
	goal.y = 39;
	goal.color = uid_drawBoard(g) % 20;

	// 초기 위치 설정 (y, x 순서로 저장)
	board_state[player1.y][player1.x] = 1;
	board_state[player2.y][player2.x] = 2;
	board_state[goal.y][goal.x] = 4;

	for (int i = 0; i < 20; ++i) {		// 장애물 생성
		int ox = uid_drawBoard(g);
		int oy = uid_drawBoard(g);
		if (board_state[oy][ox] != 0) --i;
		else board_state[oy][ox] = 3;
	}
	for (int i = 0; i < 20; ++i) {		// 색변경 생성
		int x = uid_drawBoard(g);
		int y = uid_drawBoard(g);
		if (board_state[y][x] != 0) --i;
		else board_state[y][x] = i + 15;
	}
	for (int i = 0; i < 20; ++i) {		// 모양변경 생성
		int x = uid_drawBoard(g);
		int y = uid_drawBoard(g);
		if (board_state[y][x] != 0) --i;
		else board_state[y][x] = 5 + i % 4;
	}
	for (int i = 0; i < 20; ++i) {		// 크기증가 생성
		int x = uid_drawBoard(g);
		int y = uid_drawBoard(g);
		if (board_state[y][x] != 0) --i;
		else board_state[y][x] = 9 + i % 3;
	}
	for (int i = 0; i < 20; ++i) {		// 크기감소 생성
		int x = uid_drawBoard(g);
		int y = uid_drawBoard(g);
		if (board_state[y][x] != 0) --i;
		else board_state[y][x] = 12 + i % 3;
	}
	for (int i = 0; i < 5; ++i) {		// 기본크기 생성
		int x = uid_drawBoard(g);
		int y = uid_drawBoard(g);
		if (board_state[y][x] != 0) --i;
		else board_state[y][x] = 35;
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC;
	HPEN hPen, oldPen;
	HBRUSH hBrush, oldBrush;
	TCHAR str[50];
	static int m;
	static int offsetX, offsetY;
	static int winState = 0;
	static int moveMode = 0;		// 0 = 무한이동 / 1, 2 = 번갈아 이동
	static int resetShape1 = 0;
	static int resetShape2 = 0;

	static POINT triangle[3] = { {0, -10}, {-10, 10}, {10, 10} };
	static POINT star[10] = { {0, -10}, {3, -3}, {10, -3}, {4, 2}, {7, 10}, {0, 5}, {-7, 10}, {-4, 2}, {-10, -3}, {-3, -3} };
	static POINT rect[2] = { {-10, -10}, {10, 10} };
	static POINT ellipse[2] = { {-10, -10}, {10, 10} };
	static POINT temptriangle[3];
	static POINT tempstar[10];
	static POINT temprect[2];
	static POINT tempellipse[2];

	switch (uMsg) {
	case WM_CREATE:
		initGameSetting();
		break;

	case WM_KEYDOWN:
		if (winState == 0) {
			bool moved = false;
			// 플레이어 1 이동 (W, A, S, D)
			if ((wParam == 'W' || wParam == 'A' || wParam == 'S' || wParam == 'D') && (moveMode == 0 || moveMode == 1)) {
				if (wParam == 'W' && player1.y > 0 && board_state[player1.y - 1][player1.x] != 3) {
					--player1.y;
					--resetShape1;
				}
				else if (wParam == 'A' && player1.x > 0 && board_state[player1.y][player1.x - 1] != 3) {
					--player1.x;
					--resetShape1;
				}
				else if (wParam == 'S' && player1.y < 39 && board_state[player1.y + 1][player1.x] != 3) {
					++player1.y;
					--resetShape1;
				}
				else if (wParam == 'D' && player1.x < 39 && board_state[player1.y][player1.x + 1] != 3) {
					++player1.x;
					--resetShape1;
				}

				int cell = board_state[player1.y][player1.x];
				if (cell == 4) {
					if (player1.shape == goal.shape && player1.color == goal.color && player1.size == goal.size) {
						winState = 1;
					}
				}
				else if (cell >= 5 && cell <= 8) {
					player1.shape = cell - 5;
					resetShape1 = 30;
				}
				else if (cell >= 9 && cell <= 11) 
					player1.size = cell - 8;
				else if (cell >= 12 && cell <= 14) 
					player1.size = cell - 15;
				else if (cell >= 15 && cell <= 34) 
					player1.color = cell - 15;
				else if (cell == 35)
					player1.size = 0;
				moved = true;
				if (moveMode == 1)
					moveMode = 2;
				if (resetShape1 == 0)
					player1.shape = initShape1;
			}
			// 플레이어 2 이동 (방향키)
			else if ((wParam == VK_UP || wParam == VK_LEFT || wParam == VK_DOWN || wParam == VK_RIGHT) && (moveMode == 0 || moveMode == 2)) {
				if (wParam == VK_UP && player2.y > 0 && board_state[player2.y - 1][player2.x] != 3) {
					--player2.y;
					--resetShape2;
				}
				else if (wParam == VK_LEFT && player2.x > 0 && board_state[player2.y][player2.x - 1] != 3) {
					--player2.x;
					--resetShape2;
				}
				else if (wParam == VK_DOWN && player2.y < 39 && board_state[player2.y + 1][player2.x] != 3) {
					++player2.y;
					--resetShape2;
				}
				else if (wParam == VK_RIGHT && player2.x < 39 && board_state[player2.y][player2.x + 1] != 3) {
					++player2.x;
					--resetShape2;
				}

				int cell = board_state[player2.y][player2.x];
				if (cell == 4) {
					if (player2.shape == goal.shape && player2.color == goal.color && player2.size == goal.size) {
						winState = 2;
					}
				}
				else if (cell >= 5 && cell <= 8) {
					player2.shape = cell - 5;
					resetShape2 = 30;
				}
				else if (cell >= 9 && cell <= 11) 
					player2.size = cell - 8;
				else if (cell >= 12 && cell <= 14)
					player2.size = cell - 15;
				else if (cell >= 15 && cell <= 34) 
					player2.color = cell - 15;
				else if (cell == 35)              
					player2.size = 0;
				moved = true;
				if (moveMode == 2)
					moveMode = 1;
				if (resetShape2 == 0)
					player2.shape = initShape2;
			}
			if (moved) InvalidateRect(hWnd, NULL, TRUE);
		}
		if (wParam == VK_ESCAPE || wParam == 'Q') {
			PostQuitMessage(0);
		}
		else if (wParam == 'M') {
			if (moveMode == 0)
				moveMode = 1;
			else
				moveMode = 0;
		}
		else if (wParam == 'R') {
			initGameSetting();
			winState = 0;
			moveMode = 0;
			InvalidateRect(hWnd, NULL, TRUE);
		}
		break;


	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);
		//-----------------------------기본 판 그리기
		hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
		oldPen = (HPEN)SelectObject(hDC, hPen);
		for (int i = 0; i < 40; ++i) {
			for (int j = 0; j < 40; ++j) {
				Rectangle(hDC, 20 + 20 * j, 20 + 20 * i, 40 + 20 * j, 40 + 20 * i);
			}
		}
		SelectObject(hDC, oldPen);
		DeleteObject(hPen);

		//-----------------------------특수 칸 그리기
		SetBkMode(hDC, TRANSPARENT);
		for (int i = 0; i < 40; ++i) {
			for (int j = 0; j < 40; ++j) {
				int cell = board_state[i][j];
				if (cell == 3) {
					hBrush = CreateSolidBrush(RGB(255, 0, 0));
					oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
					Rectangle(hDC, 20 + 20 * j, 20 + 20 * i, 40 + 20 * j, 40 + 20 * i);
					SelectObject(hDC, oldBrush);
					DeleteObject(hBrush);
				}
				else if (cell == 4) {
					hBrush = CreateSolidBrush(colors[goal.color]);
					oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
					int multiplier = 10;
					if (goal.size == 0) multiplier = 10;
					else if (goal.size == 1) multiplier = 11;
					else if (goal.size == 2) multiplier = 13;
					else if (goal.size == 3) multiplier = 15;
					else if (goal.size == -1) multiplier = 9;
					else if (goal.size == -2) multiplier = 7;
					else if (goal.size == -3) multiplier = 5;

					if (goal.shape == 0) {
						for (int k = 0; k < 3; ++k) {
							temptriangle[k].x = (int)(triangle[k].x * multiplier / 10.0) + (30 + j * 20);
							temptriangle[k].y = (int)(triangle[k].y * multiplier / 10.0) + (30 + i * 20);
						}
						Polygon(hDC, temptriangle, 3);
					}
					else if (goal.shape == 1) {
						for (int k = 0; k < 10; ++k) {
							tempstar[k].x = (int)(star[k].x * multiplier / 10.0) + (30 + j * 20);
							tempstar[k].y = (int)(star[k].y * multiplier / 10.0) + (30 + i * 20);
						}
						Polygon(hDC, tempstar, 10);
					}
					else if (goal.shape == 2) {
						temprect[0].x = (int)(rect[0].x * multiplier / 10.0) + (30 + j * 20);
						temprect[0].y = (int)(rect[0].y * multiplier / 10.0) + (30 + i * 20);
						temprect[1].x = (int)(rect[1].x * multiplier / 10.0) + (30 + j * 20);
						temprect[1].y = (int)(rect[1].y * multiplier / 10.0) + (30 + i * 20);
						Rectangle(hDC, temprect[0].x, temprect[0].y, temprect[1].x, temprect[1].y);
					}
					else if (goal.shape == 3) {
						tempellipse[0].x = (int)(ellipse[0].x * multiplier / 10.0) + (30 + j * 20);
						tempellipse[0].y = (int)(ellipse[0].y * multiplier / 10.0) + (30 + i * 20);
						tempellipse[1].x = (int)(ellipse[1].x * multiplier / 10.0) + (30 + j * 20);
						tempellipse[1].y = (int)(ellipse[1].y * multiplier / 10.0) + (30 + i * 20);
						Ellipse(hDC, tempellipse[0].x, tempellipse[0].y, tempellipse[1].x, tempellipse[1].y);
					}
					SelectObject(hDC, oldBrush);
					DeleteObject(hBrush);
				}
				else if (cell >= 5 && cell <= 8) {
					int multiplier = 7;
					if (cell == 5) {
						for (int k = 0; k < 3; ++k) {
							temptriangle[k].x = (int)(triangle[k].x * multiplier / 10.0) + (30 + j * 20);
							temptriangle[k].y = (int)(triangle[k].y * multiplier / 10.0) + (30 + i * 20);
						}
						Polygon(hDC, temptriangle, 3);
					}
					else if (cell == 6) {
						for (int k = 0; k < 10; ++k) {
							tempstar[k].x = (int)(star[k].x * multiplier / 10.0) + (30 + j * 20);
							tempstar[k].y = (int)(star[k].y * multiplier / 10.0) + (30 + i * 20);
						}
						Polygon(hDC, tempstar, 10);
					}
					else if (cell == 7) {
						temprect[0].x = (int)(rect[0].x * multiplier / 10.0) + (30 + j * 20);
						temprect[0].y = (int)(rect[0].y * multiplier / 10.0) + (30 + i * 20);
						temprect[1].x = (int)(rect[1].x * multiplier / 10.0) + (30 + j * 20);
						temprect[1].y = (int)(rect[1].y * multiplier / 10.0) + (30 + i * 20);
						Rectangle(hDC, temprect[0].x, temprect[0].y, temprect[1].x, temprect[1].y);
					}
					else if (cell == 8) {
						tempellipse[0].x = (int)(ellipse[0].x * multiplier / 10.0) + (30 + j * 20);
						tempellipse[0].y = (int)(ellipse[0].y * multiplier / 10.0) + (30 + i * 20);
						tempellipse[1].x = (int)(ellipse[1].x * multiplier / 10.0) + (30 + j * 20);
						tempellipse[1].y = (int)(ellipse[1].y * multiplier / 10.0) + (30 + i * 20);
						Ellipse(hDC, tempellipse[0].x, tempellipse[0].y, tempellipse[1].x, tempellipse[1].y);
					}
				}
				else if (cell >= 9 && cell <= 11) {
					hBrush = CreateSolidBrush(RGB(0, 0, 255));
					oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
					Rectangle(hDC, 20 + 20 * j, 20 + 20 * i, 40 + 20 * j, 40 + 20 * i);
					SelectObject(hDC, oldBrush);
					DeleteObject(hBrush);

					SetTextColor(hDC, RGB(255, 255, 255));
					wsprintf(str, L"%d", cell - 8);
					TextOut(hDC, 26 + 20 * j, 23 + 20 * i, str, lstrlen(str));
					SetTextColor(hDC, RGB(0, 0, 0));
				}
				else if (cell >= 12 && cell <= 14) {
					hBrush = CreateSolidBrush(RGB(0, 255, 0));
					oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
					Rectangle(hDC, 20 + 20 * j, 20 + 20 * i, 40 + 20 * j, 40 + 20 * i);
					SelectObject(hDC, oldBrush);
					DeleteObject(hBrush);

					wsprintf(str, L"%d", cell - 15);
					TextOut(hDC, 25 + 20 * j, 23 + 20 * i, str, lstrlen(str));
				}
				else if (cell >= 15 && cell <= 34) {
					hBrush = CreateSolidBrush(colors[cell - 15]);
					oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
					Rectangle(hDC, 20 + 20 * j, 20 + 20 * i, 40 + 20 * j, 40 + 20 * i);
					SelectObject(hDC, oldBrush);
					DeleteObject(hBrush);

					wsprintf(str, L"C");
					TextOut(hDC, 25 + 20 * j, 23 + 20 * i, str, lstrlen(str));
				}
				else if (cell == 35) {
					hBrush = CreateSolidBrush(RGB(255, 0, 0));
					oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
					Rectangle(hDC, 20 + 20 * j, 20 + 20 * i, 40 + 20 * j, 40 + 20 * i);
					SelectObject(hDC, oldBrush);
					DeleteObject(hBrush);

					wsprintf(str, L"0");
					TextOut(hDC, 28 + 20 * j, 23 + 20 * i, str, lstrlen(str));
				}
			}
		}

		//-----------------------------말 그리기
		hBrush = CreateSolidBrush(colors[player1.color]);
		oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
		m = 10;
		if (player1.size == 0) m = 10;
		else if (player1.size == 1) m = 11;
		else if (player1.size == 2) m = 13;
		else if (player1.size == 3) m = 15;
		else if (player1.size == -1) m = 9;
		else if (player1.size == -2) m = 7;
		else if (player1.size == -3) m = 5;

		offsetX = 30 + player1.x * 20;
		offsetY = 30 + player1.y * 20;

		if (player1.shape == 0) {
			for (int k = 0; k < 3; ++k) {
				temptriangle[k].x = (int)(triangle[k].x * m / 10.0) + offsetX;
				temptriangle[k].y = (int)(triangle[k].y * m / 10.0) + offsetY;
			}
			Polygon(hDC, temptriangle, 3);
		}
		else if (player1.shape == 1) {
			for (int k = 0; k < 10; ++k) {
				tempstar[k].x = (int)(star[k].x * m / 10.0) + offsetX;
				tempstar[k].y = (int)(star[k].y * m / 10.0) + offsetY;
			}
			Polygon(hDC, tempstar, 10);
		}
		else if (player1.shape == 2) {
			temprect[0].x = (int)(rect[0].x * m / 10.0) + offsetX;
			temprect[0].y = (int)(rect[0].y * m / 10.0) + offsetY;
			temprect[1].x = (int)(rect[1].x * m / 10.0) + offsetX;
			temprect[1].y = (int)(rect[1].y * m / 10.0) + offsetY;
			Rectangle(hDC, temprect[0].x, temprect[0].y, temprect[1].x, temprect[1].y);
		}
		else if (player1.shape == 3) {
			tempellipse[0].x = (int)(ellipse[0].x * m / 10.0) + offsetX;
			tempellipse[0].y = (int)(ellipse[0].y * m / 10.0) + offsetY;
			tempellipse[1].x = (int)(ellipse[1].x * m / 10.0) + offsetX;
			tempellipse[1].y = (int)(ellipse[1].y * m / 10.0) + offsetY;
			Ellipse(hDC, tempellipse[0].x, tempellipse[0].y, tempellipse[1].x, tempellipse[1].y);
		}
		SelectObject(hDC, oldBrush);
		DeleteObject(hBrush);
		//----------------------------------------------------------------
		hBrush = CreateSolidBrush(colors[player2.color]);
		oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
		m = 10;
		if (player2.size == 0) m = 10;
		else if (player2.size == 1) m = 11;
		else if (player2.size == 2) m = 13;
		else if (player2.size == 3) m = 15;
		else if (player2.size == -1) m = 9;
		else if (player2.size == -2) m = 7;
		else if (player2.size == -3) m = 5;

		offsetX = 30 + player2.x * 20;
		offsetY = 30 + player2.y * 20;

		if (player2.shape == 0) {
			for (int k = 0; k < 3; ++k) {
				temptriangle[k].x = (int)(triangle[k].x * m / 10.0) + offsetX;
				temptriangle[k].y = (int)(triangle[k].y * m / 10.0) + offsetY;
			}
			Polygon(hDC, temptriangle, 3);
		}
		else if (player2.shape == 1) {
			for (int k = 0; k < 10; ++k) {
				tempstar[k].x = (int)(star[k].x * m / 10.0) + offsetX;
				tempstar[k].y = (int)(star[k].y * m / 10.0) + offsetY;
			}
			Polygon(hDC, tempstar, 10);
		}
		else if (player2.shape == 2) {
			temprect[0].x = (int)(rect[0].x * m / 10.0) + offsetX;
			temprect[0].y = (int)(rect[0].y * m / 10.0) + offsetY;
			temprect[1].x = (int)(rect[1].x * m / 10.0) + offsetX;
			temprect[1].y = (int)(rect[1].y * m / 10.0) + offsetY;
			Rectangle(hDC, temprect[0].x, temprect[0].y, temprect[1].x, temprect[1].y);
		}
		else if (player2.shape == 3) {
			tempellipse[0].x = (int)(ellipse[0].x * m / 10.0) + offsetX;
			tempellipse[0].y = (int)(ellipse[0].y * m / 10.0) + offsetY;
			tempellipse[1].x = (int)(ellipse[1].x * m / 10.0) + offsetX;
			tempellipse[1].y = (int)(ellipse[1].y * m / 10.0) + offsetY;
			Ellipse(hDC, tempellipse[0].x, tempellipse[0].y, tempellipse[1].x, tempellipse[1].y);
		}
		SelectObject(hDC, oldBrush);
		DeleteObject(hBrush);

		//----------------------------------------------------- 주석 출력		
		hBrush = CreateSolidBrush(RGB(255, 0, 0));
		oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
		Rectangle(hDC, 20, 900, 40, 920);
		SelectObject(hDC, oldBrush);
		DeleteObject(hBrush);

		wsprintf(str, L" : 장애물 칸");
		TextOut(hDC, 40, 903, str, lstrlen(str));

		hBrush = CreateSolidBrush(RGB(0, 0, 255));
		oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
		Rectangle(hDC, 140, 900, 160, 920);
		SelectObject(hDC, oldBrush);
		DeleteObject(hBrush);

		wsprintf(str, L" : 크기증가 칸");
		TextOut(hDC, 160, 903, str, lstrlen(str));

		hBrush = CreateSolidBrush(RGB(0, 255, 0));
		oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
		Rectangle(hDC, 270, 900, 290, 920);
		SelectObject(hDC, oldBrush);
		DeleteObject(hBrush);

		wsprintf(str, L" : 크기감소 칸");
		TextOut(hDC, 290, 903, str, lstrlen(str));

		wsprintf(str, L"M : 이동모드변경");
		TextOut(hDC, 395, 903, str, lstrlen(str));

		wsprintf(str, L"C : 색상변경 칸");
		TextOut(hDC, 525, 903, str, lstrlen(str));

		wsprintf(str, L"R : 리셋");
		TextOut(hDC, 640, 903, str, lstrlen(str));

		wsprintf(str, L"Q : 프로그램 종료");
		TextOut(hDC, 710, 903, str, lstrlen(str));
		//------------------------------------------------------
		if (winState == 1) {
			SetTextColor(hDC, RGB(255, 0, 0));
			wsprintf(str, L"플레이어 1 승리!!");
			TextOut(hDC, 40, 873, str, lstrlen(str));
		}
		else if (winState == 2) {
			SetTextColor(hDC, RGB(0, 0, 255));
			wsprintf(str, L"플레이어 2 승리!!");
			TextOut(hDC, 40, 873, str, lstrlen(str));
		}

			EndPaint(hWnd, &ps);
			break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
