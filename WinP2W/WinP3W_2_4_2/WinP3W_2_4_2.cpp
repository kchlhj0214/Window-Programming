#include <windows.h>
#include <tchar.h>
#include <random>
#include <vector>

using namespace std;
#define LEN 800
#define HEI 600
random_device rd;
mt19937 g(rd());
uniform_int_distribution<> uid{ 0, 700 };		// 800 600 기준 12단까지
uniform_int_distribution<> uid2{ 0, 255 };
uniform_int_distribution<> uid3{ 100, LEN / 2 };
uniform_int_distribution<> uid4{ 100, HEI / 2 };
uniform_int_distribution<> uid5{ 65, 90 };		// 대문자 아스키 코드

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
	static TCHAR lpOut[4][100];
	static int num = uid5(g);
	RECT rect;
	static vector<COLORREF> colors;
	int rgbnum = 0;

	static int rectWidth = uid3(g);
	static int rectHeight = uid4(g);
	static int x;
	static int y;

	switch (uMsg) {
	case WM_CREATE:
		for (int i = 0; i < (num - 1) * 9; ++i)
			colors.push_back(RGB(uid2(g), uid2(g), uid2(g)));

		while (1) {
			x = uid(g);
			y = uid(g);
			if (x < LEN - rectWidth - 30 && y < HEI - rectHeight - 40)
				break;
		}

		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 99; ++j) {
				lpOut[i][j] = uid5(g);
			}
			lpOut[i][99] = _T('\0');
		}
		break;
	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);

		rect.left = x;
		rect.top = y;
		rect.right = x + rectWidth;
		rect.bottom = y + 20;

		SetTextColor(hDC, colors[rgbnum++]);
		DrawText(hDC, lpOut[0], lstrlen(lpOut[0]), &rect, DT_LEFT | DT_TOP | DT_WORDBREAK);

		rect.left = x;
		rect.top = y;
		rect.right = x + 15;
		rect.bottom = y + rectHeight;

		SetTextColor(hDC, colors[rgbnum++]);
		DrawText(hDC, lpOut[1], lstrlen(lpOut[1]), &rect, DT_LEFT | DT_TOP | DT_WORDBREAK | DT_EDITCONTROL);

		rect.left = x + rectWidth;
		rect.top = y;
		rect.right = x + rectWidth + 15;
		rect.bottom = y + rectHeight;

		SetTextColor(hDC, colors[rgbnum++]);
		DrawText(hDC, lpOut[2], lstrlen(lpOut[2]), &rect, DT_LEFT | DT_TOP | DT_WORDBREAK | DT_EDITCONTROL);

		rect.left = x;
		rect.top = y + rectHeight;
		rect.right = x + rectWidth;
		rect.bottom = y + rectHeight + 20;

		SetTextColor(hDC, colors[rgbnum++]);
		DrawText(hDC, lpOut[3], lstrlen(lpOut[3]), &rect, DT_LEFT | DT_TOP | DT_WORDBREAK);

		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}