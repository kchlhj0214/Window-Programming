#include <windows.h>
#include <tchar.h>
#include <random>
#include <vector>
#include <cmath>

using namespace std;
random_device rd;
mt19937 g(rd());
uniform_int_distribution<> uid_shape{ 0, 2 };
uniform_int_distribution<> uid_rgb{ 0, 255 };
uniform_int_distribution<> uid_drawBoard{ 0, 39 };
uniform_int_distribution<> uid_size{ -3, 3 };
#define LEN 1000
#define HEI 1000

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

typedef struct {
	int shape;
	int size;
	int x;
	int y;
	COLORREF color;
}SHAPES;

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC;
	HPEN hPen, oldPen;
	HBRUSH hBrush, oldBrush;
	TCHAR str[50];
	static int m;
	static int offsetX, offsetY;

	static vector<SHAPES> shapes(15);
	static int board_state[40][40] = { 0 };
	static vector<int> initShape(10);
	static vector<COLORREF> initColor(10);
	static int selected = 0;

	static POINT triangle[3] = { {0, -10}, {-10, 10}, {10, 10} };
	static POINT rect[2] = { {-10, -10}, {10, 10} };
	static POINT ellipse[2] = { {-10, -10}, {10, 10} };
	static POINT temptriangle[3];
	static POINT temprect[2];
	static POINT tempellipse[2];

	switch (uMsg) {
	case WM_CREATE:
		shapes.clear();
		break;

	case WM_KEYDOWN:
		if ((wParam == VK_UP || wParam == VK_LEFT || wParam == VK_DOWN || wParam == VK_RIGHT) && (selected > 0 && selected <= shapes.size())) {
			if (wParam == VK_UP) {
				if (shapes[selected - 1].y > 0)
					--shapes[selected - 1].y;
				else if(shapes[selected - 1].y == 0)
					shapes[selected - 1].y = 39;
			} 
			else if (wParam == VK_LEFT) {
				if (shapes[selected - 1].x > 0)
					--shapes[selected - 1].x;
				else if(shapes[selected - 1].x == 0)
					shapes[selected - 1].x = 39;
			}
			else if (wParam == VK_DOWN) {
				if (shapes[selected - 1].y < 39)
					++shapes[selected - 1].y;
				else if (shapes[selected - 1].y == 39)
					shapes[selected - 1].y = 0;
			}
			else if (wParam == VK_RIGHT) {
				if (shapes[selected - 1].x < 39)
					++shapes[selected - 1].x;
				else if (shapes[selected - 1].x == 39)
					shapes[selected - 1].x = 0;
			}

			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == 'T') {
			if (shapes.size() == 10) {
				shapes.erase(shapes.begin());
				initColor.erase(initColor.begin());
				initShape.erase(initShape.begin());
			}
			SHAPES s;
			s.color = uid_rgb(g);
			initColor.push_back(s.color);
			s.shape = 0;
			initShape.push_back(s.shape);
			s.size = 10;
			s.x = uid_drawBoard(g);
			s.y = uid_drawBoard(g);
			shapes.push_back(s);
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == 'R') {
			if (shapes.size() == 10) {
				shapes.erase(shapes.begin());
				initColor.erase(initColor.begin());
				initShape.erase(initShape.begin());
			}
			SHAPES s;
			s.color = uid_rgb(g);
			initColor.push_back(s.color);
			s.shape = 1;
			initShape.push_back(s.shape);
			s.size = 10;
			s.x = uid_drawBoard(g);
			s.y = uid_drawBoard(g);
			shapes.push_back(s);
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == 'E') {
			if (shapes.size() == 10) {
				shapes.erase(shapes.begin());
				initColor.erase(initColor.begin());
				initShape.erase(initShape.begin());
			}
			SHAPES s;
			s.color = uid_rgb(g);
			initColor.push_back(s.color);
			s.shape = 2;
			initShape.push_back(s.shape);
			s.size = 10;
			s.x = uid_drawBoard(g);
			s.y = uid_drawBoard(g);
			shapes.push_back(s);
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == '1') {
			selected = 1;
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == '2') {
			selected = 2;
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == '3') {
			selected = 3;
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == '4') {
			selected = 4;
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == '5') {
			selected = 5;
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == '6') {
			selected = 6;
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == '7') {
			selected = 7;
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == '8') {
			selected = 8;
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == '9') {
			selected = 9;
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == '0') {
			selected = 10;
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == VK_ESCAPE || wParam == 'Q') {
			PostQuitMessage(0);
		}
		else if ((wParam == VK_OEM_PLUS || wParam == VK_ADD) && (selected > 0 && selected <= shapes.size())) {
			++shapes[selected - 1].size;
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if ((wParam == VK_OEM_MINUS || wParam == VK_SUBTRACT) && (selected > 0 && selected <= shapes.size())) {
			if (shapes[selected - 1].size > 0)
				--shapes[selected - 1].size;
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == 'C') {
			
		}
		else if (wParam == 'D') {
			if (selected > 0 && selected <= shapes.size()) {
				shapes.erase(shapes.begin() + selected - 1);
				selected = 0;
			}
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == 'P') {
			shapes.clear();
			selected = 0;
			InvalidateRect(hWnd, NULL, TRUE);
		}
		break;


	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);
		//-----------------------------기본 판 그리기
		hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
		oldPen = (HPEN)SelectObject(hDC, hPen);
		for (int i = 0; i < 40; ++i) {
			for (int j = 0; j < 40; ++j) {
				Rectangle(hDC, 20 + 20 * j, 20 + 20 * i, 40 + 20 * j, 40 + 20 * i);
			}
		}
		SelectObject(hDC, oldPen);
		DeleteObject(hPen);

		//-----------------------------도형 그리기
		for (int i = 0; i < shapes.size(); ++i) {
			if (i != selected - 1) {
				hPen = CreatePen(PS_NULL, 0, RGB(0, 0, 0));
				oldPen = (HPEN)SelectObject(hDC, hPen);
				hBrush = CreateSolidBrush(shapes[i].color);
				oldBrush = (HBRUSH)SelectObject(hDC, hBrush);

				offsetX = 30 + shapes[i].x * 20;
				offsetY = 30 + shapes[i].y * 20;

				if (shapes[i].shape == 0) {
					for (int k = 0; k < 3; ++k) {
						temptriangle[k].x = (int)(triangle[k].x * shapes[i].size / 10.0) + offsetX;
						temptriangle[k].y = (int)(triangle[k].y * shapes[i].size / 10.0) + offsetY;
					}
					Polygon(hDC, temptriangle, 3);
				}
				else if (shapes[i].shape == 1) {
					temprect[0].x = (int)(rect[0].x * shapes[i].size / 10.0) + offsetX;
					temprect[0].y = (int)(rect[0].y * shapes[i].size / 10.0) + offsetY;
					temprect[1].x = (int)(rect[1].x * shapes[i].size / 10.0) + offsetX;
					temprect[1].y = (int)(rect[1].y * shapes[i].size / 10.0) + offsetY;
					Rectangle(hDC, temprect[0].x, temprect[0].y, temprect[1].x, temprect[1].y);
				}
				else if (shapes[i].shape == 2) {
					tempellipse[0].x = (int)(ellipse[0].x * shapes[i].size / 10.0) + offsetX;
					tempellipse[0].y = (int)(ellipse[0].y * shapes[i].size / 10.0) + offsetY;
					tempellipse[1].x = (int)(ellipse[1].x * shapes[i].size / 10.0) + offsetX;
					tempellipse[1].y = (int)(ellipse[1].y * shapes[i].size / 10.0) + offsetY;
					Ellipse(hDC, tempellipse[0].x, tempellipse[0].y, tempellipse[1].x, tempellipse[1].y);
				}
				SelectObject(hDC, oldBrush);
				DeleteObject(hBrush);
				SelectObject(hDC, oldPen);
				DeleteObject(hPen);
			}
		}
		//----------------------------------------------------------------
		if (selected > 0 && selected <= shapes.size()) {
			hPen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
			oldPen = (HPEN)SelectObject(hDC, hPen);
			hBrush = CreateSolidBrush(shapes[selected - 1].color);
			oldBrush = (HBRUSH)SelectObject(hDC, hBrush);

			offsetX = 30 + shapes[selected - 1].x * 20;
			offsetY = 30 + shapes[selected - 1].y * 20;

			if (shapes[selected - 1].shape == 0) {
				for (int k = 0; k < 3; ++k) {
					temptriangle[k].x = (int)(triangle[k].x * shapes[selected - 1].size / 10.0) + offsetX;
					temptriangle[k].y = (int)(triangle[k].y * shapes[selected - 1].size / 10.0) + offsetY;
				}
				Polygon(hDC, temptriangle, 3);
			}
			else if (shapes[selected - 1].shape == 1) {
				temprect[0].x = (int)(rect[0].x * shapes[selected - 1].size / 10.0) + offsetX;
				temprect[0].y = (int)(rect[0].y * shapes[selected - 1].size / 10.0) + offsetY;
				temprect[1].x = (int)(rect[1].x * shapes[selected - 1].size / 10.0) + offsetX;
				temprect[1].y = (int)(rect[1].y * shapes[selected - 1].size / 10.0) + offsetY;
				Rectangle(hDC, temprect[0].x, temprect[0].y, temprect[1].x, temprect[1].y);
			}
			else if (shapes[selected - 1].shape == 2) {
				tempellipse[0].x = (int)(ellipse[0].x * shapes[selected - 1].size / 10.0) + offsetX;
				tempellipse[0].y = (int)(ellipse[0].y * shapes[selected - 1].size / 10.0) + offsetY;
				tempellipse[1].x = (int)(ellipse[1].x * shapes[selected - 1].size / 10.0) + offsetX;
				tempellipse[1].y = (int)(ellipse[1].y * shapes[selected - 1].size / 10.0) + offsetY;
				Ellipse(hDC, tempellipse[0].x, tempellipse[0].y, tempellipse[1].x, tempellipse[1].y);
			}
			SelectObject(hDC, oldBrush);
			DeleteObject(hBrush);
			SelectObject(hDC, oldPen);
			DeleteObject(hPen);
		}

		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
