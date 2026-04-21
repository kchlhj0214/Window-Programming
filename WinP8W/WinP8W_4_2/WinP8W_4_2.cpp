#include <windows.h>
#include <tchar.h>
#include <random>
#include <vector>
#include <math.h>
#include "resource.h"

using namespace std;
random_device rd;
mt19937 g(rd());
uniform_int_distribution<> uid_movemode{ 1, 5 };
uniform_int_distribution<> uid_rgb{ 0, 255 };
uniform_int_distribution<> uid_speed{ 10, 30 };
uniform_int_distribution<> uid_color{ 2, 5 };
#define LEN 800
#define HEI 600
#define red_time 10000
#define blue_time 9000
#define yellow_time 1000
#define light_time 10.0f

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
	WndClass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	WndClass.lpfnWndProc = (WNDPROC)WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hInstance;
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
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
	int dir;
	int speed;
	int safety_distance;
	bool move;
	int x, y;
	int timer;
}CAR;

typedef struct {
	int state;
	int remain_time;
}LIGHT;

typedef struct {
	int x, y;
	int tx, ty;
	int tmpX, tmpY;
	bool isMoving;
	bool hasNext;
}MAN;

void init_setting(vector<CAR>& c, vector<LIGHT>& l) {
	for (int i = 0; i < 8; ++i) {
		c[i].dir = i % 4;
		c[i].speed = uid_speed(g);
		c[i].move = true;
		c[i].timer = i;
	}
	c[0].x = 500; c[0].y = 270;
	c[4].x = 650; c[4].y = 270;

	c[1].x = 430; c[1].y = 400;
	c[5].x = 430; c[5].y = 550;

	c[2].x = 300; c[2].y = 330;
	c[6].x = 150; c[6].y = 330;

	c[3].x = 370; c[3].y = 200;
	c[7].x = 370; c[7].y = 50;

	l[0].state = 2;
	l[0].remain_time = blue_time;
	l[1].state = 0;
	l[1].remain_time = red_time;
}

bool IsVerticalCarInCenter(const vector<CAR>& cars) {
	for (int i : {1, 3, 5, 7}) { 
		if (cars[i].x > 320 && cars[i].x < 480 && cars[i].y > 220 && cars[i].y < 380)
			return true;
	}
	return false;
}

bool IsHorizontalCarInCenter(const vector<CAR>& cars) {
	for (int i : {0, 2, 4, 6}) { 
		if (cars[i].x > 320 && cars[i].x < 480 && cars[i].y > 220 && cars[i].y < 380)
			return true;
	}
	return false;
}

