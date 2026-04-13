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
#define HEI 600

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

//void CALLBACK TimerProc(HWND hWnd, UINT iMsg, UINT idEvent, DWORD dwTime)
//{
//	HDC hDC;
//	RECT rt;
//
//	hDC = GetDC(hWnd);
//	GetClientRect(hWnd, &rt);
//}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC;
	HPEN hPen, oldPen;
	HBRUSH hBrush, oldBrush;
	TCHAR str[50];
	static RECT rt;
	static int centerX, centerY, mx, my;
	static int shape = 0;	// 0 사각형 / 1 원 / 2 삼각형
	static int print_shape = 0;
	static POINT triangle[3] = { {0, -50}, {-50, 50}, {50, 50} };
	static bool mouseClicked = false;
	static int speed = 20;
	static bool move_right = true;
	static bool move_down = true;
	static int active_direction = 0;
	static bool pause = true;

	static float angle = 0.0f;

	switch (uMsg) {
	case WM_CREATE:
		
		break;
	case WM_KEYDOWN:
		if (wParam == 'R') {
			shape = 0;
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == 'E') {
			shape = 1;
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == 'T') {
			shape = 2;
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == 'H') {
			if (active_direction != 0) 
				KillTimer(hWnd, active_direction);
			move_right = true;
			pause = true;
			active_direction = 1;
			SetTimer(hWnd, active_direction, speed, NULL);

			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == 'V') {
			if (active_direction != 0) 
				KillTimer(hWnd, active_direction);
			move_down = true;
			pause = true;
			active_direction = 2;
			SetTimer(hWnd, active_direction, speed, NULL);

			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == 'S') {
			if (active_direction != 0) 
				KillTimer(hWnd, active_direction);
			move_right = true;
			move_down = true;
			pause = true;
			active_direction = 3;
			SetTimer(hWnd, active_direction, speed, NULL);

			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == 'P') {
			if(pause) {
				KillTimer(hWnd, active_direction);
				pause = false;
			}
			else {
				SetTimer(hWnd, active_direction, speed, NULL);
				pause = true;
			}

			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == VK_ESCAPE || wParam == 'Q') {
			PostQuitMessage(0);
		}
		else if (wParam == VK_OEM_PLUS || wParam == VK_ADD) {
			if (speed > 10)
				speed--;
			if (active_direction == 1) {
				KillTimer(hWnd, 1);
				SetTimer(hWnd, 1, speed, NULL);
			}
			else if (active_direction == 2) {
				KillTimer(hWnd, 2);
				SetTimer(hWnd, 2, speed, NULL);
			}
			else if (active_direction == 3) {
				KillTimer(hWnd, 3);
				SetTimer(hWnd, 3, speed, NULL);
			}
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == VK_OEM_MINUS || wParam == VK_SUBTRACT) {
			if (speed < 40)
				speed++;
			if (active_direction == 1) {
				KillTimer(hWnd, 1);
				SetTimer(hWnd, 1, speed, NULL);
			}
			else if (active_direction == 2) {
				KillTimer(hWnd, 2);
				SetTimer(hWnd, 2, speed, NULL);
			}
			else if (active_direction == 3) {
				KillTimer(hWnd, 3);
				SetTimer(hWnd, 3, speed, NULL);
			}
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == 'A') {
			if (active_direction != 0)
				KillTimer(hWnd, active_direction);
			pause = true;
			active_direction = 4;
			SetTimer(hWnd, active_direction, speed, NULL);
		}
		break;


	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);
		
		hBrush = CreateSolidBrush(RGB(255, 153, 255));
		oldBrush = (HBRUSH)SelectObject(hDC, hBrush);

		SetBkMode(hDC, TRANSPARENT);

		SetTextAlign(hDC, TA_LEFT | TA_TOP);
		if (shape == 0) _stprintf_s(str, L"출력될 도형 : 사각형");
		else if (shape == 1) _stprintf_s(str, L"출력될 도형 : 원");
		else if (shape == 2) _stprintf_s(str, L"출력될 도형 : 삼각형");

		TextOut(hDC, 10, 10, str, _tcslen(str));

		if(mouseClicked)
		{
			if (print_shape == 0) {
				Rectangle(hDC, mx - 50, my - 50, mx + 50, my + 50);
			}
			else if (print_shape == 1) {
				Ellipse(hDC, mx - 50, my - 50, mx + 50, my + 50);
			}
			else if (print_shape == 2) {
				triangle[0].x = mx;
				triangle[0].y = my - 50;
				triangle[1].x = mx - 50;
				triangle[1].y = my + 50;
				triangle[2].x = mx + 50;
				triangle[2].y = my + 50;
				Polygon(hDC, triangle, 3);
			}
			SetBkMode(hDC, TRANSPARENT);

			SetTextAlign(hDC, TA_CENTER | TA_BASELINE);
			_stprintf_s(str, L"Speed : %d", speed);
			TextOut(hDC, mx, my, str, _tcslen(str));
		}

		SelectObject(hDC, oldBrush);
		DeleteObject(hBrush);

		EndPaint(hWnd, &ps);
		break;

	case WM_LBUTTONDOWN:
		mx = LOWORD(lParam);
		my = HIWORD(lParam);

		GetClientRect(hWnd, &rt);

		if (mx < 50) mx = 50;
		if (my < 50) my = 50;
		if (mx > rt.right - 50) mx = rt.right - 50;
		if (my > rt.bottom - 50) my = rt.bottom - 50;

		print_shape = shape;
		mouseClicked = true;

		InvalidateRect(hWnd, NULL, TRUE);
		break;
	case WM_TIMER:
		GetClientRect(hWnd, &rt);
		switch (wParam) {
		case 1:
			if (move_right) {
				if (mx + 4 > rt.right - 50) {
					move_right = false;
					mx -= 4;
					if (my + 50 > rt.bottom - 50 && my != rt.bottom - 50)
						my = rt.bottom - 50;
					else if (my == rt.bottom - 50) {
						mx = 50;
						my = 50;
						move_right = true;
					}
					else
						my += 50;
				}
				else
					mx += 4;
			}
			else {
				if (mx - 4 < 50) {
					move_right = true;
					mx += 4;
					if (my + 50 > rt.bottom - 50 && my != rt.bottom - 50)
						my = rt.bottom - 50;
					else if (my == rt.bottom - 50) {
						mx = 50;
						my = 50;
						move_right = true;
					}
					else
						my += 50;
				}
				else
					mx -= 4;
			}
			break;
		case 2:
			if (move_down) {
				if (my + 3 > rt.bottom - 50) {
					move_down = false;
					my -= 3;
					if (mx + 50 > rt.right - 50 && mx != rt.right - 50)
						mx = rt.right - 50;
					else if (mx == rt.right - 50) {
						mx = 50;
						my = 50;
						move_down = true;
					}
					else
						mx += 50;
				}
				else
					my += 3;
			}
			else {
				if (my - 3 < 50) {
					move_down = true;
					my += 3;
					if (mx + 50 > rt.right - 50 && mx != rt.right - 50)
						mx = rt.right - 50;
					else if (mx == rt.right - 50) {
						mx = 50;
						my = 50;
						move_down = true;
					}
					else
						mx += 50;
				}
				else
					my -= 3;
			}
			break;
		case 3:
			if (move_right) {
				if (mx + 4 > rt.right - 50) {
					move_right = false;
					mx -= 4;
				}
				else
					mx += 4;
			}
			else {
				if (mx - 4 < 50) {
					move_right = true;
					mx += 4;
				}
				else
					mx -= 4;
			}
			if (move_down) {
				if (my + 3 > rt.bottom - 50) {
					move_down = false;
					my -= 3;
				}
				else
					my += 3;
			}
			else {
				if (my - 3 < 50) {
					move_down = true;
					my += 3;
				}
				else
					my -= 3;
			}
			break;
		case 4:
			angle += 0.05f;

			mx = (int)(400 + 200 * cos(angle));
			my = (int)(300 + 200 * sin(angle));

			if (angle > 6.28318f) angle = 0.0f;
			break;
		}

		InvalidateRect(hWnd, NULL, TRUE);
		UpdateWindow(hWnd);
		break;
	case WM_DESTROY:
		for(int i = 1; i <= 3; ++i)
			KillTimer(hWnd, i);
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


