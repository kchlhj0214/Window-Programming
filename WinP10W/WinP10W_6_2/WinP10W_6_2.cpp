#include <windows.h>
#include <tchar.h>
#include <random>
#include <vector>
#include <math.h>
#include <algorithm>
#include <atlimage.h>
#include "resource.h"

#define LEN 1000
#define HEI 1000

using namespace std;
random_device rd;
mt19937 g(rd());
uniform_int_distribution<> uid_rgb{ 0, 255 };
uniform_int_distribution<> disX{ 50, LEN - 50 };
uniform_int_distribution<> disY{ 50, HEI - 50 };
uniform_int_distribution<> disTime{ 1000, 3000 };

int rgb_r = 0;
int rgb_g = 0;
int rgb_b = 0;

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"My Window Class";
LPCTSTR lpszWindowName = L"windows program 2";

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK Dlalog_Proc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

enum DrawMode { MODE_NONE, MODE_DRAW_LINE };
DrawMode g_CurrentMode = MODE_NONE;

POINT g_CenterPt = { 60, 60 };
int g_ShapeSize = 10;
bool isRect = true;
BOOL g_IsMoving = FALSE;
int speed = 35;
bool isGrid = true;

double g_CurrentX = 60.0;
double g_CurrentY = 60.0;
double g_SpeedX = 0.0;
double g_SpeedY = 0.0;

