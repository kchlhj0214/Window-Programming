#include <windows.h>
#include <tchar.h>
#include <random>
#include <vector>
#include <math.h>
#include "resource.h"

#define LEN 800
#define HEI 600
#define BRICK_LEN 60
#define BRICK_HEI 20
#define BRICK_LINE 3
#define BRICK_NUM 10
#define START_X 100
#define START_Y 30
#define PADDLE_LEN 150
#define PADDLE_HEI 15
#define BALL_RADIUS 10
#define MAX_SPEED 10.0f
#define MIN_SPEED 3.0f

using namespace std;
random_device rd;
mt19937 g(rd());
uniform_int_distribution<> uid_rgb{ 0, 255 };
uniform_int_distribution<> uid_speed{ 1, 5 };
uniform_int_distribution<> uid_dir{ 0, 1 };

struct Point2D { float x, y; };
struct PADDLE { Point2D pos; int velocity; int dir; };
struct BRICK { Point2D pos; int line; COLORREF color; bool IsFalling; int dir; int color_change; };
struct BALL { Point2D pos, moving_pos; };	// 현재 좌표(pos)와 x, y축으로 프레임당 얼마만큼 움직이는지(moving_pos) 저장

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"My Window Class";
LPCTSTR lpszWindowName = L"windows program 2";

PADDLE paddle;
BRICK bricks[BRICK_LINE][BRICK_NUM];
BALL ball;
int colorChangedBricks = 0;
int brokenBricks = 0;
float lastPaddleX = 0.0f;
bool isPaddleDragging = false;
bool isP = false;
bool isS = false;
bool isT = false;
int pH = 0, pM = 0, pS = 0;
int bricks_speed[BRICK_LINE];

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
	WndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU2);
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

void init_setting()
{
	// 벽돌 세팅
	for (int i = 0; i < BRICK_LINE; ++i) {
		bricks_speed[i] = uid_speed(g);
		int dir = uid_dir(g);
		for (int j = 0; j < BRICK_NUM; ++j) {
			bricks[i][j].color = RGB(255, 255, 0);
			bricks[i][j].IsFalling = false;
			bricks[i][j].line = i;
			bricks[i][j].dir = dir;
			bricks[i][j].pos.x = START_X + BRICK_LEN * j;
			bricks[i][j].pos.y = START_Y + BRICK_HEI * i;
			bricks[i][j].color_change = 0;
		}
	}

	// 패들 세팅
	paddle.pos.x = LEN / 2;		// 중심의 x좌표
	paddle.pos.y = HEI - 100;	// 윗부분 y좌표
	paddle.velocity = 0;

	// 공 세팅
	ball.pos.x = paddle.pos.x;
	ball.pos.y = paddle.pos.y - BALL_RADIUS;
	ball.moving_pos.x = 0;
	ball.moving_pos.y = 0;

	colorChangedBricks = 0;
	brokenBricks = 0;

	colorChangedBricks = 0;
	brokenBricks = 0;
	lastPaddleX = 0.0f;
	isPaddleDragging = false;
	isP = false;
	isS = false;
	isT = false;
	pH = 0; pM = 0; pS = 0;
}

