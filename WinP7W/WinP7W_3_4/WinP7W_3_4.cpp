#include <windows.h>
#include <tchar.h>
#include <random>
#include <vector>
#include <math.h>

using namespace std;
random_device rd;
mt19937 g(rd());
uniform_int_distribution<> uid_movemode{ 1, 5 };
uniform_int_distribution<> uid_rgb{ 0, 255 };
uniform_int_distribution<> uid_drawBoard{ 0, 29 };
uniform_int_distribution<> uid_color{ 2, 5 };
#define LEN 800
#define HEI 800

#define W 30
#define H 30
#define CNUM 2


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

static int board[W][H] = { 0 };
static int item_count = 0;

void spawn_item(int i) {
	int ix, iy;
	do {
		ix = uid_drawBoard(g);
		iy = uid_drawBoard(g);
	} while (board[ix][iy] != 0 || iy == 0);
	if (i % CNUM == 0) board[ix][iy] = 2;
	else if (i % CNUM == 1) board[ix][iy] = 3;
	//else if (i % 4 == 2) board[ix][iy] = 4;
	//else if (i % 4 == 3) board[ix][iy] = 5;

	item_count++;
}

typedef struct {
	COLORREF color;
	int r;
	int x, y;
} CIRCLE;

typedef struct {
	int x, y;
	int color;			// 4가지 색
	int attached_Dir;	// 0 안붙음 / 1 아래로 / 2 왼쪽 / 3 위쪽 / 4 오른쪽
}ITEM;

void init_setting(ITEM& c) {
	c.color = 1;
	c.x = uid_drawBoard(g);
	c.y = 0;
	c.attached_Dir = 1;
}

bool IsCollision(int x, int y) {
	if (x >= 0 && x < W && y >= 0 && y < H) {
		return false;
	}
	return true;
}

void UpdatePos(vector<ITEM>& v) {
	for (int i = 1; i < (int)v.size(); ++i) {
		int px = v[i - 1].x;
		int py = v[i - 1].y;

		switch (v[i].attached_Dir) {
		case 1:
			v[i].x = px;
			v[i].y = py + 1;
			break;
		case 2:
			v[i].x = px - 1;
			v[i].y = py;
			break;
		case 3:
			v[i].x = px;
			v[i].y = py - 1;
			break;
		case 4:
			v[i].x = px + 1;
			v[i].y = py;
			break;
		}
	}
	return;
}

bool CanMove(const vector<ITEM>& items, int dx, int dy) {
	vector<ITEM> nextV = items;
	nextV[0].x += dx;
	nextV[0].y += dy;

	int hX = nextV[0].x;
	int hY = nextV[0].y;

	if (hX < 0 || hX >= W || hY < 0 || hY >= H) return false;

	if (board[hX][hY] >= 2) {
		ITEM last = nextV.back();

		if (items.size() == 1)
			last.attached_Dir = 3;
		else 
			last.attached_Dir = items.back().attached_Dir;

		nextV.push_back(last);
	}

	UpdatePos(nextV);

	for (int i = 0; i < (int)nextV.size(); ++i) {
		if (nextV[i].x < 0 || nextV[i].x >= W || nextV[i].y < 0 || nextV[i].y >= H)
			return false;
		if (i > 0 && board[nextV[i].x][nextV[i].y] >= 2)
			return false;
	}

	return true;
}

