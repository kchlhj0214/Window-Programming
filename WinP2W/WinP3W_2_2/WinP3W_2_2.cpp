#include <windows.h>
#include <tchar.h>
#include <random>
#include <vector>

using namespace std;
random_device rd;
mt19937 g(rd());
uniform_int_distribution<> uid{ 2, 12 };		// 800 600 기준 12단까지
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
	TCHAR lpOut[100];
	static int num = uid(g);
	int dividedPos = LEN / (num - 1);
	RECT rect;

	switch (uMsg) {
	case WM_CREATE:
		break;
	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);
		
		for (int i = 0; i < num - 1; ++i) {
			rect.left = (0 + dividedPos * i);
			rect.top = 0;
			rect.right = (dividedPos + dividedPos * i);
			rect.bottom = HEI / 2;

			for (int j = 1; j <= 9; ++j) {
				wsprintf(lpOut, L"%dx%d = %d\n", i + 2, j, (i + 2) * j);
				DrawText(hDC, lpOut, lstrlen(lpOut), &rect, DT_LEFT | DT_TOP);
				rect.top += 30;
			}
		}

		for (int i = 0; i < num - 1; ++i) {
			rect.left = (0 + dividedPos * i);
			rect.top = HEI / 2;
			rect.right = (dividedPos + dividedPos * i);
			rect.bottom = HEI;

			for (int j = 1; j <= 9; ++j) {
				wsprintf(lpOut, L"%dx%d = %d\n", (num - i), j, (num - i) * j);
				DrawText(hDC, lpOut, lstrlen(lpOut), &rect, DT_LEFT | DT_TOP);
				rect.top += 30;
			}
		}

		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}