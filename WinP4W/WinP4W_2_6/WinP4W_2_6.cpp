#include <windows.h>
#include <tchar.h>
#include <random>
#include <vector>

using namespace std;
#define LEN 800
#define HEI 600
random_device rd;
mt19937 g(rd());
uniform_int_distribution<> uid{ 0, LEN - 200 };		// 800 600 기준 12단까지
uniform_int_distribution<> uid2{ 0, 255 };


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

struct NumRect {
	int x, y, n, count;
};

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC;
	static TCHAR str[20] = { 0 };
	static TCHAR lpOut[20] = { 0 };
	static vector<COLORREF> colors;
	int rgbnum = 0;
	static int idx = 0;
	static NumRect rects[10];
	static int rectCount = 0;
	static BYTE r_c, g_c, b_c;
	static int prtChoice = 0;		// 0 = e일경우(단일 출력), 1 = a일경우(모두 출력), 2 = 엔터일경우(출력 없이 첫 대기화면으로)

	switch (uMsg) {
	case WM_CREATE:
		for (int i = 0; i < 30; ++i)
			colors.push_back(RGB(uid2(g), uid2(g), uid2(g)));
		CreateCaret(hWnd, NULL, 2, 15);
		ShowCaret(hWnd);
		break;
	case WM_CHAR:
		hDC = GetDC(hWnd);

		if (wParam == 'e') {					// 4개 숫자 입력 후 사각형 생성
			if (rectCount < 10) {
				int tx, ty, tn, tc;
				if (_stscanf_s(str, _T("%d %d %d %d"), &tx, &ty, &tn, &tc) == 4) {
					rects[rectCount].x = tx;
					rects[rectCount].y = ty;
					rects[rectCount].n = tn;
					rects[rectCount].count = tc;
					++rectCount;
					prtChoice = 0;
				}
			}
			idx = 0;
			memset(str, 0, sizeof(str));
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == VK_RETURN) {			// 기존 사각형 지운 후 새롭게 4개 숫자 입력대기
			prtChoice = 2;
			idx = 0;
			memset(str, 0, sizeof(str));
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == 'q') {				// 프로그램 종료
			PostQuitMessage(0);
		}
		else if (wParam == 'a') {				// 그동안 출력한 문자들 전부 출력
			prtChoice = 1;
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == 'r') {				// 모두 리셋하고 처음부터 시작
			rectCount = 0;
			prtChoice = 2;
			rgbnum = 0;
			for (int i = 0; i < 30; ++i)
				colors.push_back(RGB(uid2(g), uid2(g), uid2(g)));
			idx = 0;
			memset(str, 0, sizeof(str));
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else { // 일반 문자 입력
			if (idx < 20) {
				str[idx++] = (TCHAR)wParam;
				str[idx] = '\0';
			}
		}
		InvalidateRect(hWnd, NULL, TRUE);
		break;
	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);
		
		if (rectCount > 0) {
			if (prtChoice == 1) {
				for (int i = 0; i < rectCount - 1; i++) {
					SetBkMode(hDC, OPAQUE);
					SetBkColor(hDC, colors[rgbnum++]);
					SetTextColor(hDC, colors[rgbnum++]);

					for (int j = 0; j < rects[i].count; ++j) 
						wsprintf(lpOut, L"%d", rects[i].n);

					for (int j = 0; j < rects[i].count; ++j) 
						TextOut(hDC, rects[i].x, rects[i].y, lpOut, lstrlen(lpOut));
				}
			}
			TextOut(hDC, rects[rectCount].x, rects[rectCount].y, lpOut, lstrlen(lpOut));
		}

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