void CollisionToPaddle()
{
	int r = BALL_RADIUS;
	// RECT 좌표 형변환 경고 해결
	RECT bRect = { (int)(ball.pos.x - r), (int)(ball.pos.y - r) , (int)(ball.pos.x + r) , (int)(ball.pos.y + r) };
	RECT pRect = { (int)(paddle.pos.x - PADDLE_LEN / 2), (int)paddle.pos.y, (int)(paddle.pos.x + PADDLE_LEN / 2), (int)(paddle.pos.y + PADDLE_HEI) };
	RECT temp;

	if (IntersectRect(&temp, &bRect, &pRect)) {
		if (ball.moving_pos.y > 0) {
			// 충돌 직전의 전체 속력(크기) 계산
			float currentSpeed = sqrt(pow(ball.moving_pos.x, 2) + pow(ball.moving_pos.y, 2));

			// 패들 중심에서의 거리 비율 (-1.0 ~ 1.0) 계산
			float hitPoint = (ball.pos.x - paddle.pos.x) / (PADDLE_LEN / 2.0f);

			// 새로운 방향 계산 (일단 X축 방향을 비율대로 설정)
			ball.moving_pos.x = hitPoint * 10.0f;

			// 피타고라스 정리에 의해 Y축 크기 결정 (위쪽이므로 음수)
			float tempY = -sqrt(max(0.0f, 100.0f - pow(ball.moving_pos.x, 2)));

			float minAbsY = MIN_SPEED;
			if (abs(tempY) < minAbsY) {
				tempY = -minAbsY;
				// Y를 키웠으므로, 전체 비율을 맞추기 위해 X를 줄여야 함
				float signX = (ball.moving_pos.x >= 0) ? 1.0f : -1.0f;
				ball.moving_pos.x = signX * sqrt(max(0.0f, 100.0f - pow(tempY, 2)));
			}
			ball.moving_pos.y = tempY;

			// 방향 벡터 정규화 (크기를 1로 만듦)
			float dirLen = sqrt(pow(ball.moving_pos.x, 2) + pow(ball.moving_pos.y, 2));
			ball.moving_pos.x /= dirLen;
			ball.moving_pos.y /= dirLen;

			// 패들 속도에 따른 가속/감속
			float speedFactor = 1.0f;
			if (paddle.velocity < 10) {
				speedFactor = 0.9f;  // 감속 비율
			}
			else if (paddle.velocity > 25) {
				speedFactor = 1.1f;  // 가속 비율
			}

			// 최종 속도 적용 (원래 속력 * 방향 벡터 * 가감속 비율)
			float finalSpeedVal = currentSpeed * speedFactor;

			ball.moving_pos.x *= finalSpeedVal;
			ball.moving_pos.y *= finalSpeedVal;

			// 위치 보정
			ball.pos.y = paddle.pos.y - r;
		}
	}

	// 최종 절대 속도 제한 (MAX_SPEED, MIN_SPEED 준수)
	float speed = sqrt(pow(ball.moving_pos.x, 2) + pow(ball.moving_pos.y, 2));
	if (speed > MAX_SPEED) {
		ball.moving_pos.x = (ball.moving_pos.x / speed) * MAX_SPEED;
		ball.moving_pos.y = (ball.moving_pos.y / speed) * MAX_SPEED;
	}
	else if (speed < MIN_SPEED) {
		// 속도가 너무 느려지는 것 방지
		ball.moving_pos.x = (ball.moving_pos.x / speed) * MIN_SPEED;
		ball.moving_pos.y = (ball.moving_pos.y / speed) * MIN_SPEED;
	}
}

