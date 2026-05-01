#include <windows.h>
#include <tchar.h>
#include <random>
#include <vector>
#include <math.h>

#define LEN 800
#define HEI 600
#define BRICK_LEN 60
#define BRICK_HEI 20
#define BRICK_LINE 3
#define BRICK_NUM 10
#define START_X 100
#define START_Y 20
#define PADDLE_LEN 150
#define PADDLE_HEI 15
#define BALL_RADIUS 10
#define MAX_SPEED 10.0f
#define MIN_SPEED 3.0f

using namespace std;
random_device rd;
mt19937 g(rd());
uniform_int_distribution<> uid_rgb{ 0, 255 };
uniform_int_distribution<> uid_speed{ 10, 40 };
uniform_int_distribution<> uid_dir{ 0, 1 };

struct Point2D { float x, y; };
struct PADDLE { Point2D pos; int velocity; int dir; };
struct BRICK { Point2D pos; int line; COLORREF color; bool IsFalling; int dir; };
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

void init_setting()
{
	// 벽돌 세팅
	for (int i = 0; i < BRICK_LINE; ++i) {
		int dir = uid_dir(g);
		for (int j = 0; j < BRICK_NUM; ++j) {
			bricks[i][j].color = RGB(255, 255, 0);
			bricks[i][j].IsFalling = false;
			bricks[i][j].line = i;
			bricks[i][j].dir = dir;
			bricks[i][j].pos.x = START_X + BRICK_LEN * j;
			bricks[i][j].pos.y = START_Y + BRICK_HEI * i;
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
}

void CollisionToPaddle()
{
	int r = BALL_RADIUS;
	RECT bRect = { ball.pos.x - r, ball.pos.y - r , ball.pos.x + r , ball.pos.y + r };
	RECT pRect = { paddle.pos.x - PADDLE_LEN / 2, paddle.pos.y, paddle.pos.x + PADDLE_LEN / 2, paddle.pos.y + PADDLE_HEI };
	RECT temp;
	if (IntersectRect(&temp, &bRect, &pRect)) {
		if (ball.moving_pos.y > 0) {
			// --- [1] 반사각 결정 로직 ---
			// 패들의 중심에서 공이 얼마나 떨어져 있는지 비율 계산 (-1.0 ~ 1.0)
			float hitPoint = (ball.pos.x - paddle.pos.x) / (PADDLE_LEN / 2.0f);

			// hitPoint에 따라 X축 속도를 결정 (가장자리일수록 X값이 커져서 옆으로 누움)
			// 최대 X속도를 10.0f 정도로 잡았을 때:
			ball.moving_pos.x = hitPoint * 10.0f;

			// Y축은 항상 위로 튕겨나가되, X가 커질수록(옆으로 누울수록) 작아져야 자연스러움
			// 피타고라스 정리에 의해 일정한 전체 속력을 유지하도록 설정하거나, 
			// 간단하게 고정값(단, 위쪽 방향이므로 음수)을 줍니다.
			ball.moving_pos.y = -sqrt(max(0.0f, 100.0f - pow(ball.moving_pos.x, 2))); // 전체 속력 10 유지

			// --- [2] 패들 속도에 따른 가속/감속 로직 ---
			// 패들이 느리면(예: 5 미만) 감속, 빠르면(예: 15 이상) 가속
			float speedFactor = 1.0f;
			if (paddle.velocity < 10) {
				speedFactor = 0.8f;  // 살짝 갖다 대면 속도 감소
			}
			else if (paddle.velocity > 25) {
				speedFactor = 1.3f;  // 강하게 휘두르면 속도 증가
			}

			ball.moving_pos.x *= speedFactor;
			ball.moving_pos.y *= speedFactor;

			// 위치 보정 (패들에 공이 박히는 현상 방지)
			ball.pos.y = paddle.pos.y - r;
		}
	}
	// 현재 전체 속력(크기) 계산
	float currentSpeed = sqrt(pow(ball.moving_pos.x, 2) + pow(ball.moving_pos.y, 2));

	// 최대/최소 속도 제한 적용
	if (currentSpeed > MAX_SPEED) {
		ball.moving_pos.x = (ball.moving_pos.x / currentSpeed) * MAX_SPEED;
		ball.moving_pos.y = (ball.moving_pos.y / currentSpeed) * MAX_SPEED;
	}
	else if (currentSpeed < MIN_SPEED) {
		ball.moving_pos.x = (ball.moving_pos.x / currentSpeed) * MIN_SPEED;
		ball.moving_pos.y = (ball.moving_pos.y / currentSpeed) * MIN_SPEED;
	}
}

void CollisionToBrick()
{

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

	switch (uMsg) {
	case WM_CREATE:
		init_setting();
		SetTimer(hWnd, 0, 10, NULL);
		break;
	case WM_KEYDOWN:
		if (wParam == 'S') {
			if (!isS) {
				isS = true;
				ball.moving_pos.x = 0;
				ball.moving_pos.y = -7.0f;
			}
		}
		else if (wParam == 'P') {
			if (isP && isS)
				SetTimer(hWnd, 0, 10, NULL);
			else if (!isP && isS)
				KillTimer(hWnd, 0);
			isP = !isP;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'T') {

			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == VK_OEM_PLUS || wParam == VK_ADD) {

			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == VK_OEM_MINUS || wParam == VK_SUBTRACT) {

			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'R') {
			KillTimer(hWnd, 0);

			isS = false;
			isP = false;
			init_setting();

			SetTimer(hWnd, 0, 10, NULL);
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'Q' || wParam == VK_ESCAPE) {
			KillTimer(hWnd, 0);
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

			// --- 1. 벽돌 그리기 ---
			for (int i = 0; i < BRICK_LINE; ++i) {
				for (int j = 0; j < BRICK_NUM; ++j) {
					// 파괴되지 않은 벽돌만 그림 (추후 CollisionToBrick에서 IsFalling 등을 활용할 수 있음)
					if (!bricks[i][j].IsFalling) {
						hBrush = CreateSolidBrush(bricks[i][j].color);
						oldBrush = (HBRUSH)SelectObject(memDC, hBrush);

						Rectangle(memDC, (int)bricks[i][j].pos.x, (int)bricks[i][j].pos.y, (int)bricks[i][j].pos.x + BRICK_LEN, (int)bricks[i][j].pos.y + BRICK_HEI);

						SelectObject(memDC, oldBrush);
						DeleteObject(hBrush);
					}
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
					isS = false;
					ball.moving_pos = { 0, 0 };
					ball.pos.x = paddle.pos.x;
					ball.pos.y = paddle.pos.y - BALL_RADIUS;
				}

				CollisionToPaddle();
			}
			else {
				ball.pos.x = paddle.pos.x;
				ball.pos.y = paddle.pos.y - BALL_RADIUS;
			}
			InvalidateRect(hWnd, NULL, FALSE);
		}

		InvalidateRect(hWnd, NULL, FALSE);
		UpdateWindow(hWnd);
		break;
	case WM_DESTROY:
		KillTimer(hWnd, 0);
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}