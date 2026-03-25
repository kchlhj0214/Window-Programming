#include <windows.h>
#include <tchar.h>
#include <random>
#include <vector>

using namespace std;
random_device rd;
mt19937 g(rd());
uniform_int_distribution<> uid{ 0, 600 };		// 800 600 기준 12단까지
random_device rd2;
mt19937 g2(rd2());
uniform_int_distribution<> uid2{ 0, 255 };
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
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
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
	static TCHAR str[10][31] = { 0 };
	static int count = 0;
	static SIZE size;
	static int line = 0;



	switch (uMsg) {
	case WM_CREATE:
		CreateCaret(hWnd, NULL, 2, 15);
		ShowCaret(hWnd);
		break;
	case WM_CHAR:
		hDC = GetDC(hWnd);

		if (wParam == VK_BACK) {
			if (count > 0) {
				--count;
				str[line][count] = '\0';
			}
			else if (line > 0) {
				--line;
				count = lstrlen(str[line]);
			}
		}
		else if (wParam == VK_ESCAPE) {
			count = 0;
			line = 0;
			memset(str, 0, sizeof(str));
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == VK_RETURN) {
			if (line < 9) {
				++line;
				count = 0;
			}
		}
		else {
			if (count < 30) {
				str[line][count++] = wParam;
				str[line][count] = '\0';

			}
			else {
				count = 0;
				++line;
				str[line][count++] = wParam;
				str[line][count] = '\0';
			}
		}

		str[line][count] = '\0';
		InvalidateRect(hWnd, NULL, TRUE);
		break;
	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);
		for (int i = 0; i <= line; ++i) {
			TextOut(hDC, 0, i * 20, str[i], lstrlen(str[i]));
		}
		GetTextExtentPoint32(hDC, str[line], lstrlen(str[line]), &size);
		SetCaretPos(size.cx, line * 20);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		HideCaret(hWnd);
		DestroyCaret();
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}