void car_move(int idx, vector<CAR>& cars, vector<LIGHT>& lights, RECT rt, int crossing) {
	CAR& cur = cars[idx];

	int v_w = rt.right;
	if (v_w < 800) v_w = 800;

	int v_h = rt.bottom;
	if (v_h < 600) v_h = 600;

	int other_idx;

	if (idx < 4) other_idx = idx + 4;
	else other_idx = idx - 4;

	CAR& other = cars[other_idx];		// 앞 차 저장

	int d = 0;
	if (cur.dir == 0) d = cur.x - other.x;
	else if (cur.dir == 1) d = cur.y - other.y;
	else if (cur.dir == 2) d = other.x - cur.x;
	else if (cur.dir == 3) d = other.y - cur.y;

	if (d < 0) {
		if (cur.dir == 0 || cur.dir == 2) d = d + v_w;
		else d = d + v_h;
	}

	cur.safety_distance = d;

	int l_idx;
	if (cur.dir == 0 || cur.dir == 2) l_idx = 1; // 가로 신호등
	else l_idx = 0; // 세로 신호등

	bool enemyInCenter = false;
	if (cur.dir == 0 || cur.dir == 2) {
		enemyInCenter = IsVerticalCarInCenter(cars);
	}
	else {
		enemyInCenter = IsHorizontalCarInCenter(cars);
	}

	bool mustStop = false;
	if (crossing == 3) mustStop = true;
	else if (crossing == 1 && (cur.dir == 0 || cur.dir == 2)) mustStop = true;
	else if (crossing == 2 && (cur.dir == 1 || cur.dir == 3)) mustStop = true;
	else if (lights[l_idx].state == 0 || lights[l_idx].state == 1) { // 노란불
		if (cur.dir == 0 && (cur.x - 25) >= 460) mustStop = true;
		else if (cur.dir == 1 && (cur.y - 25) >= 360) mustStop = true;
		else if (cur.dir == 2 && (cur.x + 25) <= 340) mustStop = true;
		else if (cur.dir == 3 && (cur.y + 25) <= 240) mustStop = true;
	}

	if (!mustStop && enemyInCenter) {
		bool beforeEntry = false;
		if (cur.dir == 0 && cur.x > 480) beforeEntry = true;
		else if (cur.dir == 1 && cur.y > 380) beforeEntry = true;
		else if (cur.dir == 2 && cur.x < 320) beforeEntry = true;
		else if (cur.dir == 3 && cur.y < 220) beforeEntry = true;

		if (beforeEntry) {
			mustStop = true;
		}
	}

	if (mustStop) {
		int stop_d = 9999;
		if (cur.dir == 0) stop_d = cur.x - 460;
		else if (cur.dir == 1) stop_d = cur.y - 360;
		else if (cur.dir == 2) stop_d = 340 - cur.x;
		else if (cur.dir == 3) stop_d = 240 - cur.y;

		if (stop_d >= 0 && stop_d < cur.safety_distance) {		// 앞 차보다 정지선이 가까울때
			cur.safety_distance = stop_d;
		}
	}

	int step;
	if (cur.safety_distance <= 70) step = 0;
	else if (cur.safety_distance <= 100) step = 3;
	else step = 6;

	if (cur.dir == 0) cur.x = cur.x - step;
	else if (cur.dir == 1) cur.y = cur.y - step;
	else if (cur.dir == 2) cur.x = cur.x + step;
	else if (cur.dir == 3) cur.y = cur.y + step;

	if (cur.x > v_w) cur.x = cur.x - v_w;
	else if (cur.x < 0) cur.x = cur.x + v_w;
	if (cur.y > v_h) cur.y = cur.y - v_h;
	else if (cur.y < 0) cur.y = cur.y + v_h;
}

void light_timer(vector<LIGHT>& l, float& time, int light_mode) {
	if (light_mode != 0)
		return;
	for (int i = 0; i < 2; ++i) {
		if (l[i].remain_time > 0)
			l[i].remain_time -= 100;
		else {
			if (l[i].state == 0) {
				l[i].state = 2;
				l[i].remain_time = blue_time;
			}
			else if (l[i].state == 1) {
				l[i].state = 0;
				l[i].remain_time = red_time;
			}
			else if (l[i].state == 2) {
				l[i].state = 1;
				l[i].remain_time = yellow_time;
			}
		}
	}
	if (time > 0)
		time -= 0.1;
	else if (time <= 0)
		time = 11.0f;
}

void set_man_target(MAN& man, const vector<LIGHT>& l, int light_mode) {
	int nextX = man.x;
	int nextY = man.y;

	if (light_mode == 2) {	// 좌클릭 신호
		if (man.x == 310) nextX = 490;
		else nextX = 310;
		if (man.y == 390) nextY = 210;
		else nextY = 390;
	}
	else { // 일반 신호
		if (l[0].state == 2) { // 세로 신호 -> 상하 이동
			if (man.y == 390) nextY = 210;
			else nextY = 390;
		}
		else if (l[1].state == 2) { // 가로 신호 -> 좌우 이동
			if (man.x == 310) nextX = 490;
			else nextX = 310;
		}
	}

	if (nextX != man.x || nextY != man.y) {
		if (man.isMoving == false) {
			man.tx = nextX;
			man.ty = nextY;
			man.isMoving = true;
		}
		else {
			man.tmpX = nextX;
			man.tmpY = nextY;
			man.hasNext = true;
		}
	}
}

