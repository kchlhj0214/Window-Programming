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

static int closed_board[10][10] = {0};
static int opened_board[10][10] = {0};

void init_setting()
{
	int x, y;
	for (int i = 0; i < pie_num * 4; ++i) {		// 파이 생성
		do {
			x = uid_borad_len(g);
			y = uid_borad_hei(g);
		} while (closed_board[x][y] != 0);
		closed_board[x][y] = i + 3;
	}

	for (int i = 0; i < mine_num; ++i) {		// 지뢰 생성
		do {
			x = uid_borad_len(g);
			y = uid_borad_hei(g);
		} while (closed_board[x][y] != 0);
		closed_board[x][y] = 1;
	}

	for (int i = 0; i < item_num; ++i) {		// 지뢰 생성
		do {
			x = uid_borad_len(g);
			y = uid_borad_hei(g);
		} while (closed_board[x][y] != 0);
		closed_board[x][y] = 2;
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

	static int score = 0;
	static bool show_score = false;

	switch (uMsg) {
	case WM_CREATE:
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
		{
			// 백버퍼 생성
			HDC memDC = CreateCompatibleDC(hDC);
			HBITMAP hBit = CreateCompatibleBitmap(hDC, rt.right, rt.bottom);
			HBITMAP oldBit = (HBITMAP)SelectObject(memDC, hBit);

			// 배경 지우기 (더블 버퍼링의 핵심: 하얀색으로 백버퍼를 채움)
			FillRect(memDC, &ps.rcPaint, (HBRUSH)GetStockObject(WHITE_BRUSH));

			

			if(show_score) {
				SetBkMode(memDC, TRANSPARENT);
				SetTextColor(memDC, RGB(0, 0, 0));
				SetTextAlign(memDC, TA_TOP);
				_stprintf_s(str, L"Score : %d", score);
				TextOut(memDC, 30, 700, str, _tcslen(str));
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
			

			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case ID_MENU_HINT:
			
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case ID_MENU_SCORE:
			
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		}
		break;
	case WM_ERASEBKGND:
		return 1;
	case WM_LBUTTONDOWN:
		if (light_mode != 2) {
			light_mode = 2;
			traffic_lights[0].state = 0;
			traffic_lights[1].state = 0;
		}
		set_man_target(man, traffic_lights, light_mode);

		InvalidateRect(hWnd, NULL, FALSE);
		break;
	case WM_RBUTTONDOWN:
		if (light_mode == 2) {
			light_mode = 0;
			traffic_lights[0].remain_time = blue_time;
			traffic_lights[0].state = 2;
			traffic_lights[1].remain_time = red_time;
			traffic_lights[1].state = 0;
			ttime = light_time;
		}

		InvalidateRect(hWnd, NULL, FALSE);
		break;
	case WM_TIMER:
		GetClientRect(hWnd, &rt);

		if (wParam >= 0 && wParam <= 7) {
			car_move((int)wParam, cars, traffic_lights, rt, crossing);
		}
		else if (wParam == 8) {
			light_timer(traffic_lights, ttime, light_mode);
			if (traffic_lights[0].remain_time == blue_time - yellow_time || traffic_lights[1].remain_time == blue_time - yellow_time) {
				if (man.isMoving == false)
					set_man_target(man, traffic_lights, light_mode);
			}
		}
		else if (wParam == 9) {
			man_move(man, traffic_lights, crossing);
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