void CollisionToBrick()
{
	int r = BALL_RADIUS;
	RECT bRect = { (int)(ball.pos.x - r), (int)(ball.pos.y - r), (int)(ball.pos.x + r), (int)(ball.pos.y + r) };

	for (int i = 0; i < BRICK_LINE; ++i) {
		for (int j = 0; j < BRICK_NUM; ++j) {
			if (bricks[i][j].IsFalling) continue;

			RECT brRect = { (int)bricks[i][j].pos.x, (int)bricks[i][j].pos.y, (int)(bricks[i][j].pos.x + BRICK_LEN), (int)(bricks[i][j].pos.y + BRICK_HEI) };

			RECT temp;
			if (IntersectRect(&temp, &bRect, &brRect)) {
				COLORREF c = RGB(uid_rgb(g), uid_rgb(g), uid_rgb(g));
				for (int k = 0; k < BRICK_NUM; ++k) {
					if (!bricks[i][k].IsFalling) {
						bricks[i][k].color = c;
					}
				}

				bricks[i][j].IsFalling = true;
				brokenBricks++;

				// 겹친 영역의 가로가 세로보다 길면 위/아래 충돌, 짧으면 좌/우 충돌
				int overlapWidth = temp.right - temp.left;
				int overlapHeight = temp.bottom - temp.top;

				if (overlapWidth > overlapHeight) {
					ball.moving_pos.y *= -1;
				}
				else {
					ball.moving_pos.x *= -1;
				}

				return;
			}
		}
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC;
	HPEN hPen, oldPen;
	HBRUSH hBrush, oldBrush;
	static TCHAR str[50];
	static TCHAR str1[50];
	static TCHAR str2[50];
	static TCHAR str3[50];
	static TCHAR str4[50];
	static RECT rt;
	static int mx, my;
	SYSTEMTIME curTime;

	switch (uMsg) {
	case WM_CREATE:
		init_setting();
		SetTimer(hWnd, 0, 10, NULL);	// 패들 & 공
		SetTimer(hWnd, 1, 1000, NULL);	// 시계	
		break;
	case WM_KEYDOWN:
		if (wParam == 'S') {
			if (!isS) {
				isS = true;
				ball.moving_pos.x = 0;
				ball.moving_pos.y = -5.0f;
				GetLocalTime(&curTime);
				wsprintf(str1, TEXT("Start time : %02d:%02d:%02d"), curTime.wHour, curTime.wMinute, curTime.wSecond);
				SetTimer(hWnd, 2, 1000, NULL);
				SetTimer(hWnd, 3, 10, NULL);	// 떨어지는 벽돌	
				SetTimer(hWnd, 4, 10, NULL);	// 벽돌 좌우 움직임
			}
		}
		else if (wParam == 'P') {
			if (isP) {
				SetTimer(hWnd, 0, 10, NULL);
				SetTimer(hWnd, 3, 10, NULL); 
				SetTimer(hWnd, 4, 10, NULL);
			}
			else if (!isP) {
				KillTimer(hWnd, 0);
				KillTimer(hWnd, 3);
				KillTimer(hWnd, 4);
			}
			isP = !isP;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'T') {
			isT = !isT;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == VK_OEM_PLUS || wParam == VK_ADD) {
			float currentSpeed = sqrt(pow(ball.moving_pos.x, 2) + pow(ball.moving_pos.y, 2));
			if (currentSpeed < MAX_SPEED) {
				if (ball.moving_pos.x > 0)
					ball.moving_pos.x *= 1.1f;
				else
					ball.moving_pos.x *= 1.1f;

				if (ball.moving_pos.y > 0)
					ball.moving_pos.y *= 1.1f;
				else
					ball.moving_pos.y *= 1.1f;
			}
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == VK_OEM_MINUS || wParam == VK_SUBTRACT) {
			float currentSpeed = sqrt(pow(ball.moving_pos.x, 2) + pow(ball.moving_pos.y, 2));
			if (currentSpeed > MIN_SPEED) {
				if (ball.moving_pos.x > 0)
					ball.moving_pos.x *= 0.9f;
				else
					ball.moving_pos.x *= 0.9f;

				if (ball.moving_pos.y > 0)
					ball.moving_pos.y *= 0.9f;
				else
					ball.moving_pos.y *= 0.9f;
			}
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'R') {
			KillTimer(hWnd, 0);
			KillTimer(hWnd, 2);
			KillTimer(hWnd, 3);
			KillTimer(hWnd, 4);

			isS = false;
			isP = false;
			init_setting();

			SetTimer(hWnd, 0, 10, NULL);;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'Q' || wParam == VK_ESCAPE) {
			KillTimer(hWnd, 0);
			KillTimer(hWnd, 1);
			KillTimer(hWnd, 2);
			KillTimer(hWnd, 3);
			KillTimer(hWnd, 4);
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
		GetClientRect(hWnd, &rt);
		{
			// 백버퍼 생성
			HDC memDC = CreateCompatibleDC(hDC);
			HBITMAP hBit = CreateCompatibleBitmap(hDC, rt.right, rt.bottom);
			HBITMAP oldBit = (HBITMAP)SelectObject(memDC, hBit);

			// 배경 지우기 (더블 버퍼링의 핵심: 하얀색으로 백버퍼를 채움)
			FillRect(memDC, &ps.rcPaint, (HBRUSH)GetStockObject(WHITE_BRUSH));

			if (isT) {
				SetBkMode(memDC, TRANSPARENT);
				TextOut(memDC, 50, HEI - 120, str, lstrlen(str));
				TextOut(memDC, 50, HEI - 100, str2, lstrlen(str2));
				TextOut(memDC, 50, HEI - 80, str1, lstrlen(str1));
			}

			// --- 1. 벽돌 그리기 ---
			for (int i = 0; i < BRICK_LINE; ++i) {
				for (int j = 0; j < BRICK_NUM; ++j) {
					// 파괴되지 않은 벽돌만 그림 (추후 CollisionToBrick에서 IsFalling 등을 활용할 수 있음)
					hBrush = CreateSolidBrush(bricks[i][j].color);
					oldBrush = (HBRUSH)SelectObject(memDC, hBrush);

					Rectangle(memDC, (int)bricks[i][j].pos.x, (int)bricks[i][j].pos.y, (int)bricks[i][j].pos.x + BRICK_LEN, (int)bricks[i][j].pos.y + BRICK_HEI);

					SelectObject(memDC, oldBrush);
					DeleteObject(hBrush);
				}
			}

			// --- 2. 패들 그리기 ---
			hBrush = CreateSolidBrush(RGB(100, 100, 255)); // 패들 색상 (파란색 계열)
			oldBrush = (HBRUSH)SelectObject(memDC, hBrush);

			Rectangle(memDC, (int)(paddle.pos.x - PADDLE_LEN / 2), (int)paddle.pos.y, (int)(paddle.pos.x + PADDLE_LEN / 2), (int)(paddle.pos.y + PADDLE_HEI));

			SelectObject(memDC, oldBrush);
			DeleteObject(hBrush);

			// --- 3. 공 그리기 ---
			hBrush = CreateSolidBrush(RGB(255, 100, 100)); // 공 색상 (빨간색 계열)
			oldBrush = (HBRUSH)SelectObject(memDC, hBrush);

			if (isS)
				Ellipse(memDC, (int)(ball.pos.x - BALL_RADIUS), (int)(ball.pos.y - BALL_RADIUS), (int)(ball.pos.x + BALL_RADIUS), (int)(ball.pos.y + BALL_RADIUS));
			else
				Ellipse(memDC, (int)(paddle.pos.x - BALL_RADIUS), (int)(paddle.pos.y - BALL_RADIUS * 2), (int)(paddle.pos.x + BALL_RADIUS), (int)paddle.pos.y);
			SelectObject(memDC, oldBrush);
			DeleteObject(hBrush);

			colorChangedBricks = 0;
			for (int i = 0; i < BRICK_LINE; ++i) {
				int n = 0;
				bool changed = false;
				for (int j = 0; j < BRICK_NUM; ++j) {
					if (!bricks[i][j].IsFalling)
						n++;
					else
						changed = true;
				}
				if (changed)
					colorChangedBricks += n;
			}
			wsprintf(str3, TEXT("Broken Bricks : %d"), brokenBricks);
			wsprintf(str4, TEXT("Color Changed Bricks : %d"), colorChangedBricks);
			if (isP) {
				TextOut(memDC, (LEN / 2) - 100, (HEI / 2), str3, lstrlen(str3));
				TextOut(memDC, (LEN / 2) - 100, (HEI / 2) + 20, str4, lstrlen(str4));
			}

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
		case ID_START:
			if (!isS) {
				isS = true;
				ball.moving_pos.x = 0;
				ball.moving_pos.y = -5.0f;
				GetLocalTime(&curTime);
				wsprintf(str1, TEXT("Start time : %02d:%02d:%02d"), curTime.wHour, curTime.wMinute, curTime.wSecond);
				SetTimer(hWnd, 2, 1000, NULL);
				SetTimer(hWnd, 3, 10, NULL);	// 떨어지는 벽돌	
				SetTimer(hWnd, 4, 10, NULL);	// 벽돌 좌우 움직임
			}
			break;
		case ID_PAUSE:
			if (isP) {
				SetTimer(hWnd, 0, 10, NULL);
				SetTimer(hWnd, 3, 10, NULL);
				SetTimer(hWnd, 4, 10, NULL);
			}
			else if (!isP) {
				KillTimer(hWnd, 0);
				KillTimer(hWnd, 3);
				KillTimer(hWnd, 4);
			}
			isP = !isP;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case ID_TIME:
			isT = !isT;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case ID_PLUS:
		{
			float currentSpeed = sqrt(pow(ball.moving_pos.x, 2) + pow(ball.moving_pos.y, 2));
			if (currentSpeed < MAX_SPEED) {
				if (ball.moving_pos.x > 0)
					ball.moving_pos.x *= 1.1f;
				else
					ball.moving_pos.x *= 1.1f;

				if (ball.moving_pos.y > 0)
					ball.moving_pos.y *= 1.1f;
				else
					ball.moving_pos.y *= 1.1f;
			}
			InvalidateRect(hWnd, NULL, FALSE);
		}
			break;
		case ID_MINUS:
		{
			float currentSpeed = sqrt(pow(ball.moving_pos.x, 2) + pow(ball.moving_pos.y, 2));
			if (currentSpeed > MIN_SPEED) {
				if (ball.moving_pos.x > 0)
					ball.moving_pos.x *= 0.9f;
				else
					ball.moving_pos.x *= 0.9f;

				if (ball.moving_pos.y > 0)
					ball.moving_pos.y *= 0.9f;
				else
					ball.moving_pos.y *= 0.9f;
			}
			InvalidateRect(hWnd, NULL, FALSE);
		}
			break;
		case ID_RESET:
			KillTimer(hWnd, 0);
			KillTimer(hWnd, 2);
			KillTimer(hWnd, 3);
			KillTimer(hWnd, 4);

			isS = false;
			isP = false;
			init_setting();

			SetTimer(hWnd, 0, 10, NULL);;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case ID_QUIT:
			KillTimer(hWnd, 0);
			KillTimer(hWnd, 1);
			KillTimer(hWnd, 2);
			KillTimer(hWnd, 3);
			KillTimer(hWnd, 4);
			PostQuitMessage(0);
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		}
		break;
	case WM_ERASEBKGND:
		return 1;

	case WM_LBUTTONDOWN:
	{
		int mx = LOWORD(lParam);
		int my = HIWORD(lParam);

		if (mx >= paddle.pos.x - PADDLE_LEN / 2 && mx <= paddle.pos.x + PADDLE_LEN / 2 && my >= paddle.pos.y && my <= paddle.pos.y + PADDLE_HEI)
			isPaddleDragging = true;
	}
	break;

	case WM_MOUSEMOVE:
		if (isPaddleDragging) {
			paddle.pos.x = (float)LOWORD(lParam);

			// 화면 경계 제한 (패들이 창 밖으로 나가지 않게)
			if (paddle.pos.x < PADDLE_LEN / 2) paddle.pos.x = PADDLE_LEN / 2;
			if (paddle.pos.x > rt.right - PADDLE_LEN / 2) paddle.pos.x = rt.right - PADDLE_LEN / 2;
		}
		break;

	case WM_LBUTTONUP:
		if (isPaddleDragging) {
			isPaddleDragging = false;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;

	case WM_TIMER:
		GetClientRect(hWnd, &rt);

		if (wParam == 0) {
			float displacement = paddle.pos.x - lastPaddleX;
			paddle.velocity = abs(displacement);
			lastPaddleX = paddle.pos.x;

			// 패들 화면 경계 제한
			if (paddle.pos.x - PADDLE_LEN / 2.0f < rt.left)
				paddle.pos.x = rt.left + PADDLE_LEN / 2.0f;
			if (paddle.pos.x + PADDLE_LEN / 2.0f > rt.right)
				paddle.pos.x = rt.right - PADDLE_LEN / 2.0f;

			if (isS) {
				ball.pos.x += ball.moving_pos.x;
				ball.pos.y += ball.moving_pos.y;

				// 벽 충돌 (좌/우)
				if (ball.pos.x - BALL_RADIUS < rt.left) {
					ball.pos.x = rt.left + BALL_RADIUS;
					ball.moving_pos.x *= -1;
				}
				else if (ball.pos.x + BALL_RADIUS > rt.right) {
					ball.pos.x = rt.right - BALL_RADIUS;
					ball.moving_pos.x *= -1;
				}

				// 천장 충돌
				if (ball.pos.y - BALL_RADIUS < rt.top) {
					ball.pos.y = (float)rt.top + BALL_RADIUS;
					ball.moving_pos.y *= -1;
				}

				// 바닥 충돌
				if (ball.pos.y - BALL_RADIUS > rt.bottom) {
					ball.moving_pos = { 0, -0.5f };
					ball.pos.x = paddle.pos.x;
					ball.pos.y = paddle.pos.y - BALL_RADIUS;
				}

				CollisionToPaddle();
				CollisionToBrick();
			}
			else {
				ball.pos.x = paddle.pos.x;
				ball.pos.y = paddle.pos.y - BALL_RADIUS;
			}
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 1) {
			GetLocalTime(&curTime);
			wsprintf(str, TEXT("Current time : %02d:%02d:%02d"), curTime.wHour, curTime.wMinute, curTime.wSecond);
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 2) {
			pS++;
			if (pS == 60) {
				pS = 0;
				pM++;
				if (pM == 60) {
					pM = 0;
					pH++;
				}
			}
			wsprintf(str2, TEXT("Playing time : %02d:%02d:%02d"), pH, pM, pS);
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 3) {
			for (int i = 0; i < BRICK_LINE; ++i) {
				for (int j = 0; j < BRICK_NUM; ++j) {
					if (bricks[i][j].IsFalling) {
						bricks[i][j].pos.y++;
						bricks[i][j].color_change++;
					}
					if (bricks[i][j].color_change == 100) {
						bricks[i][j].color = RGB(uid_rgb(g), uid_rgb(g), uid_rgb(g));
						bricks[i][j].color_change = 0;
					}
				}
			}
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 4) {
			for (int i = 0; i < BRICK_LINE; ++i) {
				int first = 0, last = BRICK_NUM - 1;
				for (int j = 0; j < BRICK_NUM; ++j) {		// 맨 앞과 맨 뒤 찾기
					if (!bricks[i][j].IsFalling) {
						first = j;
						break;
					}
				}
				for (int j = BRICK_NUM - 1; j >= 0; --j) {		// 맨 앞과 맨 뒤 찾기
					if (!bricks[i][j].IsFalling) {
						last = j;
						break;
					}
				}

				if (bricks[i][first].pos.x <= rt.left) {
					for (int j = 0; j < BRICK_NUM; ++j) {
						if (!bricks[i][j].IsFalling) {
							bricks[i][j].dir = 1;
							bricks[i][j].pos.x += bricks_speed[i];
						}
					}
				}
				else if(bricks[i][last].pos.x + BRICK_LEN >= rt.right) {
					for (int j = 0; j < BRICK_NUM; ++j) {
						if (!bricks[i][j].IsFalling) {
							bricks[i][j].dir = 0;
							bricks[i][j].pos.x -= bricks_speed[i];
						}
					}
				}
				else {
					for (int j = 0; j < BRICK_NUM; ++j) {
						if (!bricks[i][j].IsFalling) {
							if (bricks[i][first].dir == 0)
								bricks[i][j].pos.x -= bricks_speed[i];
							else
								bricks[i][j].pos.x += bricks_speed[i];
						}
					}
				}
			}
		}

		InvalidateRect(hWnd, NULL, FALSE);
		UpdateWindow(hWnd);
		break;
	case WM_DESTROY:
		KillTimer(hWnd, 0);
		KillTimer(hWnd, 1);
		KillTimer(hWnd, 2);
		KillTimer(hWnd, 3);
		KillTimer(hWnd, 4);
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}