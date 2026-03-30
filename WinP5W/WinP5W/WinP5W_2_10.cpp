#include <windows.h>
#include <tchar.h>
#include <random>
#include <vector>
#include <cmath>

using namespace std;
random_device rd;
mt19937 g(rd());
uniform_int_distribution<> uid_tx{ 50, 850 };
uniform_int_distribution<> uid_ty{ 50, 850 };
uniform_int_distribution<> uid_rx{ 150, 750 };
uniform_int_distribution<> uid_ry{ 100, 500 };
uniform_int_distribution<> uid_ex{ 300, 600 };
uniform_int_distribution<> uid_ey{ 200, 400 };
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
	RECT win = { 0, 0, 800, 580 };
	RECT rt = { 150, 100, 750, 500 };
	RECT rt2 = { 300, 200, 600, 400 };
	static vector<COLORREF> colors;

	static vector<POINT> trianglePoints(18);
	static vector<POINT> rectPoints(8);
	static vector<POINT> ellipsePoints(4);
	static int triangle_num = 0, rect_num = 0, ellipse_num = 0;;

	switch (uMsg) {
	case WM_CREATE:
		for (int i = 0; i < 3; ++i)		// 색 초기값
			colors.push_back(RGB(uid_rgb(g), uid_rgb(g), uid_rgb(g)));

		while (triangle_num < 18) {
			int centerX = uid_tx(g);
			int centerY = uid_ty(g);

			POINT p1, p2, p3;
			p1.x = centerX;
			p1.y = centerY - 30;
			p2.x = centerX - 20;
			p2.y = centerY + 30;
			p3.x = centerX + 20;
			p3.y = centerY + 30;

			if (!(PtInRect(&win, p1) || PtInRect(&win, p2) || PtInRect(&win, p3)))		// 윈도우창 내부로 범위 제한
				continue;
			if (PtInRect(&rt, p1) || PtInRect(&rt, p2) || PtInRect(&rt, p3))			// rt 외부로 범위 제한
				continue;
			bool isOverlap = false;
			for (int i = 0; i < triangle_num; i += 3) {									// 절댓값을 이용해 삼각형 겹침 방지
				if (abs(p1.x - trianglePoints[i].x) < 50 && abs(p1.y - trianglePoints[i].y) < 70) {
					isOverlap = true;
					break;
				}
			}

			if (isOverlap) continue;

			trianglePoints[triangle_num] = p1;
			trianglePoints[triangle_num + 1] = p2;
			trianglePoints[triangle_num + 2] = p3;

			triangle_num += 3;
		}

		while (rect_num < 8) {
			POINT p1, p2;
			p1.x = uid_rx(g);
			p1.y = uid_ry(g);
			p2.x = p1.x + 35;
			p2.y = p1.y + 45;

			RECT newRect = { p1.x, p1.y, p2.x, p2.y };
			RECT temp;

			if (IntersectRect(&temp, &newRect, &rt2))
				continue;

			if (!(PtInRect(&rt, p1) && PtInRect(&rt, p2)))
				continue;

			bool isOverlap = false;
			for (int i = 0; i < rect_num; i += 2) {
				if (abs(p1.x - rectPoints[i].x) < 40 && abs(p1.y - rectPoints[i].y) < 50) {
					isOverlap = true;
					break;
				}
			}

			if (isOverlap) continue;
			rectPoints[rect_num] = p1;
			rectPoints[rect_num + 1] = p2;

			rect_num += 2;
		}

		while (ellipse_num < 4) {
			POINT p1, p2;
			p1.x = uid_ex(g);
			p1.y = uid_ey(g);
			p2.x = p1.x + 45;
			p2.y = p1.y + 45;

			RECT newRect = { p1.x, p1.y, p2.x, p2.y };
			RECT temp;

			if (!(PtInRect(&rt2, p1) && PtInRect(&rt2, p2)))
				continue;

			bool isOverlap = false;
			for (int i = 0; i < ellipse_num; i += 2) {
				if (abs(p1.x - ellipsePoints[i].x) < 50 && abs(p1.y - ellipsePoints[i].y) < 50) {
					isOverlap = true;
					break;
				}
			}

			if (isOverlap) continue;
			ellipsePoints[ellipse_num] = p1;
			ellipsePoints[ellipse_num + 1] = p2;

			ellipse_num += 2;
		}

		break;
	case WM_CHAR:
		hDC = GetDC(hWnd);

		if (wParam == VK_RETURN) {
			triangle_num = 0;		// 위치 변환을 위해 생성 사이클 초기화
			rect_num = 0;
			ellipse_num = 0;

			for (int i = 0; i < 3; ++i)
				colors[i] = RGB(uid_rgb(g), uid_rgb(g), uid_rgb(g));

			while (triangle_num < 18) {
				int centerX = uid_tx(g);
				int centerY = uid_ty(g);

				POINT p1, p2, p3;
				p1.x = centerX;
				p1.y = centerY - 30;
				p2.x = centerX - 20;
				p2.y = centerY + 30;
				p3.x = centerX + 20;
				p3.y = centerY + 30;

				if (!(PtInRect(&win, p1) || PtInRect(&win, p2) || PtInRect(&win, p3)))
					continue;
				if (PtInRect(&rt, p1) || PtInRect(&rt, p2) || PtInRect(&rt, p3))
					continue;
				bool isOverlap = false;
				for (int i = 0; i < triangle_num; i += 3) {
					if (abs(p1.x - trianglePoints[i].x) < 60 && abs(p1.y - trianglePoints[i].y) < 80) {
						isOverlap = true;
						break;
					}
				}

				if (isOverlap) continue;

				trianglePoints[triangle_num] = p1;
				trianglePoints[triangle_num + 1] = p2;
				trianglePoints[triangle_num + 2] = p3;

				triangle_num += 3;
			}

			while (rect_num < 8) {
				POINT p1, p2;
				p1.x = uid_rx(g);
				p1.y = uid_ry(g);
				p2.x = p1.x + 35;
				p2.y = p1.y + 45;

				RECT newRect = { p1.x, p1.y, p2.x, p2.y };
				RECT temp;

				if (IntersectRect(&temp, &newRect, &rt2))
					continue;

				if (!(PtInRect(&rt, p1) && PtInRect(&rt, p2)))
					continue;

				bool isOverlap = false;
				for (int i = 0; i < rect_num; i += 2) {
					if (abs(p1.x - rectPoints[i].x) < 40 && abs(p1.y - rectPoints[i].y) < 50) {
						isOverlap = true;
						break;
					}
				}

				if (isOverlap) continue;
				rectPoints[rect_num] = p1;
				rectPoints[rect_num + 1] = p2;

				rect_num += 2;
			}

			while (ellipse_num < 4) {
				POINT p1, p2;
				p1.x = uid_ex(g);
				p1.y = uid_ey(g);
				p2.x = p1.x + 45;
				p2.y = p1.y + 45;

				RECT newRect = { p1.x, p1.y, p2.x, p2.y };
				RECT temp;

				if (!(PtInRect(&rt2, p1) && PtInRect(&rt2, p2)))
					continue;

				bool isOverlap = false;
				for (int i = 0; i < ellipse_num; i += 2) {
					if (abs(p1.x - ellipsePoints[i].x) < 50 && abs(p1.y - ellipsePoints[i].y) < 50) {
						isOverlap = true;
						break;
					}
				}

				if (isOverlap) continue;
				ellipsePoints[ellipse_num] = p1;
				ellipsePoints[ellipse_num + 1] = p2;

				ellipse_num += 2;
			}

			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if(wParam == '1') {
			triangle_num = 0;
			colors[0] = RGB(uid_rgb(g), uid_rgb(g), uid_rgb(g));
			while (triangle_num < 18) {
				int centerX = uid_tx(g);
				int centerY = uid_ty(g);

				POINT p1, p2, p3;
				p1.x = centerX;
				p1.y = centerY - 30;
				p2.x = centerX - 20;
				p2.y = centerY + 30;
				p3.x = centerX + 20;
				p3.y = centerY + 30;

				if (!(PtInRect(&win, p1) || PtInRect(&win, p2) || PtInRect(&win, p3)))
					continue;
				if (PtInRect(&rt, p1) || PtInRect(&rt, p2) || PtInRect(&rt, p3))
					continue;
				bool isOverlap = false;
				for (int i = 0; i < triangle_num; i += 3) {
					if (abs(p1.x - trianglePoints[i].x) < 60 && abs(p1.y - trianglePoints[i].y) < 80) {
						isOverlap = true;
						break;
					}
				}

				if (isOverlap) continue;

				trianglePoints[triangle_num] = p1;
				trianglePoints[triangle_num + 1] = p2;
				trianglePoints[triangle_num + 2] = p3;

				triangle_num += 3;
			}
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == '2') {
			rect_num = 0;
			colors[1] = RGB(uid_rgb(g), uid_rgb(g), uid_rgb(g));
			while (rect_num < 8) {
				POINT p1, p2;
				p1.x = uid_rx(g);
				p1.y = uid_ry(g);
				p2.x = p1.x + 35;
				p2.y = p1.y + 45;

				RECT newRect = { p1.x, p1.y, p2.x, p2.y };
				RECT temp;

				if (IntersectRect(&temp, &newRect, &rt2))
					continue;

				if (!(PtInRect(&rt, p1) && PtInRect(&rt, p2)))
					continue;

				bool isOverlap = false;
				for (int i = 0; i < rect_num; i += 2) {
					if (abs(p1.x - rectPoints[i].x) < 40 && abs(p1.y - rectPoints[i].y) < 50) {
						isOverlap = true;
						break;
					}
				}

				if (isOverlap) continue;
				rectPoints[rect_num] = p1;
				rectPoints[rect_num + 1] = p2;

				rect_num += 2;
			}
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == '3') {
			ellipse_num = 0;
			colors[2] = RGB(uid_rgb(g), uid_rgb(g), uid_rgb(g));
			while (ellipse_num < 4) {
				POINT p1, p2;
				p1.x = uid_ex(g);
				p1.y = uid_ey(g);
				p2.x = p1.x + 45;
				p2.y = p1.y + 45;

				RECT newRect = { p1.x, p1.y, p2.x, p2.y };
				RECT temp;

				if (!(PtInRect(&rt2, p1) && PtInRect(&rt2, p2)))
					continue;

				bool isOverlap = false;
				for (int i = 0; i < ellipse_num; i += 2) {
					if (abs(p1.x - ellipsePoints[i].x) < 50 && abs(p1.y - ellipsePoints[i].y) < 50) {
						isOverlap = true;
						break;
					}
				}

				if (isOverlap) continue;
				ellipsePoints[ellipse_num] = p1;
				ellipsePoints[ellipse_num + 1] = p2;

				ellipse_num += 2;
			}
			InvalidateRect(hWnd, NULL, TRUE);
		}
		break;
	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);

		hPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
		oldPen = (HPEN)SelectObject(hDC, hPen);
		Rectangle(hDC, rt.left, rt.top, rt.right, rt.bottom);
		Rectangle(hDC, rt2.left, rt2.top, rt2.right, rt2.bottom);
		SelectObject(hDC, oldPen);
		DeleteObject(hPen);

		hBrush = CreateSolidBrush(colors[0]);
		oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
		for (int i = 0; i < 18; i += 3) 
			Polygon(hDC, &trianglePoints[i], 3);
		SelectObject(hDC, oldBrush);
		DeleteObject(hBrush);

		hBrush = CreateSolidBrush(colors[1]);
		oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
		for (int i = 0; i < 8; i += 2)
			Rectangle(hDC, rectPoints[i].x, rectPoints[i].y, rectPoints[i + 1].x, rectPoints[i + 1].y);
		SelectObject(hDC, oldBrush);
		DeleteObject(hBrush);

		hBrush = CreateSolidBrush(colors[2]);
		oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
		for (int i = 0; i < 4; i += 2)
			Ellipse(hDC, ellipsePoints[i].x, ellipsePoints[i].y, ellipsePoints[i + 1].x, ellipsePoints[i + 1].y);
		SelectObject(hDC, oldBrush);
		DeleteObject(hBrush);
	
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}