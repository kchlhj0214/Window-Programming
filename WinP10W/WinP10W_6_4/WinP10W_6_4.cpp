#include <windows.h>
#include <tchar.h>
#include <random>
#include <vector>
#include <math.h>
#include <algorithm>
#include <atlimage.h>
#include <commctrl.h>
#include "resource.h"

#pragma comment(lib, "comctl32.lib")

#define LEN 1000
#define HEI 1000
#define GRID_SIZE 5
#define ROWS (HEI / GRID_SIZE)
#define COLS (LEN / GRID_SIZE)
#define PREVIEW_X 107
#define PREVIEW_Y 450

using namespace std;
random_device rd;
mt19937 g(rd());
uniform_int_distribution<> uid_rgb{ 0, 255 };
uniform_int_distribution<> disX{ 50, LEN - 50 };
uniform_int_distribution<> disY{ 50, HEI - 50 };
uniform_int_distribution<> disTime{ 1000, 3000 };

HINSTANCE g_hInst;
HWND g_hDlg = NULL;
LPCTSTR lpszClass = L"My Window Class";
LPCTSTR lpszWindowName = L"windows program 2";

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK Dlalog_Proc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

COLORREF gridData[ROWS][COLS] = { 0 };
bool showGrid = true;
bool isBrush = false;
bool isErase = false;
int rgbR = 0, rgbG = 0, rgbB = 0;

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
		if (g_hDlg == NULL || !IsDialogMessage(g_hDlg, &Message))
		{
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}
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
		for (int y = 0; y < ROWS; y++) {
			for (int x = 0; x < COLS; x++) {
				gridData[y][x] = RGB(255, 255, 255);
			}

		}
		g_hDlg = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, (DLGPROC)Dlalog_Proc);

		if (g_hDlg != NULL) {
			RECT rcMain, rcDlg;
			GetWindowRect(hWnd, &rcMain);

			GetWindowRect(g_hDlg, &rcDlg);
			int dlgWidth = rcDlg.right - rcDlg.left;
			int dlgHeight = rcDlg.bottom - rcDlg.top;

			SetWindowPos(g_hDlg, NULL,
				rcMain.right + 10,
				rcMain.top,
				dlgWidth, dlgHeight,
				SWP_NOZORDER | SWP_NOSIZE | SWP_SHOWWINDOW);

			ShowWindow(g_hDlg, SW_SHOW);
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

			// 백버퍼 생성
			HDC memDC = CreateCompatibleDC(hDC);
			HBITMAP hBit = CreateCompatibleBitmap(hDC, rt.right, rt.bottom);
			HBITMAP oldBit = (HBITMAP)SelectObject(memDC, hBit);

			// 배경 지우기 (더블 버퍼링의 핵심: 하얀색으로 백버퍼를 채움)
			FillRect(memDC, &ps.rcPaint, (HBRUSH)GetStockObject(WHITE_BRUSH));

			//-------------------------------------
			int startX = ps.rcPaint.left / GRID_SIZE - 1;
			int endX = ps.rcPaint.right / GRID_SIZE + 1;
			int startY = ps.rcPaint.top / GRID_SIZE - 1;
			int endY = ps.rcPaint.bottom / GRID_SIZE + 1;

			if (startX < 0) startX = 0;
			if (endX > COLS) endX = COLS;
			if (startY < 0) startY = 0;
			if (endY > ROWS) endY = ROWS;

			// 색상 칠하기
			for (int y = startY; y < endY; y++) {
				for (int x = startX; x < endX; x++) {
					if (gridData[y][x] != RGB(255, 255, 255)) {
						HBRUSH hBr = CreateSolidBrush(gridData[y][x]);
						RECT r = { x * GRID_SIZE, y * GRID_SIZE, (x + 1) * GRID_SIZE, (y + 1) * GRID_SIZE };
						FillRect(memDC, &r, hBr);
						DeleteObject(hBr);
					}
				}
			}

			// 그리드 선 긋기
			if (showGrid) {
				HPEN hPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
				HPEN oldPen = (HPEN)SelectObject(memDC, hPen);

				for (int y = startY; y <= endY; y++) {
					if (y <= ROWS) {
						MoveToEx(memDC, ps.rcPaint.left, y * GRID_SIZE, NULL);
						LineTo(memDC, ps.rcPaint.right, y * GRID_SIZE);
					}
				}
				for (int x = startX; x <= endX; x++) {
					if (x <= COLS) {
						MoveToEx(memDC, x * GRID_SIZE, ps.rcPaint.top, NULL);
						LineTo(memDC, x * GRID_SIZE, ps.rcPaint.bottom);
					}
				}

				SelectObject(memDC, oldPen);
				DeleteObject(hPen);
			}
			//-------------------------------------

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
	case WM_MOUSEMOVE:
		if (wParam & MK_LBUTTON) {
			int x = LOWORD(lParam) / GRID_SIZE;
			int y = HIWORD(lParam) / GRID_SIZE;
			if (x >= 0 && x < COLS && y >= 0 && y < ROWS) {

				bool isChanged = false;
				if (isBrush && gridData[y][x] != RGB(rgbR, rgbG, rgbB)) {
					gridData[y][x] = RGB(rgbR, rgbG, rgbB);
					isChanged = true;
				}
				else if (isErase && gridData[y][x] != RGB(255, 255, 255)) {
					gridData[y][x] = RGB(255, 255, 255);
					isChanged = true;
				}

				if (isChanged) {
					RECT updateRect = { x * GRID_SIZE, y * GRID_SIZE, (x + 1) * GRID_SIZE + 1, (y + 1) * GRID_SIZE + 1 };
					InvalidateRect(hWnd, &updateRect, FALSE);
				}
			}
		}
		break;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


BOOL CALLBACK Dlalog_Proc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	HWND hParentWnd = GetParent(hDlg);

	switch (iMsg) {
	case WM_INITDIALOG:
		CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO2, NULL);
		CheckDlgButton(hDlg, IDC_CHECK1, BST_CHECKED);

		for (int i = IDC_SLIDER1; i <= IDC_SLIDER3; i++)
			SendDlgItemMessage(hDlg, i, TBM_SETRANGE, TRUE, MAKELONG(0, 255));

		SetDlgItemText(hDlg, IDC_STATIC_R, _T("R - 000"));
		SetDlgItemText(hDlg, IDC_STATIC_G, _T("G - 000"));
		SetDlgItemText(hDlg, IDC_STATIC_B, _T("B - 000"));
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_RADIO1:
			isBrush = true; 
			isErase = false;
			break;
		case IDC_RADIO2:
			isBrush = false; 
			isErase = true;
			break;
		case IDC_BUTTON1:
			for (int y = 0; y < ROWS; y++) {
				for (int x = 0; x < COLS; x++) {
					gridData[y][x] = RGB(255, 255, 255);
				}
			}
			InvalidateRect(hParentWnd, NULL, FALSE);
			break;
		case IDC_BUTTON2:
			EndDialog(hDlg, 0);
			break;
		case IDC_BUTTON3:
			for (int y = 0; y < ROWS; y++) {
				for (int x = 0; x < COLS; x++) {
					gridData[y][x] = RGB(rgbR, rgbG, rgbB);
				}
			}
			InvalidateRect(hParentWnd, NULL, FALSE);
			break;
		case IDC_CHECK1:
			showGrid = IsDlgButtonChecked(hDlg, IDC_CHECK1);
			InvalidateRect(hParentWnd, NULL, FALSE);
			break;
		}
		return TRUE;
	case WM_VSCROLL:
	{
		rgbR = SendDlgItemMessage(hDlg, IDC_SLIDER1, TBM_GETPOS, 0, 0);
		rgbG = SendDlgItemMessage(hDlg, IDC_SLIDER2, TBM_GETPOS, 0, 0);
		rgbB = SendDlgItemMessage(hDlg, IDC_SLIDER3, TBM_GETPOS, 0, 0);

		TCHAR buf[32];
		wsprintf(buf, _T("R - %03d"), rgbR); SetDlgItemText(hDlg, IDC_STATIC_R, buf);
		wsprintf(buf, _T("G - %03d"), rgbG); SetDlgItemText(hDlg, IDC_STATIC_G, buf);
		wsprintf(buf, _T("B - %03d"), rgbB); SetDlgItemText(hDlg, IDC_STATIC_B, buf);

		RECT rcColor = { PREVIEW_X, PREVIEW_Y, PREVIEW_X + 20, PREVIEW_Y + 20 };
		InvalidateRect(hDlg, &rcColor, TRUE);
		return TRUE;
	}
	case WM_PAINT: // 색상 미리보기 그리기
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hDlg, &ps);
		HBRUSH hBr = CreateSolidBrush(RGB(rgbR, rgbG, rgbB));
		RECT r = { PREVIEW_X, PREVIEW_Y, PREVIEW_X + 20, PREVIEW_Y + 20 };
		FillRect(hdc, &r, hBr);
		DeleteObject(hBr);
		EndPaint(hDlg, &ps);
		return TRUE;
	}
	case WM_CLOSE:
		EndDialog(hDlg, 0);
		break;
	}
	return 0;
}