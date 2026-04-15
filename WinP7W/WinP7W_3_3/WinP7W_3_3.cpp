#include <windows.h>
#include <tchar.h>
#include <random>
#include <vector>
#include <math.h>

using namespace std;
random_device rd;
mt19937 g(rd());
uniform_int_distribution<> uid_shape{ 0, 2 };
uniform_int_distribution<> uid_rgb{ 0, 255 };
uniform_int_distribution<> uid_drawBoard{ 0, 39 };
uniform_int_distribution<> uid_size{ -3, 3 };
#define LEN 100
#define HEI 100

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

typedef struct {
	COLORREF color;
	float r;
	int dire;
	bool shape; // true 원 false 삼각형
	bool jump;
	int centerX;
	int centerY;
	int pos;	// 원의 순서
	int movemode;
} CIRCLE;

void circle_move(CIRCLE c) {

}

void tailCircle_move(CIRCLE c) {

}

void init_setting(CIRCLE c) {
	c.color = uid_rgb(g);
	c.size = 10
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC;
	HPEN hPen, oldPen;
	HBRUSH hBrush, oldBrush;
	TCHAR str[50];
	static vector<CIRCLE> circles(20);
	static vector<CIRCLE> tail_circles(20);
	static RECT rt;
	static int speed = 500;
	static int mx, my;
	static int board[40][40] = {0};

	switch (uMsg) {
	case WM_CREATE:
		init_setting(circles[0]);
		break;
	case WM_KEYDOWN:
		if (wParam == 'S') {
			
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == VK_UP) {
			
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == VK_RIGHT) {
			
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == VK_DOWN) {
			
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == VK_LEFT) {

			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == VK_OEM_PLUS || wParam == VK_ADD) {

			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == VK_OEM_MINUS || wParam == VK_SUBTRACT) {

			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'J') {

			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'T') {

			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'A') {

			InvalidateRect(hWnd, NULL, FALSE);
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
			for (int i = 0; i < 40; ++i) {
				for (int j = 0; j < 40; ++j) {
					Rectangle(hDC, 20 + 20 * j, 20 + 20 * i, 40 + 20 * j, 40 + 20 * i);
				}
			}
			SelectObject(memDC, oldPen);
			DeleteObject(hPen);

			for (int i = 0; i < 4; ++i) {
				//---------------------------------------------------
				if (!sectors[i].reverse) {
					hPen = CreatePen(PS_SOLID, 2, RGB(102, 255, 255));
					oldPen = (HPEN)SelectObject(memDC, hPen);
				}
				else {
					hPen = CreatePen(PS_SOLID, 2, RGB(153, 0, 0));
					oldPen = (HPEN)SelectObject(memDC, hPen);
				}
				if (sectors[i].orbit == 0) {
					Ellipse(memDC, sectors[i].centerX - r, sectors[i].centerY - r,
						sectors[i].centerX + r, sectors[i].centerY + r);
				}
				else if (sectors[i].orbit == 1) {
					Rectangle(memDC, sectors[i].centerX - r - 4, sectors[i].centerY - r,
						sectors[i].centerX + r + 5, sectors[i].centerY + r + 2);
				}
				else if (sectors[i].orbit == 2) {
					POINT tri[3] = { {sectors[i].centerX , sectors[i].centerY - r}, {sectors[i].centerX - r , sectors[i].centerY + r}, {sectors[i].centerX + r , sectors[i].centerY + r} };
					Polygon(memDC, tri, 3);
				}
				SelectObject(memDC, oldPen);
				DeleteObject(hPen);
				//---------------------------------------------------
				if (!sectors[i].reverse) {
					hBrush = CreateSolidBrush(RGB(255, 0, 0));
					oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
				}
				else {
					hBrush = CreateSolidBrush(RGB(0, 255, 255));
					oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
				}
				Ellipse(memDC, sectors[i].centerX - 5, sectors[i].centerY - 5, sectors[i].centerX + 5, sectors[i].centerY + 5);
				SelectObject(memDC, oldBrush);
				DeleteObject(hBrush);
				//---------------------------------------------------			
				if (i == selected && sectors[i].reverse == true) {
					hBrush = CreateSolidBrush(RGB(255 - GetRValue(sectors[i].color), 255 - GetGValue(sectors[i].color), 255 - GetBValue(sectors[i].color)));
					oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
				}
				else {
					hBrush = CreateSolidBrush(sectors[i].color);
					oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
				}
				if (sectors[i].moving_shape == 0) {
					Ellipse(memDC, sectors[i].mx - 10, sectors[i].my - 10, sectors[i].mx + 10, sectors[i].my + 10);
				}
				else {
					Rectangle(memDC, sectors[i].mx - 10, sectors[i].my - 10, sectors[i].mx + 10, sectors[i].my + 10);
				}
				SelectObject(memDC, oldBrush);
				DeleteObject(hBrush);
				//---------------------------------------------------
				if (selected != -1) {
					hPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
					oldPen = (HPEN)SelectObject(memDC, hPen);

					SelectObject(memDC, GetStockObject(NULL_BRUSH));

					Rectangle(memDC, sectors[selected].centerX - 150, sectors[selected].centerY - 150,
						sectors[selected].centerX + 150, sectors[selected].centerY + 150);

					SelectObject(memDC, oldPen);
					DeleteObject(hPen);
				}
			}
			if (selected != -1) {
				SetBkMode(memDC, TRANSPARENT);
				SetTextColor(memDC, RGB(0, 0, 0));
				SetTextAlign(memDC, TA_CENTER | TA_TOP);
				_stprintf_s(str, L"Speed : %d", sectors[selected].speed);
				TextOut(memDC, 400, 300, str, _tcslen(str));
			}

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
		if (selected != -1) {
			if (sectors[selected].speed > 10)
				sectors[selected].speed--;
			else if (sectors[selected].speed == 10)
				sectors[selected].speed = 40;
			KillTimer(hWnd, selected);
			SetTimer(hWnd, selected, sectors[selected].speed, NULL);

			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;
	case WM_RBUTTONDOWN:
		if (selected != -1) {
			int oldCenterX = sectors[selected].centerX;
			int oldCenterY = sectors[selected].centerY;

			if (!sectors[selected].right_clicked) {
				sectors[selected].centerX = LOWORD(lParam);
				sectors[selected].centerY = HIWORD(lParam);
				sectors[selected].right_clicked = true;
			}
			else {
				if (selected == 0) {
					sectors[selected].centerX = 200;
					sectors[selected].centerY = 150;
					sectors[selected].right_clicked = false;
				}
				else if (selected == 1) {
					sectors[selected].centerX = 600;
					sectors[selected].centerY = 150;
					sectors[selected].right_clicked = false;
				}
				else if (selected == 2) {
					sectors[selected].centerX = 200;
					sectors[selected].centerY = 450;
					sectors[selected].right_clicked = false;
				}
				else if (selected == 3) {
					sectors[selected].centerX = 600;
					sectors[selected].centerY = 450;
					sectors[selected].right_clicked = false;
				}
			}
			int offsetX = sectors[selected].centerX - oldCenterX;
			int offsetY = sectors[selected].centerY - oldCenterY;

			sectors[selected].mx += offsetX;
			sectors[selected].my += offsetY;

			InvalidateRect(hWnd, NULL, FALSE);
		}

		break;
	case WM_RBUTTONDBLCLK:
		sectors[selected].reverse = !sectors[selected].reverse;
		break;
	case WM_TIMER:
		switch (wParam) {
		case 0:
		case 1:
		case 2:
		case 3:
			if (sectors[wParam].clockwise == true) {
				if (sectors[wParam].orbit == 0) {
					sectors[wParam].angle += 0.05f;
					sectors[wParam].mx = (int)(sectors[wParam].centerX + r * cos(sectors[wParam].angle));
					sectors[wParam].my = (int)(sectors[wParam].centerY + r * sin(sectors[wParam].angle));
					if (sectors[wParam].angle > 6.28318f) sectors[wParam].angle = 0.0f;
				}
				else if (sectors[wParam].orbit == 1) {
					if (sectors[wParam].rect_dire == 0) {
						if (sectors[wParam].mx >= sectors[wParam].centerX + r) {
							sectors[wParam].rect_dire = 1;
							sectors[wParam].my += 6;
						}
						else sectors[wParam].mx += 6;
					}
					else if (sectors[wParam].rect_dire == 1) {
						if (sectors[wParam].my >= sectors[wParam].centerY + r) {
							sectors[wParam].rect_dire = 2;
							sectors[wParam].mx -= 6;
						}
						else sectors[wParam].my += 6;
					}
					else if (sectors[wParam].rect_dire == 2) {
						if (sectors[wParam].mx <= sectors[wParam].centerX - r) {
							sectors[wParam].rect_dire = 3;
							sectors[wParam].my -= 6;
						}
						else sectors[wParam].mx -= 6;
					}
					else if (sectors[wParam].rect_dire == 3) {
						if (sectors[wParam].my <= sectors[wParam].centerY - r) {
							sectors[wParam].rect_dire = 0;
							sectors[wParam].mx += 6;
						}
						else sectors[wParam].my -= 6;
					}
				}
				else if (sectors[wParam].orbit == 2) {
					if (sectors[wParam].tri_dire == 0) {
						if (sectors[wParam].mx >= sectors[wParam].centerX + r && sectors[wParam].my >= sectors[wParam].centerY + r) {
							sectors[wParam].tri_dire = 1;
							sectors[wParam].mx -= 6;
						}
						else {
							sectors[wParam].mx += 3;
							sectors[wParam].my += 6;
						}
					}
					else if (sectors[wParam].tri_dire == 1) {
						if (sectors[wParam].mx <= sectors[wParam].centerX - r) {
							sectors[wParam].tri_dire = 2;
							sectors[wParam].mx += 3;
							sectors[wParam].my -= 6;
						}
						else sectors[wParam].mx -= 6;
					}
					else if (sectors[wParam].tri_dire == 2) {
						if (sectors[wParam].mx >= sectors[wParam].centerX && sectors[wParam].my <= sectors[wParam].centerY - r) {
							sectors[wParam].tri_dire = 0;
							sectors[wParam].mx += 3;
							sectors[wParam].my += 6;
						}
						else {
							sectors[wParam].mx += 3;
							sectors[wParam].my -= 6;
						}
					}
				}
			}
			else {
				if (sectors[wParam].orbit == 0) {
					sectors[wParam].angle -= 0.05f;
					sectors[wParam].mx = (int)(sectors[wParam].centerX + r * cos(sectors[wParam].angle));
					sectors[wParam].my = (int)(sectors[wParam].centerY + r * sin(sectors[wParam].angle));
					if (sectors[wParam].angle < 0.0f) sectors[wParam].angle = 6.28318f;
				}
				else if (sectors[wParam].orbit == 1) {
					if (sectors[wParam].rect_dire == 0) {
						if (sectors[wParam].mx <= sectors[wParam].centerX - r) {
							sectors[wParam].rect_dire = 1;
							sectors[wParam].my += 6;
						}
						else sectors[wParam].mx -= 6;
					}
					else if (sectors[wParam].rect_dire == 1) {
						if (sectors[wParam].my >= sectors[wParam].centerY + r) {
							sectors[wParam].rect_dire = 2;
							sectors[wParam].mx += 6;
						}
						else sectors[wParam].my += 6;
					}
					else if (sectors[wParam].rect_dire == 2) {
						if (sectors[wParam].mx >= sectors[wParam].centerX + r) {
							sectors[wParam].rect_dire = 3;
							sectors[wParam].my -= 6;
						}
						else sectors[wParam].mx += 6;
					}
					else if (sectors[wParam].rect_dire == 3) {
						if (sectors[wParam].my <= sectors[wParam].centerY - r) {
							sectors[wParam].rect_dire = 0;
							sectors[wParam].mx -= 6;
						}
						else sectors[wParam].my -= 6;
					}
				}
				else if (sectors[wParam].orbit == 2) {
					if (sectors[wParam].tri_dire == 0) {
						if (sectors[wParam].mx <= sectors[wParam].centerX - r && sectors[wParam].my >= sectors[wParam].centerY + r) {
							sectors[wParam].tri_dire = 1;
							sectors[wParam].mx += 6;
						}
						else {
							sectors[wParam].mx -= 3;
							sectors[wParam].my += 6;
						}
					}
					else if (sectors[wParam].tri_dire == 1) {
						if (sectors[wParam].mx >= sectors[wParam].centerX + r) {
							sectors[wParam].tri_dire = 2;
							sectors[wParam].mx -= 3;
							sectors[wParam].my -= 6;
						}
						else sectors[wParam].mx += 6;
					}
					else if (sectors[wParam].tri_dire == 2) {
						if (sectors[wParam].mx <= sectors[wParam].centerX && sectors[wParam].my <= sectors[wParam].centerY - r) {
							sectors[wParam].tri_dire = 0;
							sectors[wParam].mx -= 3;
							sectors[wParam].my += 6;
						}
						else {
							sectors[wParam].mx -= 3;
							sectors[wParam].my -= 6;
						}
					}
				}
			}
			break;
		}
		if (wParam == 4) {
			for (int i = 0; i < 4; ++i) {
				sectors[i].color = RGB(uid_rgb(g), uid_rgb(g), uid_rgb(g));
			}
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		}
		InvalidateRect(hWnd, NULL, TRUE);
		UpdateWindow(hWnd);
		break;
	case WM_DESTROY:
		for (int i = 0; i < 4; ++i)
			KillTimer(hWnd, i);
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}