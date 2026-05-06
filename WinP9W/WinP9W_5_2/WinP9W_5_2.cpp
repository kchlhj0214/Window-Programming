#include <windows.h>
#include <tchar.h>
#include <random>
#include <vector>
#include <math.h>
#include "resource.h"

#define LEN 1000
#define HEI 1000
#define PLUS 10
#define MINUS -10
#define BORAR_SIZE 5

using namespace std;
random_device rd;
mt19937 g(rd());
uniform_int_distribution<> uid_rgb{ 0, 255 };
uniform_int_distribution<> uid_speed{ 1, 10 };
uniform_int_distribution<> uid_pos1{ 0, 8 };
uniform_int_distribution<> uid_pos2{ 0, 15 };
uniform_int_distribution<> uid_pos3{ 0, 24 };

struct RECTS { int boardPos; RECT pic; };

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"My Window Class";
LPCTSTR lpszWindowName = L"windows program 2";

bool IsA = false;
int bmW, bmH;
int pictureDivision = 1;
int selected = 0;
vector<RECTS> divRects;
bool IsR = false;
int board[BORAR_SIZE][BORAR_SIZE];
vector<POINT> pos;

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
	WndClass.lpszMenuName = MAKEINTRESOURCE("IDR_MENU1");
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

void UpdateRects(int mode, int w, int h) {
	pos.clear();
	divRects.clear();
	int rows = mode;
	int cols = mode;

	int cellW = w / cols;
	int cellH = h / rows;

	int p = 0;

	for (int r = 0; r < rows; r++) {
		for (int c = 0; c < cols; c++) {
			RECTS rct;
			POINT pt;
			if ((r != rows - 1) || (c != cols - 1))
				rct = { p++, { c * cellW, r * cellH, (c + 1) * cellW, (r + 1) * cellH }};
			pt = { c * cellW, r * cellH };
			pos.push_back(pt);
			divRects.push_back(rct);
		}
	}
}

void ShuffleRects(vector<RECTS>& r, int mode) {
	
}



LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC;
	HPEN hPen, oldPen;
	HBRUSH hBrush, oldBrush;
	static RECT rt;
	static int mx, my;
	static BITMAP bmp;
	static HBITMAP hBitmap1, hBitmap2;

	switch (uMsg) {
	case WM_CREATE:
		hBitmap1 = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP1));
		hBitmap2 = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP2));
		GetObject(hBitmap1, sizeof(BITMAP), &bmp1);
		GetObject(hBitmap2, sizeof(BITMAP), &bmp2);
		bmW = bmp.bmWidth;
		bmH = bmp.bmHeight;
		UpdateRects(1, bmW, bmH);
		break;
	case WM_KEYDOWN:
		GetClientRect(hWnd, &rt);
		if (wParam == 'S') {
			if (IsA) {
				bmW = bmp.bmWidth;
				bmH = bmp.bmHeight;
				IsA = !IsA;
			}
			else {
				bmW = rt.right;
				bmH = rt.bottom;
				IsA = !IsA;
			}
			UpdateRects(pictureDivision, bmW, bmH);
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'F') {
			if (bmW < rt.right && bmH < rt.bottom)
				bmW += PLUS; bmH += PLUS;
			if (bmW > rt.right) bmW = rt.right;
			if (bmH > rt.bottom) bmH = rt.bottom;
			UpdateRects(pictureDivision, bmW, bmH);
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'V') {
			if ((bmW > rt.right / 4) && (bmH > rt.bottom / 4))
				bmW += MINUS; bmH += MINUS;
			if (bmW < rt.right / 4) bmW = rt.right / 4;
			if (bmH < rt.bottom / 4) bmH = rt.bottom / 4;
			UpdateRects(pictureDivision, bmW, bmH);
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'H') {
			pictureDivision = wParam - '0';
			selected = 0;
			IsR = false;
			if (wParam == '1')
				UpdateRects(1, bmp.bmWidth, bmp.bmHeight);
			else {
				UpdateRects(wParam - '0', rt.right, rt.bottom);
				bmW = rt.right;
				bmH = rt.bottom;
			}
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'Q' || wParam == VK_ESCAPE) {
			DeleteObject(hBitmap);
			PostQuitMessage(0);
			break;
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
		GetClientRect(hWnd, &rt);
		{
			// 백버퍼 생성
			HDC memDC = CreateCompatibleDC(hDC);
			HBITMAP hBit = CreateCompatibleBitmap(hDC, rt.right, rt.bottom);
			HBITMAP oldBit = (HBITMAP)SelectObject(memDC, hBit);

			// 배경 지우기 (더블 버퍼링의 핵심: 하얀색으로 백버퍼를 채움)
			FillRect(memDC, &ps.rcPaint, (HBRUSH)GetStockObject(WHITE_BRUSH));
			//------------------------------------------------------------------
			HDC hMemDC = CreateCompatibleDC(hDC);
			SelectObject(hMemDC, hBitmap);

			for (int i = 0; i < divRects.size(); i++) {
				int curW = divRects[i].right - divRects[i].left;
				int curH = divRects[i].bottom - divRects[i].top;

				if ((selected == i + 1) && IsR)
					StretchBlt(memDC, divRects[i].left, divRects[i].top, curW, curH, 
						hMemDC, divRects[i].left, divRects[i].top, divRects[i].right, divRects[i].bottom, NOTSRCCOPY);
				else
					StretchBlt(memDC, divRects[i].left, divRects[i].top, curW, curH, hMemDC, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);

				if (selected == i + 1) {
					HBRUSH hOldBrush = (HBRUSH)SelectObject(memDC, GetStockObject(NULL_BRUSH));
					HPEN hRedPen = CreatePen(PS_SOLID, 5, RGB(255, 0, 0));
					HPEN hOldPen = (HPEN)SelectObject(memDC, hRedPen);

					Rectangle(memDC, divRects[i].left, divRects[i].top, divRects[i].right, divRects[i].bottom);

					SelectObject(memDC, hOldBrush);
					SelectObject(memDC, hOldPen);
					DeleteObject(hRedPen);
				}
			}

			DeleteDC(hMemDC);
			//------------------------------------------------------------------
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
	{
		mx = LOWORD(lParam);
		my = HIWORD(lParam);
		POINT pt = { mx, my };
		IsR = false;

		selected = 0;
		for (int i = 0; i < divRects.size(); i++) {
			if (PtInRect(&divRects[i], pt)) {
				selected = i + 1;
				break;
			}
		}
		InvalidateRect(hWnd, NULL, FALSE);
	}
	break;
	case WM_DESTROY:
		DeleteObject(hBitmap);
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}