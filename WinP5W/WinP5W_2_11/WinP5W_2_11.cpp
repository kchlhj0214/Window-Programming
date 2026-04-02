#include <windows.h>
#include <tchar.h>
#include <random>
#include <vector>
#include <cmath>

using namespace std;
random_device rd;
mt19937 g(rd());
uniform_int_distribution<> uid{ 0, 3 };
uniform_int_distribution<> uid_rgb{ 0, 255 };
#define LEN 900
#define HEI 700

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
	HPEN hPen, oldPen;
	HBRUSH hBrush, oldBrush;
	RECT rt = { 350, 225, 550, 375 };
	static vector<COLORREF> colors(5);
	int centerX = 450;
	int centerY = 300;
								// 상			우			하			좌
	static POINT posCenters[4] = { {450, 150}, {650, 300}, {450, 450}, {250, 300} };

	static POINT sandGlass[4] = { {-50, -50}, {50, 50}, {-50, 50}, {50, -50} };
	static POINT pentagon[5] = { {0, -50}, {50, -10}, {30, 50}, {-30, 50}, {-50, -10} };
	static POINT pie[4] = { {-50, -50}, {50, 50}, {0, -50}, {50, 0} };
	static POINT ellipse[2] = { {-50, -50}, {50, 50} };

	static POINT sandGlass_in[4] = { {400, 250}, {500, 350}, {500, 250}, {400, 350} };
	static POINT pentagon_in[5] = { {450, 350}, {500, 310}, {480, 250}, {420, 250}, {400, 310} };
	static POINT pie_in[4] = { {400, 250}, {500, 350}, {500, 300}, {450, 250} };
	static POINT ellipse_in[2] = { {380, 270}, {520, 330} };

	static vector<int> polygonPos(4);		// [0] 모래시계 [1] 오각형 [2] 파이 [3] 원 // 0 상 1 우 2 하 3 좌

	static bool wShow = false;				// w 모래시계 / d 오각형 / s 파이 / a 원
	static bool dShow = false;
	static bool sShow = false;
	static bool aShow = false;
	static int mShow = uid(g);

	switch (uMsg) {
	case WM_CREATE:
		for (int i = 0; i < 4; ++i)		// 색 초기값
			colors[i] = (RGB(uid_rgb(g), uid_rgb(g), uid_rgb(g)));
		for (int i = 0; i < 4; ++i)
			polygonPos[i] = i;
		break;
	case WM_KEYDOWN:
		hDC = GetDC(hWnd);
		
		if (wParam == 'W') {
			dShow = false;
			sShow = false;
			aShow = false;
			if (!wShow) {
				wShow = true;
				mShow = 0;
				colors[4] = RGB(uid_rgb(g), uid_rgb(g), uid_rgb(g));
				InvalidateRect(hWnd, NULL, TRUE);
			}
		}
		else if (wParam == 'D') {
			wShow = false;
			sShow = false;
			aShow = false;
			if (!dShow) {
				dShow = true;
				mShow = 1;
				colors[4] = RGB(uid_rgb(g), uid_rgb(g), uid_rgb(g));
				InvalidateRect(hWnd, NULL, TRUE);
			}
		}
		else if (wParam == 'S') {
			wShow = false;
			dShow = false;
			aShow = false;
			if (!sShow) {
				sShow = true;
				mShow = 2;
				colors[4] = RGB(uid_rgb(g), uid_rgb(g), uid_rgb(g));
				InvalidateRect(hWnd, NULL, TRUE);
			}
		}
		else if (wParam == 'A') {
			wShow = false;
			dShow = false;
			sShow = false;
			if (!aShow) {
				aShow = true;
				mShow = 3;
				colors[4] = RGB(uid_rgb(g), uid_rgb(g), uid_rgb(g));
				InvalidateRect(hWnd, NULL, TRUE);
			}
		}
		else if (wParam == VK_UP) {
			wShow = false;
			dShow = false;
			sShow = false;
			aShow = false;
			for (int i = 0; i < 4; ++i) {
				if (polygonPos[i] == 0)
					polygonPos[i] = 2;
				else if(polygonPos[i] == 2)
					polygonPos[i] = 0;
			}
			for (int i = 0; i < 4; ++i) {
				if (polygonPos[i] == 0) {
					mShow = i;
					break;
				}
			}
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == VK_DOWN) {
			wShow = false;
			dShow = false;
			sShow = false;
			aShow = false;
			for (int i = 0; i < 4; ++i) {
				if (polygonPos[i] == 1)
					polygonPos[i] = 3;
				else if (polygonPos[i] == 3)
					polygonPos[i] = 1;
			}
			for (int i = 0; i < 4; ++i) {
				if (polygonPos[i] == 0) {
					mShow = i;
					break;
				}
			}
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == VK_LEFT) {
			wShow = false;
			dShow = false;
			sShow = false;
			aShow = false;
			for (int i = 0; i < 4; ++i) {
				if (polygonPos[i] == 0)
					polygonPos[i] = 3;
				else
					--polygonPos[i];
			}
			for (int i = 0; i < 4; ++i) {
				if (polygonPos[i] == 0) {
					mShow = i;
					break;
				}
			}
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == VK_RIGHT) {
			wShow = false;
			dShow = false;
			sShow = false;
			aShow = false;
			for (int i = 0; i < 4; ++i) {
				if (polygonPos[i] == 3)
					polygonPos[i] = 0;
				else
					++polygonPos[i];
			}
			for (int i = 0; i < 4; ++i) {
				if (polygonPos[i] == 0) {
					mShow = i;
					break;
				}
			}
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == VK_ESCAPE || wParam == 'Q') {
			PostQuitMessage(0);
		}

		break;
	case WM_KEYUP:
		hDC = GetDC(hWnd);

		if (wParam == 'W') {
			wShow = false;
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == 'D') {
			dShow = false;
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == 'S') {
			sShow = false;
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == 'A') {
			aShow = false;
			InvalidateRect(hWnd, NULL, TRUE);
		}

		break;
	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);

		hPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
		oldPen = (HPEN)SelectObject(hDC, hPen);
		Rectangle(hDC, rt.left, rt.top, rt.right, rt.bottom);
		SelectObject(hDC, oldPen);
		DeleteObject(hPen);
		//-------------------------------------사각형 자리 찾아가기
		for (int i = 0; i < 4; ++i) {
			sandGlass[i].x += posCenters[polygonPos[0]].x;
			sandGlass[i].y += posCenters[polygonPos[0]].y;
		}
		for (int i = 0; i < 5; ++i) {
			pentagon[i].x += posCenters[polygonPos[1]].x;
			pentagon[i].y += posCenters[polygonPos[1]].y;
		}
		for (int i = 0; i < 4; ++i) {
			pie[i].x += posCenters[polygonPos[2]].x;
			pie[i].y += posCenters[polygonPos[2]].y;
		}
		for (int i = 0; i < 2; ++i) {
			ellipse[i].x += posCenters[polygonPos[3]].x;
			ellipse[i].y += posCenters[polygonPos[3]].y;
		}
		//---------------------------------------
		if (!wShow) {
			hBrush = CreateSolidBrush(colors[0]);
			oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
			Polygon(hDC, sandGlass, 4);
			SelectObject(hDC, oldBrush);
			DeleteObject(hBrush);
		}
		else {
			hBrush = CreateSolidBrush(colors[4]);
			oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
			Polygon(hDC, sandGlass, 4);
			SelectObject(hDC, oldBrush);
			DeleteObject(hBrush);
		}

		if (!dShow) {
			hBrush = CreateSolidBrush(colors[1]);
			oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
			Polygon(hDC, pentagon, 5);
			SelectObject(hDC, oldBrush);
			DeleteObject(hBrush);
		}
		else {
			hBrush = CreateSolidBrush(colors[4]);
			oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
			Polygon(hDC, pentagon, 5);
			SelectObject(hDC, oldBrush);
			DeleteObject(hBrush);
		}

		if (!sShow) {
			hBrush = CreateSolidBrush(colors[2]);
			oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
			Pie(hDC, pie[0].x, pie[0].y, pie[1].x, pie[1].y, pie[2].x, pie[2].y, pie[3].x, pie[3].y);
			SelectObject(hDC, oldBrush);
			DeleteObject(hBrush);
		}
		else {
			hBrush = CreateSolidBrush(colors[4]);
			oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
			Pie(hDC, pie[0].x, pie[0].y, pie[1].x, pie[1].y, pie[2].x, pie[2].y, pie[3].x, pie[3].y);
			SelectObject(hDC, oldBrush);
			DeleteObject(hBrush);
		}

		if (!aShow) {
			hBrush = CreateSolidBrush(colors[3]);
			oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
			Ellipse(hDC, ellipse[0].x, ellipse[0].y, ellipse[1].x, ellipse[1].y);
			SelectObject(hDC, oldBrush);
			DeleteObject(hBrush);
		}
		else {
			hBrush = CreateSolidBrush(colors[4]);
			oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
			Ellipse(hDC, ellipse[0].x, ellipse[0].y, ellipse[1].x, ellipse[1].y);
			SelectObject(hDC, oldBrush);
			DeleteObject(hBrush);
		}

		switch (mShow) {	// 가운데 도형 출력
		case 0:
			if (!wShow) {
				hBrush = CreateSolidBrush(colors[0]);
				oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
				Polygon(hDC, sandGlass_in, 4);
				SelectObject(hDC, oldBrush);
				DeleteObject(hBrush);
			}
			else {
				hBrush = CreateSolidBrush(colors[4]);
				oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
				Polygon(hDC, sandGlass_in, 4);
				SelectObject(hDC, oldBrush);
				DeleteObject(hBrush);
			}
			break;
		case 1:
			if (!dShow) {
				hBrush = CreateSolidBrush(colors[1]);
				oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
				Polygon(hDC, pentagon_in, 5);
				SelectObject(hDC, oldBrush);
				DeleteObject(hBrush);
			}
			else {
				hBrush = CreateSolidBrush(colors[4]);
				oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
				Polygon(hDC, pentagon_in, 5);
				SelectObject(hDC, oldBrush);
				DeleteObject(hBrush);
			}
			break;
		case 2:
			if (!sShow) {
				hBrush = CreateSolidBrush(colors[2]);
				oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
				Pie(hDC, pie_in[0].x, pie_in[0].y, pie_in[1].x, pie_in[1].y, pie_in[2].x, pie_in[2].y, pie_in[3].x, pie_in[3].y);
				SelectObject(hDC, oldBrush);
				DeleteObject(hBrush);
			}
			else {
				hBrush = CreateSolidBrush(colors[4]);
				oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
				Pie(hDC, pie_in[0].x, pie_in[0].y, pie_in[1].x, pie_in[1].y, pie_in[2].x, pie_in[2].y, pie_in[3].x, pie_in[3].y);
				SelectObject(hDC, oldBrush);
				DeleteObject(hBrush);
			}
			break;
		case 3:
			if (!aShow) {
				hBrush = CreateSolidBrush(colors[3]);
				oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
				Ellipse(hDC, ellipse_in[0].x, ellipse_in[0].y, ellipse_in[1].x, ellipse_in[1].y);
				SelectObject(hDC, oldBrush);
				DeleteObject(hBrush);
			}
			else {
				hBrush = CreateSolidBrush(colors[4]);
				oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
				Ellipse(hDC, ellipse_in[0].x, ellipse_in[0].y, ellipse_in[1].x, ellipse_in[1].y);
				SelectObject(hDC, oldBrush);
				DeleteObject(hBrush);
			}
			break;
		}

		//-------------------------------------사각형 자리 원복하기
		for (int i = 0; i < 4; ++i) {
			sandGlass[i].x -= posCenters[polygonPos[0]].x;
			sandGlass[i].y -= posCenters[polygonPos[0]].y;
		}
		for (int i = 0; i < 5; ++i) {
			pentagon[i].x -= posCenters[polygonPos[1]].x;
			pentagon[i].y -= posCenters[polygonPos[1]].y;
		}
		for (int i = 0; i < 4; ++i) {
			pie[i].x -= posCenters[polygonPos[2]].x;
			pie[i].y -= posCenters[polygonPos[2]].y;
		}
		for (int i = 0; i < 2; ++i) {
			ellipse[i].x -= posCenters[polygonPos[3]].x;
			ellipse[i].y -= posCenters[polygonPos[3]].y;
		}
		//---------------------------------------

		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}