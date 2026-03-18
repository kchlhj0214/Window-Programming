#include <windows.h>
#include <tchar.h>
#include <random>
#include <vector>

using namespace std;
random_device rd;
mt19937 g(rd());
uniform_int_distribution<> uid{ 0, 650 };

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
	hWnd = CreateWindow(lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW, 0, 0, 800, 600, NULL, (HMENU)NULL, hInstance, NULL);
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
	TCHAR lpOut[100] = L"2x9 = 18";
	int num;

	switch (uMsg) {
	case WM_CREATE:
		break;
	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);

		TextOut(hDC, 0, 0, lpOut, lstrlen(lpOut));			// x최소간격 70, y간격 30
		TextOut(hDC, 70, 0, lpOut, lstrlen(lpOut));
		TextOut(hDC, 140, 0, lpOut, lstrlen(lpOut));
		TextOut(hDC, 210, 0, lpOut, lstrlen(lpOut));
		TextOut(hDC, 280, 0, lpOut, lstrlen(lpOut));
		TextOut(hDC, 350, 0, lpOut, lstrlen(lpOut));
		TextOut(hDC, 420, 0, lpOut, lstrlen(lpOut));
		TextOut(hDC, 490, 0, lpOut, lstrlen(lpOut));
		TextOut(hDC, 560, 0, lpOut, lstrlen(lpOut));
		TextOut(hDC, 630, 0, lpOut, lstrlen(lpOut));
		TextOut(hDC, 700, 0, lpOut, lstrlen(lpOut));

		TextOut(hDC, 0, 30, lpOut, lstrlen(lpOut));
		TextOut(hDC, 0, 240, lpOut, lstrlen(lpOut));
		TextOut(hDC, 0, 510, lpOut, lstrlen(lpOut));

		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}