void man_move(MAN& man, const vector<LIGHT> l, int& crossing) {
	if (man.isMoving == false) {
		crossing = 0;
		return;
	}

	if (man.x < man.tx) man.x = man.x + 3;
	else if (man.x > man.tx) man.x = man.x - 3;

	if (man.y < man.ty) man.y = man.y + 3;
	else if (man.y > man.ty) man.y = man.y - 3;

	if (man.x == man.tx) {	// 도착 확인
		if (man.y == man.ty) {
			if (man.hasNext == true) {	// 예약된 목적지 확인
				man.tx = man.tmpX;
				man.ty = man.tmpY;
				man.hasNext = false;
				man.isMoving = true;
			}
			else {
				man.isMoving = false;
				crossing = false;
			}
		}
	}

	crossing = 0;

	if ((man.x > 310 && man.x < 490) && (man.y == 390 || man.y == 210))		// 세로 정지
		crossing = 2;
	if ((man.y > 210 && man.y < 390) && (man.x == 310 || man.x == 490))	// 가로 정지
		crossing = 1;
	if (man.y > 210 && man.y < 390 && man.x > 310 && man.x < 490)
		crossing = 3;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC;
	HPEN hPen, oldPen;
	HBRUSH hBrush, oldBrush;
	TCHAR str[50];
	static RECT rt;
	static int mx, my;

	static vector<CAR> cars(8);
	static vector<LIGHT> traffic_lights(2);
	static int selected = -1;
	static float ttime = light_time;
	static int light_mode = 0;
	static int crossing = 0;

	static MAN man = { 310, 390, 310, 390, 310, 390, false, false };


	switch (uMsg) {
	case WM_CREATE:
		init_setting(cars, traffic_lights);
		for (int i = 0; i < 8; ++i)
			SetTimer(hWnd, i, cars[i].speed, NULL);
		SetTimer(hWnd, 8, 100, NULL);
		SetTimer(hWnd, 9, 100, NULL);
		break;
	case WM_KEYDOWN:
		if (wParam == VK_OEM_PLUS || wParam == VK_ADD) {
			if (selected == -1) {
				for (int i = 0; i < 8; ++i) {
					if (cars[i].speed > 10) {
						cars[i].speed -= 5;
						KillTimer(hWnd, i);
						SetTimer(hWnd, i, cars[i].speed, NULL);
					}
				}
			}
			else {
				if (cars[selected - 1].speed > 10) {
					cars[selected - 1].speed -= 5;
					KillTimer(hWnd, selected - 1);
					SetTimer(hWnd, selected - 1, cars[selected - 1].speed, NULL);
				}
			}

			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == VK_OEM_MINUS || wParam == VK_SUBTRACT) {
			if (selected == -1) {
				for (int i = 0; i < 8; ++i) {
					if (cars[i].speed < 30) {
						cars[i].speed += 5;
						KillTimer(hWnd, i);
						SetTimer(hWnd, i, cars[i].speed, NULL);
					}
				}
			}
			else {
				if (cars[selected - 1].speed < 30) {
					cars[selected - 1].speed += 5;
					KillTimer(hWnd, selected - 1);
					SetTimer(hWnd, selected - 1, cars[selected - 1].speed, NULL);
				}
			}

			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam >= '1' && wParam <= '8') {
			switch (wParam - '0') {
			case 1:
				if (selected == 1) selected = -1;
				else selected = 1;
				break;
			case 2:
				if (selected == 2) selected = -1;
				else selected = 2;
				break;
			case 3:
				if (selected == 3) selected = -1;
				else selected = 3;
				break;
			case 4:
				if (selected == 4) selected = -1;
				else selected = 4;
				break;
			case 5:
				if (selected == 5) selected = -1;
				else selected = 5;
				break;
			case 6:
				if (selected == 6) selected = -1;
				else selected = 6;
				break;
			case 7:
				if (selected == 7) selected = -1;
				else selected = 7;
				break;
			case 8:
				if (selected == 8) selected = -1;
				else selected = 8;
				break;
			}
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'A') {
			if (light_mode == 0)
				light_mode = 1;
			else if (light_mode == 1)
				light_mode = 0;
		}
		else if (wParam == VK_ESCAPE || wParam == 'Q') {
			PostQuitMessage(0);
		}
		break;
	case WM_SIZE:
		InvalidateRect(hWnd, NULL, FALSE);
		return 0;
	case WM_GETMINMAXINFO:
	{
		LPMINMAXINFO lpmmi = (LPMINMAXINFO)lParam;

		lpmmi->ptMinTrackSize.x = 800;
		lpmmi->ptMinTrackSize.y = 600;
	}
	return 0;
	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);
		{
			GetClientRect(hWnd, &rt);
			int v_w = rt.right;
			if (v_w < 800) v_w = 800;

			int v_h = rt.bottom;
			if (v_h < 600) v_h = 600;
			// 백버퍼 생성
			HDC memDC = CreateCompatibleDC(hDC);
			HBITMAP hBit = CreateCompatibleBitmap(hDC, rt.right, rt.bottom);
			HBITMAP oldBit = (HBITMAP)SelectObject(memDC, hBit);

			// 배경 지우기 (더블 버퍼링의 핵심: 하얀색으로 백버퍼를 채움)
			FillRect(memDC, &ps.rcPaint, (HBRUSH)GetStockObject(WHITE_BRUSH));

			//-------------------------------------도로 그리기
			hPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
			oldPen = (HPEN)SelectObject(memDC, hPen);

			MoveToEx(memDC, 340, 0, NULL); LineTo(memDC, 340, rt.bottom);
			MoveToEx(memDC, 400, 0, NULL); LineTo(memDC, 400, rt.bottom);
			MoveToEx(memDC, 460, 0, NULL); LineTo(memDC, 460, rt.bottom);

			MoveToEx(memDC, 0, 240, NULL); LineTo(memDC, rt.right, 240);
			MoveToEx(memDC, 0, 300, NULL); LineTo(memDC, rt.right, 300);
			MoveToEx(memDC, 0, 360, NULL); LineTo(memDC, rt.right, 360);

			SelectObject(memDC, oldPen);
			DeleteObject(hPen);
			//-------------------------------------자동차 그리기
			for (int i = 0; i < 8; ++i) {
				CAR& c = cars[i];
				int w = 0, h = 0;

				if (c.dir == 0 || c.dir == 2) { w = 25; h = 15; }
				else { w = 15; h = 25; }

				hBrush = CreateSolidBrush(RGB(102, 255, 255));
				oldBrush = (HBRUSH)SelectObject(memDC, hBrush);

				if (selected == i + 1) {
					hPen = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
				}
				else {
					hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
				}
				oldPen = (HPEN)SelectObject(memDC, hPen);

				Rectangle(memDC, c.x - w, c.y - h, c.x + w, c.y + h);

				if (c.x - w < 0)
					Rectangle(memDC, v_w + (c.x - w), c.y - h, v_w + (c.x + w), c.y + h);
				if (c.x + w > v_w)
					Rectangle(memDC, (c.x - w) - v_w, c.y - h, (c.x + w) - v_w, c.y + h);
				if (c.y - h < 0)
					Rectangle(memDC, c.x - w, v_h + (c.y - h), c.x + w, v_h + (c.y + h));
				if (c.y + h > v_h)
					Rectangle(memDC, c.x - w, (c.y - h) - v_h, c.x + w, (c.y + h) - v_h);

				SelectObject(memDC, oldPen);
				DeleteObject(hPen);
				SelectObject(memDC, oldBrush);
				DeleteObject(hBrush);
			}
			//-------------------------------------신호등 그리기
			hBrush = CreateSolidBrush(RGB(0, 0, 0));
			oldBrush = (HBRUSH)SelectObject(memDC, hBrush);

			Rectangle(memDC, 200, 10, 330, 60);
			Rectangle(memDC, 470, 470, 600, 520);

			SelectObject(memDC, oldBrush);
			DeleteObject(hBrush);

			if (traffic_lights[0].state == 0) {
				hBrush = CreateSolidBrush(RGB(255, 0, 0));
				oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
				Ellipse(memDC, 210, 20, 240, 50);
				SelectObject(memDC, oldBrush);
				DeleteObject(hBrush);
			}
			else if (traffic_lights[0].state == 1) {
				hBrush = CreateSolidBrush(RGB(255, 255, 102));
				oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
				Ellipse(memDC, 250, 20, 280, 50);
				SelectObject(memDC, oldBrush);
				DeleteObject(hBrush);
			}
			else if (traffic_lights[0].state == 2) {
				hBrush = CreateSolidBrush(RGB(0, 102, 255));
				oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
				Ellipse(memDC, 290, 20, 320, 50);
				SelectObject(memDC, oldBrush);
				DeleteObject(hBrush);
			}

			if (traffic_lights[1].state == 0) {
				hBrush = CreateSolidBrush(RGB(255, 0, 0));
				oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
				Ellipse(memDC, 480, 480, 510, 510);
				SelectObject(memDC, oldBrush);
				DeleteObject(hBrush);
			}
			else if (traffic_lights[1].state == 1) {
				hBrush = CreateSolidBrush(RGB(255, 255, 102));
				oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
				Ellipse(memDC, 520, 480, 550, 510);
				SelectObject(memDC, oldBrush);
				DeleteObject(hBrush);
			}
			else if (traffic_lights[1].state == 2) {
				hBrush = CreateSolidBrush(RGB(0, 102, 255));
				oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
				Ellipse(memDC, 560, 480, 590, 510);
				SelectObject(memDC, oldBrush);
				DeleteObject(hBrush);
			}
			//-------------------------------------사람 그리기
			hBrush = CreateSolidBrush(RGB(255, 255, 102));
			oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
			Ellipse(memDC, man.x - 20, man.y - 20, man.x + 20, man.y + 20);
			SelectObject(memDC, oldBrush);
			DeleteObject(hBrush);

			SetBkMode(memDC, TRANSPARENT);
			SetTextColor(memDC, RGB(0, 0, 0));
			SetTextAlign(memDC, TA_TOP);
			_stprintf_s(str, L"신호 변경까지 %.1f초", ttime);
			TextOut(memDC, 30, 500, str, _tcslen(str));

			// 완성된 백버퍼를 실제 화면으로 한 번에 복사
			BitBlt(hDC, 0, 0, rt.right, rt.bottom, memDC, 0, 0, SRCCOPY);

			SelectObject(memDC, oldBit);
			DeleteObject(hBit);
			DeleteDC(memDC);
		}
		EndPaint(hWnd, &ps);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_TRAFFICLIGHTS_H40003:
			traffic_lights[1].remain_time = blue_time;
			traffic_lights[1].state = 2;
			traffic_lights[0].remain_time = red_time;
			traffic_lights[0].state = 0;
			ttime = light_time;

			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case ID_TRAFFICLIGHTS_H40004:
			traffic_lights[0].remain_time = blue_time;
			traffic_lights[0].state = 2;
			traffic_lights[1].remain_time = red_time;
			traffic_lights[1].state = 0;
			ttime = light_time;

			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case ID_MENU_AUTO:
			if (light_mode == 1)
				light_mode = 0;
			break;
		case ID_MENU_STOP:
			if (light_mode == 0)
				light_mode = 1;
			break;
		case ID_MENU_QUIT:
			PostQuitMessage(0);
			break;
		}
		break;
	case WM_ERASEBKGND:
		return 1;
	case WM_LBUTTONDOWN:
		if (light_mode != 2) {
			light_mode = 2;
			traffic_lights[0].state = 0;
			traffic_lights[1].state = 0;
		}
		set_man_target(man, traffic_lights, light_mode);

		InvalidateRect(hWnd, NULL, FALSE);
		break;
	case WM_RBUTTONDOWN:
		if (light_mode == 2) {
			light_mode = 0;
			traffic_lights[0].remain_time = blue_time;
			traffic_lights[0].state = 2;
			traffic_lights[1].remain_time = red_time;
			traffic_lights[1].state = 0;
			ttime = light_time;
		}

		InvalidateRect(hWnd, NULL, FALSE);
		break;
	case WM_TIMER:
		GetClientRect(hWnd, &rt);

		if (wParam >= 0 && wParam <= 7) {
			car_move((int)wParam, cars, traffic_lights, rt, crossing);
		}
		else if (wParam == 8) {
			light_timer(traffic_lights, ttime, light_mode);
			if (traffic_lights[0].remain_time == blue_time - yellow_time || traffic_lights[1].remain_time == blue_time - yellow_time) {
				if (man.isMoving == false)
					set_man_target(man, traffic_lights, light_mode);
			}
		}
		else if (wParam == 9) {
			man_move(man, traffic_lights, crossing);
		}

		InvalidateRect(hWnd, NULL, FALSE);
		UpdateWindow(hWnd);
		break;
	case WM_DESTROY:
		KillTimer(hWnd, 1);
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}