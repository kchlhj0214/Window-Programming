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
	switch (uMsg) {
	case WM_CREATE:
		DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, (DLGPROC)Dlalog_Proc);

		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

POINT ConstrainLineToRect(POINT start, POINT mouse)
{
	double x1 = start.x;
	double y1 = start.y;
	double x2 = mouse.x;
	double y2 = mouse.y;

	// 그리기 영역 경계 정의
	double minX = 0;
	double maxX = 600;
	double minY = 0;
	double maxY = 350;

	// 마우스가 이미 영역 안에 있다면 계산 없이 그대로 반환
	if (x2 >= minX && x2 <= maxX && y2 >= minY && y2 <= maxY) {
		return mouse;
	}

	double dx = x2 - x1;
	double dy = y2 - y1;

	// 수직선이 아니고, 좌우 경계를 벗어난 경우 가로축 교점 계산
	if (dx != 0) {
		if (x2 < minX) {
			y2 = y1 + (minX - x1) * (dy / dx);
			x2 = minX;
		}
		else if (x2 > maxX) {
			y2 = y1 + (maxX - x1) * (dy / dx);
			x2 = maxX;
		}
	}

	// 수평선이 아니고, 상하 경계를 벗어난 경우 세로축 교점 계산
	if (dy != 0) {
		if (y2 < minY) {
			x2 = x1 + (minY - y1) * (dx / dy);
			y2 = minY;
		}
		else if (y2 > maxY) {
			x2 = x1 + (maxY - y1) * (dx / dy);
			y2 = maxY;
		}
	}

	// 계산된 교점이 다른 축의 경계를 다시 벗어나지 않도록 최종 보정(Clamp)
	if (x2 < minX) x2 = minX;
	if (x2 > maxX) x2 = maxX;
	if (y2 < minY) y2 = minY;
	if (y2 > maxY) y2 = maxY;

	POINT result = { (long)x2, (long)y2 };
	return result;
}