void ClearFullLine(int y, int& score) {
	int targetColor = board[0][y];
	if (targetColor < 2) return;

	for (int x = 0; x < W; ++x) {
		if (board[x][y] != targetColor) 
			return;
	}

	for (int x = 0; x < W; ++x) {
		board[x][y] = 0;
		item_count--;
	}

	for (int i = y; i > 0; --i) {
		for (int j = 0; j < W; ++j) {
			board[j][i] = board[j][i - 1];
		}
	}
	for (int j = 0; j < W; ++j) {
		board[j][0] = 0;
	}

	score++;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC;
	HPEN hPen, oldPen;
	HBRUSH hBrush, oldBrush;
	TCHAR str[50];
	static vector<ITEM> items;	// 0번은 주인공 원, 나머지는 붙은 아이템
	static RECT rt;
	static int speed = 200;
	static int mx, my;
	static bool auto_move = true;
	static int score = 0;

	switch (uMsg) {
	case WM_CREATE:
		ITEM head;
		init_setting(head);
		items.push_back(head);
		for (int i = 0; i < 60; i++) spawn_item(i);
		SetTimer(hWnd, 1, speed, NULL);
		break;
	case WM_KEYDOWN:
		if (wParam == VK_UP || wParam == VK_RIGHT || wParam == VK_DOWN || wParam == VK_LEFT) {
			int dx = 0, dy = 0;

			if (wParam == VK_UP) dy = -1;
			else if (wParam == VK_DOWN) dy = 1;
			else if (wParam == VK_LEFT) dx = -1;
			else if (wParam == VK_RIGHT) dx = 1;

			if (CanMove(items, dx, dy)) {
				items[0].x += dx;
				items[0].y += dy;

				int hX = items[0].x;
				int hY = items[0].y;

				if (board[hX][hY] >= 2) {
					ITEM nt = items.back();
					nt.color = board[hX][hY];

					if (items.size() == 1) {	// 습득 아이템 위치 디폴트
						nt.attached_Dir = 3;
					}

					items.push_back(nt);
					board[hX][hY] = 0;
				}
				UpdatePos(items);
				InvalidateRect(hWnd, NULL, FALSE);
			}
		}
		else if (wParam == VK_OEM_PLUS || wParam == VK_ADD) {
			if (speed > 100) {
				speed -= 10;
				KillTimer(hWnd, 1);
				SetTimer(hWnd, 1, speed, NULL);
			}

			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == VK_OEM_MINUS || wParam == VK_SUBTRACT) {
			if (speed < 300) {
				speed += 10;
				KillTimer(hWnd, 1);
				SetTimer(hWnd, 1, speed, NULL);
			}

			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == VK_RETURN) {
			vector<ITEM> nextV = items;
			for (int i = 1; i < (int)nextV.size(); ++i) {
				// 방향 전환: 1->2, 2->3, 3->4, 4->1
				nextV[i].attached_Dir = (nextV[i].attached_Dir % 4) + 1;
			}

			UpdatePos(nextV);

			bool canR = true;
			for (auto& it : nextV) {
				if (it.x < 0 || it.x >= W || it.y < 0 || it.y >= H || board[it.x][it.y] >= 2) {
					canR = false; 
					break;
				}
			}

			if (canR) items = nextV;

			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'R') {
			items.clear();

			for (int i = 0; i < W; ++i) {
				for (int j = 0; j < H; ++j) {
					board[i][j] = 0;
				}
			}

			ITEM head;
			init_setting(head);
			items.push_back(head);
			for (int i = 0; i < 60; i++) spawn_item(i);
			score = 0;

			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'W') {
			if (items[0].y == H - 1 || items[items.size() - 1].y == H - 1) {
				if (items[0].y == H - 1) {
					for (int i = 1; i < items.size(); ++i) {
						board[items[i].x][items[i].y + 1] = items[i].color;
					}
				}
				else {
					for (int i = 1; i < items.size(); ++i) {
						board[items[i].x][items[i].y] = items[i].color;
					}
				}
				items.erase(items.begin() + 1, items.end());

				items[0].x = uid_drawBoard(g);
				items[0].y = 0;
			}

			for (int y = 0; y < H; ++y) {
				ClearFullLine(y, score);
			}

			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'A') {
			auto_move = !auto_move;
		}
		else if (wParam == VK_ESCAPE || wParam == 'Q') {
			PostQuitMessage(0);
		}
		break;

	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);
		{
			// 백버퍼 생성
			HDC memDC = CreateCompatibleDC(hDC);
			HBITMAP hBit = CreateCompatibleBitmap(hDC, LEN, HEI);
			HBITMAP oldBit = (HBITMAP)SelectObject(memDC, hBit);

			// 배경 지우기 (더블 버퍼링의 핵심: 하얀색으로 백버퍼를 채움)
			FillRect(memDC, &ps.rcPaint, (HBRUSH)GetStockObject(WHITE_BRUSH));

			//-----------------------------기본 판 그리기
			hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
			oldPen = (HPEN)SelectObject(memDC, hPen);
			for (int i = 0; i < H; ++i) {
				for (int j = 0; j < W; ++j) {
					Rectangle(memDC, 20 + 20 * j, 20 + 20 * i, 40 + 20 * j, 40 + 20 * i);
				}
			}
			SelectObject(memDC, oldPen);
			DeleteObject(hPen);
			//--------------------------------아이템 그리기
			for (int i = 0; i < H; ++i) {
				for (int j = 0; j < W; ++j) {
					if (board[j][i] == 2) {
						hBrush = CreateSolidBrush(RGB(255, 0, 0));
						oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
						Rectangle(memDC, 20 + 20 * j, 20 + 20 * i, 40 + 20 * j, 40 + 20 * i);
						SelectObject(memDC, hBrush);
						DeleteObject(hBrush);
					}
					else if (board[j][i] == 3) {
						hBrush = CreateSolidBrush(RGB(0, 255, 0));
						oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
						Rectangle(memDC, 20 + 20 * j, 20 + 20 * i, 40 + 20 * j, 40 + 20 * i);
						SelectObject(memDC, hBrush);
						DeleteObject(hBrush);
					}
					else if (board[j][i] == 4) {
						hBrush = CreateSolidBrush(RGB(0, 0, 255));
						oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
						Rectangle(memDC, 20 + 20 * j, 20 + 20 * i, 40 + 20 * j, 40 + 20 * i);
						SelectObject(memDC, hBrush);
						DeleteObject(hBrush);
					}
					else if (board[j][i] == 5) {
						hBrush = CreateSolidBrush(RGB(255, 255, 102));
						oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
						Rectangle(memDC, 20 + 20 * j, 20 + 20 * i, 40 + 20 * j, 40 + 20 * i);
						SelectObject(memDC, hBrush);
						DeleteObject(hBrush);
					}
				}
			}
			//------------------------주인공과 꼬리원 그리기
			for (int i = 0; i < (int)items.size(); ++i) {
				int centerX = 20 + 20 * items[i].x;
				int centerY = 20 + 20 * items[i].y;

				if (items[i].color == 1) {
					hBrush = CreateSolidBrush(RGB(102, 255, 255));
					oldBrush = (HBRUSH)SelectObject(memDC, hBrush);

					if (i == 0)
						Ellipse(memDC, centerX - 4, centerY - 4, centerX + 24, centerY + 24);

					SelectObject(memDC, hBrush);
					DeleteObject(hBrush);
				}
				else if (items[i].color == 2) {
					hBrush = CreateSolidBrush(RGB(255, 0, 0));
					oldBrush = (HBRUSH)SelectObject(memDC, hBrush);

					Rectangle(memDC, centerX, centerY, centerX + 20, centerY + 20);

					SelectObject(memDC, hBrush);
					DeleteObject(hBrush);
				}
				else if (items[i].color == 3) {
					hBrush = CreateSolidBrush(RGB(0, 255, 0));
					oldBrush = (HBRUSH)SelectObject(memDC, hBrush);

					Rectangle(memDC, centerX, centerY, centerX + 20, centerY + 20);

					SelectObject(memDC, hBrush);
					DeleteObject(hBrush);
				}
				else if (items[i].color == 4) {
					hBrush = CreateSolidBrush(RGB(0, 0, 255));
					oldBrush = (HBRUSH)SelectObject(memDC, hBrush);

					Rectangle(memDC, centerX, centerY, centerX + 20, centerY + 20);

					SelectObject(memDC, hBrush);
					DeleteObject(hBrush);
				}
				else if (items[i].color == 5) {
					hBrush = CreateSolidBrush(RGB(255, 255, 102));
					oldBrush = (HBRUSH)SelectObject(memDC, hBrush);

					Rectangle(memDC, centerX, centerY, centerX + 20, centerY + 20);

					SelectObject(memDC, hBrush);
					DeleteObject(hBrush);
				}
				
			}

			SetBkMode(memDC, TRANSPARENT);
			SetTextColor(memDC, RGB(0, 0, 0));
			SetTextAlign(memDC, TA_TOP);
			_stprintf_s(str, L"Speed : %d", speed);
			TextOut(memDC, 30, 650, str, _tcslen(str));
			_stprintf_s(str, L"Item Count(Max 300) : %d", item_count);
			TextOut(memDC, 30, 680, str, _tcslen(str));
			_stprintf_s(str, L"Score : %d", score);
			TextOut(memDC, 30, 710, str, _tcslen(str));

			// 완성된 백버퍼를 실제 화면으로 한 번에 복사
			BitBlt(hDC, 0, 0, LEN, HEI, memDC, 0, 0, SRCCOPY);

			SelectObject(memDC, oldBit);
			DeleteObject(hBit);
			DeleteDC(memDC);
		}
		EndPaint(hWnd, &ps);
		break;
	case WM_ERASEBKGND:
		return 1;
	case WM_LBUTTONDOWN:
		mx = LOWORD(lParam);
		my = HIWORD(lParam);

		{
			int boardX = (mx - 20) / 20;
			int boardY = (my - 20) / 20;

			if ((boardX >= 0 && boardX < W && boardY >= 0 && boardY < H) && (board[boardX][boardY] == 0) && item_count < 300) {
				//board[boardX][boardY] = uid_color(g);
				board[boardX][boardY] = 2;		// 줄 사라짐 테스트용
				item_count++;
			}
		}

		InvalidateRect(hWnd, NULL, FALSE);
		break;
	case WM_RBUTTONDOWN:
		mx = LOWORD(lParam);
		my = HIWORD(lParam);

		{
			int boardX = (mx - 20) / 20;
			int boardY = (my - 20) / 20;

			if ((boardX >= 0 && boardX < W && boardY >= 0 && boardY < H) && (board[boardX][boardY] >= 2)) {
				board[boardX][boardY] = 0;
				item_count--;
			}
		}

		InvalidateRect(hWnd, NULL, FALSE);
		break;
	case WM_TIMER:
	{
		if (items[0].y == H - 1 || items[items.size() - 1].y == H - 1) {
			if (items[items.size() - 1].y != H - 1) {
				for (int i = 1; i < items.size(); ++i) {
					board[items[i].x][items[i].y + 1] = items[i].color;
				}
			}
			else {
				for (int i = 1; i < items.size(); ++i) {
					board[items[i].x][items[i].y] = items[i].color;
				}
			}
			items.erase(items.begin() + 1, items.end());

			items[0].x = uid_drawBoard(g);
			items[0].y = 0;
		}

		for (int y = 0; y < H; ++y) {
			ClearFullLine(y, score);
		}

		for (int y = 0; y < H; ++y) {
			ClearFullLine(y, score);
		}

		int dx = 0, dy = 0;
		if(auto_move)
			dy += 1;

		if (CanMove(items, dx, dy)) {
			items[0].x += dx;
			items[0].y += dy;

			int hX = items[0].x;
			int hY = items[0].y;

			if (board[hX][hY] >= 2) {
				ITEM nt = items.back();
				nt.color = board[hX][hY];

				if ((int)items.size() == 1) {
					nt.attached_Dir = 3;	// 습득 아이템 위치 디폴트
				}

				items.push_back(nt);
				board[hX][hY] = 0;
			}
			UpdatePos(items);
		}

		InvalidateRect(hWnd, NULL, FALSE);
		UpdateWindow(hWnd);
		break;
	}
	case WM_DESTROY:
		KillTimer(hWnd, 1);
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}