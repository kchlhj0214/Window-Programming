#include <windows.h>
#include <tchar.h>
#include <random>
#include <vector>

using namespace std;
random_device rd;
mt19937 g(rd());
uniform_int_distribution<> uid{ 2, 20 };		// 800 600 기준 12단까지
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
	TCHAR lpOut[100];
	static int num = uid(g);
	RECT rect;
	static vector<COLORREF> colors;
	int danPos = 21;
	int rgbnum = 0;
	RECT clientRect;
	int winWidth;
	int winHeight;
	int dividedPos;

	switch (uMsg) {
	case WM_CREATE:
		for (int i = 0; i < (num - 1) * 9; ++i)
			colors.push_back(RGB(uid2(g), uid2(g), uid2(g)));
		
		break;
	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);

		GetClientRect(hWnd, &clientRect);
		winWidth = clientRect.right;
		winHeight = clientRect.bottom;

		dividedPos = winWidth / num;

		while(danPos > num)
			danPos = uid(g) - 1;

		for (int i = 0; i < 9; ++i) {
			SetTextColor(hDC, colors[rgbnum]);
			wsprintf(lpOut, L"%dx%d = %d", num, i + 1, (i + 1) * num);

			int x = (dividedPos * (danPos - 1)) + (i * 20);
			int y = 50 + (i * 30);

			TextOut(hDC, x, y, lpOut, lstrlen(lpOut));
			++rgbnum;
		}

		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
//
//{
//	PAINTSTRUCT ps;
//	HDC hDC;
//	TCHAR lpOut[100];
//	static int num = uid(g);
//	int dividedPos = LEN / num;
//	static vector<COLORREF> colors;
//	static int danPos = 21;
//	int rgbnum = 0;
//
//	switch (uMsg) {
//	case WM_CREATE:
//		for (int i = 0; i < (num - 1) * 9; ++i)
//			colors.push_back(RGB(uid2(g), uid2(g), uid2(g)));
//		break;
//	case WM_PAINT:
//
//		hDC = BeginPaint(hWnd, &ps);
//
//		while (danPos > num)
//			danPos = uid(g) - 1;
//
//		for (int i = 0; i < 9; ++i) {
//			SetTextColor(hDC, colors[rgbnum]);
//			wsprintf(lpOut, L"%dx%d = %d", num, i + 1, (i + 1) * num);
//			int x = (dividedPos * (danPos - 1)) + (i * 20);
//			int y = 50 + (i * 30);
//		
//			TextOut(hDC, x, y, lpOut, lstrlen(lpOut));
//			++rgbnum;
//		}
//		EndPaint(hWnd, &ps);
//		break;
//	case WM_DESTROY:
//		PostQuitMessage(0);
//		break;
//	}
//	return DefWindowProc(hWnd, uMsg, wParam, lParam);
//}