// 마우스 드래그 및 선 정보
BOOL g_isDragging = FALSE;
POINT g_LineStart = { 60, 60 };
POINT g_LineEnd = { 60, 60 };
BOOL g_hasLine = FALSE;

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
	while (GetMessage(&Message, NULL, 0, 0))
	{
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
	TCHAR str[50];
	static RECT rt;
	static int mx, my;


	switch (uMsg) {
	case WM_CREATE:
		
		break;
	case WM_KEYDOWN:
		if (wParam == VK_OEM_PLUS || wParam == VK_ADD) {
			if (selected == -1) {
				for (int i = 0; i < 8; ++i) {
					if (cars[i].speed > 10) {
						cars[i].speed -= 5;
						KillTimer(hWnd, i);
						SetTimer(hWnd, i, cars[i].speed, NULL);
					}
				}
			}
			else {
				if (cars[selected - 1].speed > 10) {
					cars[selected - 1].speed -= 5;
					KillTimer(hWnd, selected - 1);
					SetTimer(hWnd, selected - 1, cars[selected - 1].speed, NULL);
				}
			}

			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == VK_OEM_MINUS || wParam == VK_SUBTRACT) {
			if (selected == -1) {
				for (int i = 0; i < 8; ++i) {
					if (cars[i].speed < 30) {
						cars[i].speed += 5;
						KillTimer(hWnd, i);
						SetTimer(hWnd, i, cars[i].speed, NULL);
					}
				}
			}
			else {
				if (cars[selected - 1].speed < 30) {
					cars[selected - 1].speed += 5;
					KillTimer(hWnd, selected - 1);
					SetTimer(hWnd, selected - 1, cars[selected - 1].speed, NULL);
				}
			}

			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'A') {
			if (light_mode == 0)
				light_mode = 1;
			else if (light_mode == 1)
				light_mode = 0;
		}
		else if (wParam == VK_ESCAPE || wParam == 'Q') {
			PostQuitMessage(0);
		}
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
		{
			GetClientRect(hWnd, &rt);
			int v_w = rt.right;
			if (v_w < 800) v_w = 800;

			int v_h = rt.bottom;
			if (v_h < 600) v_h = 600;
			// 백버퍼 생성
			HDC memDC = CreateCompatibleDC(hDC);
			HBITMAP hBit = CreateCompatibleBitmap(hDC, rt.right, rt.bottom);
			HBITMAP oldBit = (HBITMAP)SelectObject(memDC, hBit);

			// 배경 지우기 (더블 버퍼링의 핵심: 하얀색으로 백버퍼를 채움)
			FillRect(memDC, &ps.rcPaint, (HBRUSH)GetStockObject(WHITE_BRUSH));

			//-------------------------------------도로 그리기
			hPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
			oldPen = (HPEN)SelectObject(memDC, hPen);

			MoveToEx(memDC, 340, 0, NULL); LineTo(memDC, 340, rt.bottom);
			MoveToEx(memDC, 400, 0, NULL); LineTo(memDC, 400, rt.bottom);
			MoveToEx(memDC, 460, 0, NULL); LineTo(memDC, 460, rt.bottom);

			MoveToEx(memDC, 0, 240, NULL); LineTo(memDC, rt.right, 240);
			MoveToEx(memDC, 0, 300, NULL); LineTo(memDC, rt.right, 300);
			MoveToEx(memDC, 0, 360, NULL); LineTo(memDC, rt.right, 360);

			SelectObject(memDC, oldPen);
			DeleteObject(hPen);
			//-------------------------------------자동차 그리기
			for (int i = 0; i < 8; ++i) {
				CAR& c = cars[i];
				int w = 0, h = 0;

				if (c.dir == 0 || c.dir == 2) { w = 25; h = 15; }
				else { w = 15; h = 25; }

				hBrush = CreateSolidBrush(RGB(102, 255, 255));
				oldBrush = (HBRUSH)SelectObject(memDC, hBrush);

				if (selected == i + 1) {
					hPen = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
				}
				else {
					hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
				}
				oldPen = (HPEN)SelectObject(memDC, hPen);

				Rectangle(memDC, c.x - w, c.y - h, c.x + w, c.y + h);

				if (c.x - w < 0)
					Rectangle(memDC, v_w + (c.x - w), c.y - h, v_w + (c.x + w), c.y + h);
				if (c.x + w > v_w)
					Rectangle(memDC, (c.x - w) - v_w, c.y - h, (c.x + w) - v_w, c.y + h);
				if (c.y - h < 0)
					Rectangle(memDC, c.x - w, v_h + (c.y - h), c.x + w, v_h + (c.y + h));
				if (c.y + h > v_h)
					Rectangle(memDC, c.x - w, (c.y - h) - v_h, c.x + w, (c.y + h) - v_h);

				SelectObject(memDC, oldPen);
				DeleteObject(hPen);
				SelectObject(memDC, oldBrush);
				DeleteObject(hBrush);
			}
			//-------------------------------------신호등 그리기
			hBrush = CreateSolidBrush(RGB(0, 0, 0));
			oldBrush = (HBRUSH)SelectObject(memDC, hBrush);

			Rectangle(memDC, 200, 10, 330, 60);
			Rectangle(memDC, 470, 520, 600, 570);

			SelectObject(memDC, oldBrush);
			DeleteObject(hBrush);

			if (traffic_lights[0].state == 0) {
				hBrush = CreateSolidBrush(RGB(255, 0, 0));
				oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
				Ellipse(memDC, 210, 20, 240, 50);
				SelectObject(memDC, oldBrush);
				DeleteObject(hBrush);
			}
			else if (traffic_lights[0].state == 1) {
				hBrush = CreateSolidBrush(RGB(255, 255, 102));
				oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
				Ellipse(memDC, 250, 20, 280, 50);
				SelectObject(memDC, oldBrush);
				DeleteObject(hBrush);
			}
			else if (traffic_lights[0].state == 2) {
				hBrush = CreateSolidBrush(RGB(0, 102, 255));
				oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
				Ellipse(memDC, 290, 20, 320, 50);
				SelectObject(memDC, oldBrush);
				DeleteObject(hBrush);
			}

			if (traffic_lights[1].state == 0) {
				hBrush = CreateSolidBrush(RGB(255, 0, 0));
				oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
				Ellipse(memDC, 480, 530, 510, 560);
				SelectObject(memDC, oldBrush);
				DeleteObject(hBrush);
			}
			else if (traffic_lights[1].state == 1) {
				hBrush = CreateSolidBrush(RGB(255, 255, 102));
				oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
				Ellipse(memDC, 520, 530, 550, 560);
				SelectObject(memDC, oldBrush);
				DeleteObject(hBrush);
			}
			else if (traffic_lights[1].state == 2) {
				hBrush = CreateSolidBrush(RGB(0, 102, 255));
				oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
				Ellipse(memDC, 560, 530, 590, 560);
				SelectObject(memDC, oldBrush);
				DeleteObject(hBrush);
			}
			//-------------------------------------사람 그리기
			hBrush = CreateSolidBrush(RGB(255, 255, 102));
			oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
			Ellipse(memDC, man.x - 20, man.y - 20, man.x + 20, man.y + 20);
			SelectObject(memDC, oldBrush);
			DeleteObject(hBrush);

			SetBkMode(memDC, TRANSPARENT);
			SetTextColor(memDC, RGB(0, 0, 0));
			SetTextAlign(memDC, TA_TOP);
			_stprintf_s(str, L"신호 변경까지 %.1f초", ttime);
			TextOut(memDC, 30, 500, str, _tcslen(str));

			// 완성된 백버퍼를 실제 화면으로 한 번에 복사
			BitBlt(hDC, 0, 0, rt.right, rt.bottom, memDC, 0, 0, SRCCOPY);

			SelectObject(memDC, oldBit);
			DeleteObject(hBit);
			DeleteDC(memDC);
		}
		EndPaint(hWnd, &ps);
		break;
	case WM_ERASEBKGND:
		return 1;
	case WM_LBUTTONDOWN:
		

		InvalidateRect(hWnd, NULL, FALSE);
		break;
	case WM_RBUTTONDOWN:
		

		InvalidateRect(hWnd, NULL, FALSE);
		break;
	case WM_TIMER:
		GetClientRect(hWnd, &rt);

		if (wParam == 1) {

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


BOOL CALLBACK Dlalog_Proc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch (iMsg) {
	case WM_INITDIALOG:
		CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO4, NULL);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_RADIO1:
			break;
		case IDC_RADIO2:
			break;
		case IDC_RADIO3:
			break;
		case IDC_RADIO4:
			break;
		case IDC_CHECK1:
			break;
		case IDC_CHECK2:
			break;
		case IDC_CHECK3:
			break;
		case IDC_CHECK4:
			break;
		case IDC_BUTTON1:
			break;
		case IDC_BUTTON2:
			break;
		case IDC_BUTTON3:
			break;
		case IDC_BUTTON4:
			break;
		case IDC_BUTTON5:
			break;
		case IDOK:
			break;
		case IDCANCEL:
			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg, 0);
		break;
	}
	return 0;
}