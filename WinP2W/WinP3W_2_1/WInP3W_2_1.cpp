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
	TCHAR lpOut[100];
	int num;
	vector<int> posX;
	vector<int> posY;
	posX.push_back(400);
	posY.push_back(300);
	int x, y;
	int crt{};

	for (int i = 1; i < 16; ++i) {
		x = uid(g);
		y = uid(g);
		for (auto& a : posX) {
			if (x >= a - 100 && x < a + 100) {
				++crt;
				break;
			}
		}
		for (auto& a : posY) {
			if ((y >= a - 20 && y < a + 20) || y > 500) {
				++crt;
				break;
			}
		}
		if (crt != 2) {
			crt = 0;
			posX.push_back(x);
			posY.push_back(y);
		}
		else {
			crt = 0;
			--i;
		}
	}

	switch (uMsg) {
	case WM_CREATE:
		break;
	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);

		for (int i = 0; i < 16; ++i) {
			wsprintf(lpOut, L"%d: (%d, %d)", i, posX[i], posY[i]);
			TextOut(hDC, posX[i], posY[i], lpOut, lstrlen(lpOut));
		}

		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}