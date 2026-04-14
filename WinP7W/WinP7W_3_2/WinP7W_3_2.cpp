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
#define LEN 800
#define HEI 700

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
	int centerX;
	int centerY;
	int moving_shape;
	int orbit;
	int speed;
	int mx;
	int my;
	float angle;
	bool reverse;
	bool right_clicked;
	bool clockwise;
	int rect_dire;
	int tri_dire;
	COLORREF color;
} SECTORS;

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC;
	HPEN hPen, oldPen;
	HBRUSH hBrush, oldBrush;
	TCHAR str[50];
	static RECT rt;
	static int mx, my;
	static vector<SECTORS> sectors(4);
	static int selected = -1;
	static int r = 140;
	

	static float angle = 0.0f;

	switch (uMsg) {
	case WM_CREATE:
		sectors[0].centerX = 200;
		sectors[0].centerY = 150;
		sectors[0].moving_shape = 0;
		sectors[0].orbit = 0;
		sectors[0].speed = 20;
		sectors[0].mx = sectors[0].centerX;
		sectors[0].my = sectors[0].centerY - r;
		sectors[0].reverse = false;
		sectors[0].right_clicked = false;
		sectors[0].clockwise = true;
		sectors[0].rect_dire = 0;
		sectors[0].tri_dire = 0;
		sectors[0].angle = -1.570796f;
		sectors[0].color = RGB(uid_rgb(g), uid_rgb(g), uid_rgb(g));

		sectors[1].centerX = 600;
		sectors[1].centerY = 150;
		sectors[1].moving_shape = 0;
		sectors[1].orbit = 0;
		sectors[1].speed = 20;
		sectors[1].mx = sectors[1].centerX;
		sectors[1].my = sectors[1].centerY - r;
		sectors[1].reverse = false;
		sectors[1].right_clicked = false;
		sectors[1].clockwise = false;
		sectors[1].rect_dire = 0;
		sectors[1].tri_dire = 0;
		sectors[1].angle = -1.570796f;
		sectors[1].color = RGB(uid_rgb(g), uid_rgb(g), uid_rgb(g));

		sectors[2].centerX = 200;
		sectors[2].centerY = 450;
		sectors[2].moving_shape = 0;
		sectors[2].orbit = 0;
		sectors[2].speed = 20;
		sectors[2].mx = sectors[2].centerX;
		sectors[2].my = sectors[2].centerY - r;
		sectors[2].reverse = false;
		sectors[2].right_clicked = false;
		sectors[2].clockwise = false;
		sectors[2].rect_dire = 0;
		sectors[2].tri_dire = 0;
		sectors[2].angle = -1.570796f;
		sectors[2].color = RGB(uid_rgb(g), uid_rgb(g), uid_rgb(g));

		sectors[3].centerX = 600;
		sectors[3].centerY = 450;
		sectors[3].moving_shape = 0;
		sectors[3].orbit = 0;
		sectors[3].speed = 20;
		sectors[3].mx = sectors[3].centerX;
		sectors[3].my = sectors[3].centerY - r;
		sectors[3].reverse = false;
		sectors[3].right_clicked = false;
		sectors[3].clockwise = true;
		sectors[3].rect_dire = 0;
		sectors[3].tri_dire = 0;
		sectors[3].angle = -1.570796f;
		sectors[3].color = RGB(uid_rgb(g), uid_rgb(g), uid_rgb(g));

		SetTimer(hWnd, 0, sectors[0].speed, NULL);
		SetTimer(hWnd, 1, sectors[1].speed, NULL);
		SetTimer(hWnd, 2, sectors[2].speed, NULL);
		SetTimer(hWnd, 3, sectors[3].speed, NULL);
		SetTimer(hWnd, 4, 3000, NULL);		// 색상 변경 타이머
		break;
	case WM_KEYDOWN:
		if (wParam == '1') {
			if (selected != 0)
				selected = 0;
			else
				selected = -1;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == '2') {
			if (selected != 1)
				selected = 1;
			else
				selected = -1;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == '3') {
			if (selected != 2)
				selected = 2;
			else
				selected = -1;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == '4') {
			if (selected != 3)
				selected = 3;
			else
				selected = -1;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'C') {
			if (selected != -1) {
				sectors[selected].clockwise = !sectors[selected].clockwise;
			}
			KillTimer(hWnd, selected);
			SetTimer(hWnd, selected, sectors[selected].speed, NULL);
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'M') {
			if (selected != -1) {
				if (sectors[selected].moving_shape == 0)
					sectors[selected].moving_shape = 1;
				else
					sectors[selected].moving_shape = 0;
			}
			KillTimer(hWnd, selected);
			SetTimer(hWnd, selected, sectors[selected].speed, NULL);
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'R') {
			if (selected != -1) {
				if (sectors[selected].orbit != 0) {
					sectors[selected].orbit = 0;
					sectors[selected].angle = -1.570796f;
				}
				else
					sectors[selected].orbit = 1;
			}
			sectors[selected].rect_dire = 0;
			sectors[selected].tri_dire = 0;
			sectors[selected].mx = sectors[selected].centerX;
			sectors[selected].my = sectors[selected].centerY - r;
			KillTimer(hWnd, selected);
			SetTimer(hWnd, selected, sectors[selected].speed, NULL);
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'T') {
			if (selected != -1) {
				if (sectors[selected].orbit != 0) {
					sectors[selected].orbit = 0;
					sectors[selected].angle = -1.570796f;
				}
				else
					sectors[selected].orbit = 2;
			}
			sectors[selected].rect_dire = 0;
			sectors[selected].tri_dire = 0;
			sectors[selected].mx = sectors[selected].centerX;
			sectors[selected].my = sectors[selected].centerY - r;
			KillTimer(hWnd, selected);
			SetTimer(hWnd, selected, sectors[selected].speed, NULL);
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
				if(selected != -1) {
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
		if(selected != -1) {
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