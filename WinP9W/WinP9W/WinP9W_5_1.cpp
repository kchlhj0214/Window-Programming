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

using namespace std;
random_device rd;
mt19937 g(rd());
uniform_int_distribution<> uid_rgb{ 0, 255 };
uniform_int_distribution<> uid_speed{ 1, 10 };
uniform_int_distribution<> uid_dir{ 0, 1 };

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"My Window Class";
LPCTSTR lpszWindowName = L"windows program 2";

bool IsA = false;
int bmW, bmH;

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





LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC;
	HPEN hPen, oldPen;
	HBRUSH hBrush, oldBrush;
	static RECT rt;
	static int mx, my;
	static BITMAP bmp;
	static HBITMAP hBitmap;

	switch (uMsg) {
	case WM_CREATE:
		hBitmap = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP2));
		GetObject(hBitmap, sizeof(BITMAP), &bmp);
		bmW = bmp.bmWidth;
		bmH = bmp.bmHeight;
		break;
	case WM_KEYDOWN:
		GetClientRect(hWnd, &rt);
		if (wParam == 'A') {
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
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == VK_OEM_PLUS || wParam == VK_ADD) {
			if (bmW < rt.right && bmH < rt.bottom)
				bmW += PLUS; bmH += PLUS;
			if (bmW > rt.right) bmW = rt.right;
			if (bmH > rt.bottom) bmH = rt.bottom;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == VK_OEM_MINUS || wParam == VK_SUBTRACT) {
			if ((bmW > rt.right / 4) && (bmH > rt.bottom / 4))
				bmW += MINUS; bmH += MINUS;
			if (bmW < rt.right / 4) bmW = rt.right / 4;
			if (bmH < rt.bottom / 4) bmH = rt.bottom / 4;
			InvalidateRect(hWnd, NULL, FALSE);
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
			HDC hMemDC;
			hMemDC = CreateCompatibleDC(hDC);
			SelectObject(hMemDC, hBitmap);

			StretchBlt(memDC, 0, 0, bmW, bmH, hMemDC, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);

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
		int mx = LOWORD(lParam);
		int my = HIWORD(lParam);

		
	}
	break;

	case WM_MOUSEMOVE:
		
		break;
	case WM_LBUTTONUP:
		
		break;
	case WM_DESTROY:
		DeleteObject(hBitmap);
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}