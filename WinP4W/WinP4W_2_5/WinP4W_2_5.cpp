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
	static TCHAR str[5][21] = {0};
	static int count = 0;
	static int yPos = 0;
	static SIZE size;
	static int line = 0;
	static int x, y;
	static BYTE r_c, g_c, b_c;

	

	switch (uMsg) {
	case WM_CREATE:
		x = uid(g);
		y = uid(g) % 500;
		r_c = (BYTE)uid2(g2);
		g_c = (BYTE)uid2(g2);
		b_c = (BYTE)uid2(g2);
		CreateCaret(hWnd, NULL, 2, 15);
		ShowCaret(hWnd);
		break;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) {
			PostQuitMessage(0);
		}
		break;
	case WM_CHAR:
		hDC = GetDC(hWnd);

		if (wParam == VK_BACK) {
			if (count > 0){
				--count;
				str[line][count] = '\0';
			}
			else if(line > 0){
				--line;
				count = lstrlen(str[line]);
			}
		}
		else if (wParam == VK_RETURN) {
			if (yPos + ((line + 1) * 20) + y < HEI - 50)
				yPos += 20;
		}
		else {
			if (count < 20) {
				str[line][count++] = wParam;
				str[line][count] = '\0';
				
			}
			else {
				if (line < 4 && yPos + ((line + 1) * 20) + y < HEI - 50) {	// yPos + ((line + 1) * 20) + y < HEI - 50 이 조건이 없다면 현재 입력된 문자열의 끝 줄이 맨 아래까지 이동하더라도 총 줄의 수가 5보다 작다면 화면 밖에 문자열을 추가함
					count = 0;
					++line;
					str[line][count++] = wParam;
					str[line][count] = '\0';
				}
			}
		}

		str[line][count] = '\0';
		InvalidateRect(hWnd, NULL, TRUE);
		break;
	case WM_PAINT:
			hDC = BeginPaint(hWnd, &ps);
			SetTextColor(hDC, RGB(r_c, g_c, b_c));
			for (int i = 0; i <= line; ++i) {
				TextOut(hDC, x, yPos + (i * 20) + y, str[i], lstrlen(str[i]));
			}
			GetTextExtentPoint32(hDC, str[line], lstrlen(str[line]), &size);
			SetCaretPos(size.cx + x, yPos + y + (line * 20));
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