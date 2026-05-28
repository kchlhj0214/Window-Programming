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

// --- [전역 상태 변수] ---
enum DrawMode { MODE_NONE, MODE_DRAW_LINE };
DrawMode g_CurrentMode = MODE_NONE;

// [변경] 도형 정보: 중심점 좌표와 크기(가로/세로)만 관리
POINT g_CenterPt = { 60, 60 };  // 기본 중심점 (50,50 위치에 크기 20일 때의 중심)
int g_ShapeSize = 10;           // 기본 크기 (사이즈 변경 가능)

// 마우스 드래그 및 선 정보
BOOL g_isDragging = FALSE;
POINT g_ptCurrent = { 60, 60 }; // 현재 마우스 위치 (고무줄용)
POINT g_ptEnd = { 60, 60 };     // 확정된 끝점
BOOL g_hasLine = FALSE;         // 고정된 선 존재 여부

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
	RECT updateRect = { 0, 0, 600, 350 };
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
			
			break;
		case IDC_RADIO2:

			break;
		case IDC_RADIO3:

			break;
		case IDC_RADIO4:

			break;
		case IDC_RADIO5:

			break;
		case IDC_RADIO6:

			break;
		case IDC_RADIO7:
			
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
			g_CurrentMode = MODE_DRAW_LINE;	
			InvalidateRect(hDlg, NULL, FALSE);

			break;
		case IDC_BUTTON2:

			break;
		case IDC_BUTTON3:

			break;
		case IDC_BUTTON4:

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

			g_ptCurrent = g_CenterPt;
		}
		break;

	case WM_MOUSEMOVE:
		if (g_isDragging) {
			POINT mousePt = { LOWORD(lParam), HIWORD(lParam) };

			// 🚨 [핵심] 마우스 위치가 가리키는 방향을 유지하되, 경계선 교점 좌표를 고무줄 끝점으로 저장
			g_ptCurrent = ConstrainLineToRect(g_CenterPt, mousePt);

			InvalidateRect(hDlg, &updateRect, FALSE);
		}
		break;

	case WM_LBUTTONUP:
		if (g_isDragging) {
			g_isDragging = FALSE;
			ReleaseCapture();

			POINT mousePt = { LOWORD(lParam), HIWORD(lParam) };

			// 🚨 [핵심] 마우스를 뗀 순간의 최종 직선 끝 좌표도 경계선 교점 좌표로 정확히 계산 후 저장!
			g_ptEnd = ConstrainLineToRect(g_CenterPt, mousePt);
			g_hasLine = TRUE;
			g_CurrentMode = MODE_NONE;

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
		// 🚨 [핵심 추가] 전역 변수 rgb_r, g, b를 이용한 브러시 생성 및 적용
		// -------------------------------------------------------------
		HBRUSH hShapeBrush = CreateSolidBrush(RGB(rgb_r, rgb_g, rgb_b));
		HBRUSH hOldBrush = (HBRUSH)SelectObject(hMemDC, hShapeBrush);
		// -------------------------------------------------------------

		// 1. 도형 그리기 (이제 설정한 rgb 색상으로 내부가 채워집니다)
		int halfSize = g_ShapeSize / 2;
		RECT drawRect = { g_CenterPt.x - halfSize, g_CenterPt.y - halfSize, g_CenterPt.x + halfSize, g_CenterPt.y + halfSize };
		Rectangle(hMemDC, drawRect.left, drawRect.top, drawRect.right, drawRect.bottom);

		// -------------------------------------------------------------
		// 🚨 [주의] 도형을 그린 후 기존 브러시로 원상복구하고 생성한 브러시 삭제
		// (선이나 다른 요소에 원치 않는 색이 들어가는 것을 방지하고 메모리 누수를 막습니다)
		// -------------------------------------------------------------
		SelectObject(hMemDC, hOldBrush);
		DeleteObject(hShapeBrush);
		// -------------------------------------------------------------

		// 2. 완성된 고정 선 그리기
		if (g_hasLine) {
			MoveToEx(hMemDC, g_CenterPt.x, g_CenterPt.y, NULL);
			LineTo(hMemDC, g_ptEnd.x, g_ptEnd.y);
		}

		// 3. 드래그 중인 고무줄 선 그리기
		if (g_isDragging) {
			HPEN hDotPen = CreatePen(PS_DOT, 1, RGB(255, 0, 0));
			HPEN hOldPen = (HPEN)SelectObject(hMemDC, hDotPen);

			MoveToEx(hMemDC, g_CenterPt.x, g_CenterPt.y, NULL);
			LineTo(hMemDC, g_ptCurrent.x, g_ptCurrent.y);

			SelectObject(hMemDC, hOldPen);
			DeleteObject(hDotPen);
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