BOOL CALLBACK Dlalog_Proc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC;
	RECT updateRect = { 0, 0, 650, 400 };
	static int check_rgb[3] = { 0, 0, 0 };

	switch (iMsg) {
	case WM_INITDIALOG:
		CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO2, IDC_RADIO1);
		CheckRadioButton(hDlg, IDC_RADIO3, IDC_RADIO5, IDC_RADIO3);
		CheckRadioButton(hDlg, IDC_RADIO6, IDC_RADIO7, IDC_RADIO6);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_RADIO1:
			isRect = true;
			InvalidateRect(hDlg, &updateRect, FALSE);
			
			break;
		case IDC_RADIO2:
			isRect = false;
			InvalidateRect(hDlg, &updateRect, FALSE);

			break;
		case IDC_RADIO3:
			g_ShapeSize = 10;
			InvalidateRect(hDlg, &updateRect, FALSE);

			break;
		case IDC_RADIO4:
			g_ShapeSize = 20;
			InvalidateRect(hDlg, &updateRect, FALSE);

			break;
		case IDC_RADIO5:
			g_ShapeSize = 30;
			InvalidateRect(hDlg, &updateRect, FALSE);

			break;
		case IDC_RADIO6:
			isGrid = true;
			InvalidateRect(hDlg, &updateRect, FALSE);

			break;
		case IDC_RADIO7:
			isGrid = false;
			InvalidateRect(hDlg, &updateRect, FALSE);
			
			break;
		case IDC_CHECK1:
			check_rgb[0] = 1 - check_rgb[0];
			rgb_r = check_rgb[0] * 255;

			InvalidateRect(hDlg, &updateRect, FALSE);

			break;
		case IDC_CHECK2:
			check_rgb[1] = 1 - check_rgb[1];
			rgb_g = check_rgb[1] * 255;

			InvalidateRect(hDlg, &updateRect, FALSE);

			break;
		case IDC_CHECK3:
			check_rgb[2] = 1 - check_rgb[2];
			rgb_b = check_rgb[2] * 255;

			InvalidateRect(hDlg, &updateRect, FALSE);

			break;
		case IDC_BUTTON1:
			if (g_IsMoving) {
				KillTimer(hDlg, 1);
				g_IsMoving = FALSE;
			}
			g_CurrentMode = MODE_DRAW_LINE;
			InvalidateRect(hDlg, NULL, FALSE);
			break;
		case IDC_BUTTON2:
			if (g_LineStart.x == g_LineEnd.x && g_LineStart.y == g_LineEnd.y) break;

			if (g_IsMoving) {
				// 이동 중일 때 누르면 중간에 정지
				KillTimer(hDlg, 1);
				g_IsMoving = FALSE;
			}
			else {
				g_CurrentX = (double)g_CenterPt.x;
				g_CurrentY = (double)g_CenterPt.y;

				double totalDeltaX = g_LineEnd.x - g_CurrentX;
				double totalDeltaY = g_LineEnd.y - g_CurrentY;
				double totalDistance = sqrt(totalDeltaX * totalDeltaX + totalDeltaY * totalDeltaY);

				double baseSpeed = 4.0;

				g_SpeedX = (totalDeltaX / totalDistance) * baseSpeed;
				g_SpeedY = (totalDeltaY / totalDistance) * baseSpeed;

				SetTimer(hDlg, 1, speed, NULL);
				g_IsMoving = TRUE;
			}
			break;
		case IDC_BUTTON3:
			if (speed > 20) {
				speed -= 5;
				if (g_IsMoving) SetTimer(hDlg, 1, speed, NULL);
			}
			break;
		case IDC_BUTTON4:
			if (speed < 50) {
				speed += 5;
				if (g_IsMoving) SetTimer(hDlg, 1, speed, NULL);
			}
			break;
		case IDC_BUTTON5:			
			EndDialog(hDlg, 0);
			break;
		}
		break;
	case WM_LBUTTONDOWN:
		if (g_CurrentMode == MODE_DRAW_LINE) {
			g_isDragging = TRUE;
			SetCapture(hDlg);
			g_LineStart = g_CenterPt;
			g_LineEnd = g_CenterPt;
		}
		break;

	case WM_MOUSEMOVE:
		if (g_isDragging) {
			POINT mousePt = { LOWORD(lParam), HIWORD(lParam) };
			g_LineEnd = ConstrainLineToRect(g_LineStart, mousePt);
			InvalidateRect(hDlg, &updateRect, FALSE);
		}
		break;

	case WM_LBUTTONUP:
		if (g_isDragging) {
			g_isDragging = FALSE;
			ReleaseCapture();

			POINT mousePt = { LOWORD(lParam), HIWORD(lParam) };
			g_LineEnd = ConstrainLineToRect(g_LineStart, mousePt);

			g_hasLine = TRUE;
			g_CurrentMode = MODE_NONE;

			InvalidateRect(hDlg, &updateRect, FALSE);
		}
		break;

	case WM_TIMER:
		if (wParam == 1) {
			g_CurrentX += g_SpeedX;
			g_CurrentY += g_SpeedY;

			double remainX = g_LineEnd.x - g_CurrentX;
			double remainY = g_LineEnd.y - g_CurrentY;
			double remainDist = sqrt(remainX * remainX + remainY * remainY);

			double stepDist = sqrt(g_SpeedX * g_SpeedX + g_SpeedY * g_SpeedY);

			if (remainDist <= stepDist) {
				g_CenterPt = g_LineEnd; // 최종 위치 강제 고정
				g_IsMoving = FALSE;
				KillTimer(hDlg, 1);

				// 출발점과 끝점 정보 Swap
				POINT temp = g_LineStart;
				g_LineStart = g_LineEnd;
				g_LineEnd = temp;
			}
			else {
				g_CenterPt.x = (long)(g_CurrentX + 0.5);
				g_CenterPt.y = (long)(g_CurrentY + 0.5);
			}

			InvalidateRect(hDlg, &updateRect, FALSE);
		}
		break;

	case WM_PAINT:
	{
		hDC = BeginPaint(hDlg, &ps);

		RECT clientRect;
		GetClientRect(hDlg, &clientRect);
		int width = clientRect.right - clientRect.left;
		int height = clientRect.bottom - clientRect.top;

		HDC hMemDC = CreateCompatibleDC(hDC);
		HBITMAP hNewBitmap = CreateCompatibleBitmap(hDC, width, height);
		HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, hNewBitmap);

		HBRUSH hBackBrush = GetSysColorBrush(COLOR_3DFACE);
		FillRect(hMemDC, &clientRect, hBackBrush);
		

		// -------------------------------------------------------------

		if (isGrid) {
			HPEN hPen = NULL;
			HPEN hOldPen = NULL;

			hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
			hOldPen = (HPEN)SelectObject(hMemDC, hPen);

			for (int i = 0; i < 24; ++i) {
				for (int j = 0; j < 14; ++j) {
					Rectangle(hMemDC, i * 25, j * 25, (i + 1) * 25, (j + 1) * 25);
				}
			}

			SelectObject(hMemDC, hOldPen);
			DeleteObject(hPen);
		}

		// -------------------------------------------------------------

		HBRUSH hShapeBrush = CreateSolidBrush(RGB(rgb_r, rgb_g, rgb_b));
		HBRUSH hOldBrush = (HBRUSH)SelectObject(hMemDC, hShapeBrush);

		// ----도형 그리기----------------------------------------------
		int halfSize = g_ShapeSize / 2;
		RECT drawRect = { g_CenterPt.x - halfSize, g_CenterPt.y - halfSize, g_CenterPt.x + halfSize, g_CenterPt.y + halfSize };
		if (isRect) Rectangle(hMemDC, drawRect.left, drawRect.top, drawRect.right, drawRect.bottom);
		else Ellipse(hMemDC, drawRect.left, drawRect.top, drawRect.right, drawRect.bottom);

		SelectObject(hMemDC, hOldBrush);
		DeleteObject(hShapeBrush);
		// -------------------------------------------------------------

		// ----선 그리기------------------------------------------------
		if (g_LineStart.x != g_LineEnd.x || g_LineStart.y != g_LineEnd.y) {
			HPEN hPen = NULL;
			HPEN hOldPen = NULL;

			if (g_isDragging) {
				hPen = CreatePen(PS_DOT, 1, RGB(255, 0, 0));
				hOldPen = (HPEN)SelectObject(hMemDC, hPen);
				MoveToEx(hMemDC, g_LineStart.x, g_LineStart.y, NULL);
				LineTo(hMemDC, g_LineEnd.x, g_LineEnd.y);
			}
			else {
				hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
				hOldPen = (HPEN)SelectObject(hMemDC, hPen);
				MoveToEx(hMemDC, g_LineStart.x, g_LineStart.y, NULL);
				LineTo(hMemDC, g_LineEnd.x, g_LineEnd.y);
			}

			SelectObject(hMemDC, hOldPen);
			DeleteObject(hPen);
		}

		BitBlt(hDC, 0, 0, width, height, hMemDC, 0, 0, SRCCOPY);

		SelectObject(hMemDC, hOldBitmap);
		DeleteObject(hNewBitmap);
		DeleteDC(hMemDC);

		EndPaint(hDlg, &ps);
	}
	return TRUE;
	case WM_ERASEBKGND:
		return TRUE;
	case WM_CLOSE:
		EndDialog(hDlg, 0);
		break;
	}
	return 0;
}