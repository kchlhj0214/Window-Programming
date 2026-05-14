#include <windows.h>
#include <tchar.h>
#include <random>
#include <vector>
#include <math.h>
#include <algorithm>
#include <atlimage.h>


#define LEN 1000
#define HEI 1000
#define CAT_SIZE 128
#define MOUSE_SIZE 48

using namespace std;
random_device rd;
mt19937 g(rd());
uniform_int_distribution<> uid_rgb{ 0, 255 };
uniform_int_distribution<> disX{ 50, LEN - 50 };
uniform_int_distribution<> disY{ 50, HEI - 50 };
uniform_int_distribution<> disTime{ 1000, 3000 };


HINSTANCE g_hInst;
LPCTSTR lpszClass = L"My Window Class";
LPCTSTR lpszWindowName = L"windows program 2";

enum class CatState { IDLE, CHASE, ATTACK, SPECIAL };

struct GameObject {
	float x, y;
	float targetX, targetY;
	int frame;
	int motionRow; // 스프라이트 시트의 줄 번호
	CatState state;
	DWORD lastActionTime;
	bool isLeft;
	bool isLocked;
};
struct Food {
	float x, y;
	bool exists = false;
};

Food g_food;
GameObject g_cat;
GameObject g_mouse;
bool g_isMouseDown = false;
CImage img_map, img_cat, img_mouse;
float g_catSpeedMult = 1.0f;

void cat_reset(HWND hWnd);

// 고양이 방향 및 애니메이션 설정
void UpdateCatMotion(HWND hWnd)
{
	// 애니메이션 속도 조절
	static int speedDivider = 0;
	if (++speedDivider < 4) return;
	speedDivider = 0;

	if (g_food.exists) {
		float dx = g_food.x - g_cat.x;
		float dy = g_food.y - g_cat.y;
		float dist = sqrt(dx * dx + dy * dy);

		if (abs(dx) > 1.0f) g_cat.isLeft = (dx < 0);

		if (dist < 10.0f) { // 먹이에 도착했을 때
			g_food.exists = false;
			g_cat.isLocked = false;

			// 먹이를 먹으면 속도 증가
			g_catSpeedMult += 0.2f;
			if (g_catSpeedMult > 5.0f) g_catSpeedMult = 5.0f;

			g_cat.state = CatState::IDLE;
			g_cat.targetX = (float)disX(g);
			g_cat.targetY = (float)disY(g);
		}
		else {
			float chaseSpeed = 8.0f * g_catSpeedMult;
			g_cat.x += (dx / dist) * chaseSpeed;
			g_cat.y += (dy / dist) * chaseSpeed;
			g_cat.frame = (g_cat.frame + 1) % 4;
		}
		return; // 먹이 먹는 중에는 다른 로직(SPECIAL 등) 실행 방지
	}

	// 특수 애니메이션(SPECIAL) 상태 처리
	if (g_cat.state == CatState::SPECIAL) {
		g_cat.frame++;

		// 66번 줄 8프레임 (0~7)
		if (g_cat.motionRow == 66) {
			if (g_cat.frame >= 8) {
				g_cat.motionRow = 68;
				g_cat.frame = 0;
			}
		}
		// 68번 줄 4프레임 (0~3)
		else if (g_cat.motionRow == 68) {
			if (g_cat.frame >= 4) {
				g_cat.motionRow = 70;
				g_cat.frame = 0;
			}
		}
		// 70번 줄 10프레임 (0~9)
		else if (g_cat.motionRow == 70) {
			if (g_cat.frame >= 10) {
				g_cat.state = CatState::IDLE;
				g_cat.isLocked = false;
				g_cat.frame = 0;
				g_cat.motionRow = 5;
				g_cat.targetX = (float)disX(g);
				g_cat.targetY = (float)disY(g);
				cat_reset(hWnd);
			}
		}
		return; // ★ SPECIAL 중에는 아래의 일반 이동 로직을 절대 실행하지 않음
	}

	// 3. 일반 상태 로직 (ATTACK, CHASE, IDLE)
	int maxFrame = 8; // 기본값

	if (g_cat.state == CatState::ATTACK) {
		g_cat.motionRow = 74;
		maxFrame = 4;
		if (GetTickCount() - g_cat.lastActionTime > 1500) {
			g_cat.state = CatState::IDLE;
			g_cat.targetX = (float)disX(g);
			g_cat.targetY = (float)disY(g);
		}
	}
	else if (g_isMouseDown) {
		g_cat.state = CatState::CHASE;
		g_cat.targetX = g_mouse.x;
		g_cat.targetY = g_mouse.y;
		g_cat.motionRow = 39;
		maxFrame = 4;
	}
	else {
		g_cat.state = CatState::IDLE;
		g_cat.motionRow = 5;
		maxFrame = 8;
	}

	// 이동 처리
	float dx = g_cat.targetX - g_cat.x;
	float dy = g_cat.targetY - g_cat.y;
	float dist = sqrt(dx * dx + dy * dy);

	if (abs(dx) > 1.0f) g_cat.isLeft = (dx < 0);

	if (dist < 40.0f) {
		if (g_cat.state == CatState::CHASE) {
			g_cat.state = CatState::ATTACK;
			g_cat.lastActionTime = GetTickCount();
			g_cat.frame = 0;
			g_isMouseDown = false;
		}
		else {
			g_cat.targetX = (float)disX(g);
			g_cat.targetY = (float)disY(g);
		}
	}
	else if (g_cat.state != CatState::ATTACK) {
		float baseSpeed = (g_cat.state == CatState::CHASE) ? 8.0f : 3.0f;
		float finalSpeed = baseSpeed * g_catSpeedMult;

		g_cat.x += (dx / dist) * finalSpeed;
		g_cat.y += (dy / dist) * finalSpeed;
	}

	// 일반 애니메이션 프레임 순환
	g_cat.frame = (g_cat.frame + 1) % maxFrame;
}

void cat_reset(HWND hWnd)
{
	g_cat.state = CatState::IDLE;
	g_cat.motionRow = 5;
	g_cat.frame = 0;
	g_cat.isLeft = false;
	g_cat.isLocked = false;

	g_isMouseDown = false;
	g_cat.lastActionTime = 0;

	g_catSpeedMult = 1.0f;

	g_food.exists = false;

	InvalidateRect(hWnd, NULL, FALSE);
}

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

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC;
	HPEN hPen, oldPen;
	HBRUSH hBrush, oldBrush;
	static RECT rt;
	static int mx, my;

	switch (uMsg) {
	case WM_CREATE:
		img_map.Load(TEXT("map.png"));
		img_cat.Load(TEXT("Cat_Grey_White.png"));
		img_mouse.Load(TEXT("MouseIdle.png"));

		g_cat = { 400, 400, 100, 100, 0, 0, CatState::IDLE, 0 };
		SetTimer(hWnd, 1, 30, NULL);
		break;
	case WM_SIZE:
		InvalidateRect(hWnd, NULL, FALSE);
		return 0;
	case WM_GETMINMAXINFO:
	{
		LPMINMAXINFO lpmmi = (LPMINMAXINFO)lParam;

		lpmmi->ptMinTrackSize.x = LEN;
		lpmmi->ptMinTrackSize.y = HEI;
	}
	return 0;
	case WM_KEYDOWN:
		if (wParam == VK_OEM_PLUS || wParam == VK_ADD) {
			g_catSpeedMult += 0.2f;
			if (g_catSpeedMult > 5.0f) g_catSpeedMult = 5.0f;
		}
		else if (wParam == VK_OEM_MINUS || wParam == VK_SUBTRACT) {
			g_catSpeedMult -= 0.2f;
			if (g_catSpeedMult < 0.2f) g_catSpeedMult = 0.2f;
		}
		else if (wParam == 'R') {
			cat_reset(hWnd);
		}
		else if (wParam == 'Q' || wParam == VK_ESCAPE) {
			PostQuitMessage(0);
			break;
		}
	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd, &rt);
		{
			// 백버퍼 생성
			HDC memDC = CreateCompatibleDC(hDC);
			HBITMAP hBit = CreateCompatibleBitmap(hDC, rt.right, rt.bottom);
			HBITMAP oldBit = (HBITMAP)SelectObject(memDC, hBit);

			// 배경 지우기 (더블 버퍼링의 핵심: 하얀색으로 백버퍼를 채움)
			//FillRect(memDC, &ps.rcPaint, (HBRUSH)GetStockObject(WHITE_BRUSH));
			if (!img_map.IsNull()) {
				img_map.Draw(memDC, 0, 0, rt.right, rt.bottom);
			}
			//------------------------------------------------------------------
			if (g_isMouseDown && !img_mouse.IsNull()) {
				int mWidth = img_mouse.GetWidth() / 2;
				img_mouse.Draw(memDC, (int)g_mouse.x - MOUSE_SIZE / 2, (int)g_mouse.y - MOUSE_SIZE / 2,
					MOUSE_SIZE, MOUSE_SIZE, g_mouse.frame * mWidth, 0, mWidth, img_mouse.GetHeight());
			}

			if (!img_cat.IsNull()) {
				// 행/프레임 계산
				int localFrame = g_cat.frame;


				int srcX = localFrame * 32;
				int srcY = g_cat.motionRow * 32;

				if (g_cat.isLeft) {
					HDC tempDC = CreateCompatibleDC(memDC);
					HBITMAP tempBit = CreateCompatibleBitmap(memDC, 32, 32);
					HBITMAP oldTempBit = (HBITMAP)SelectObject(tempDC, tempBit);

					HDC catDC = img_cat.GetDC();
					StretchBlt(tempDC, 31, 0, -32, 32, catDC, srcX, srcY, 32, 32, SRCCOPY);
					img_cat.ReleaseDC();

					BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };		// 원본 애니메이션의 투명한 배경 정보 활용을 위한 코드
					GdiAlphaBlend(memDC, (int)g_cat.x - CAT_SIZE / 2, (int)g_cat.y - CAT_SIZE / 2,
						CAT_SIZE, CAT_SIZE, tempDC, 0, 0, 32, 32, bf);

					SelectObject(tempDC, oldTempBit);
					DeleteObject(tempBit);
					DeleteDC(tempDC);
				}
				else {
					// 오른쪽은 문제 없으므로 그대로 Draw
					img_cat.Draw(memDC, (int)g_cat.x - CAT_SIZE / 2, (int)g_cat.y - CAT_SIZE / 2,
						CAT_SIZE, CAT_SIZE, srcX, srcY, 32, 32);
				}
			}

			if (g_food.exists && !img_cat.IsNull()) {
				// 58번 줄, 첫 번째 프레임 고정 (32x32 크기 가정)
				int foodSrcX = 0;           // 첫 번째 프레임
				int foodSrcY = 58 * 32;     // 58번 줄

				img_cat.Draw(memDC, (int)g_food.x - CAT_SIZE / 2, (int)g_food.y - CAT_SIZE / 2,
					CAT_SIZE, CAT_SIZE, foodSrcX, foodSrcY, 32, 32);
			}
			//------------------------------------------------------------------
			// 완성된 백버퍼를 실제 화면으로 한 번에 복사
			BitBlt(hDC, 0, 0, rt.right, rt.bottom, memDC, 0, 0, SRCCOPY);

			SelectObject(memDC, oldBit);
			DeleteObject(hBit);
			DeleteDC(memDC);
		}
		EndPaint(hWnd, &ps);
		break;
	
	case WM_ERASEBKGND:
		return 1;

	case WM_LBUTTONDOWN:
	{
		if (g_cat.isLocked) return 0; // 애니메이션 중이면 모든 입력 무시

		mx = LOWORD(lParam);
		my = HIWORD(lParam);

		// 고양이 클릭 판정
		float dx = mx - g_cat.x;
		float dy = my - g_cat.y;
		float dist = sqrt(dx * dx + dy * dy);

		if (dist < CAT_SIZE / 2) {
			// 고양이 클릭 시 특수 애니메이션 시작
			g_cat.state = CatState::SPECIAL;
			g_cat.motionRow = 66;
			g_cat.frame = 0;
			g_cat.isLocked = true;
			g_isMouseDown = false;
		}
		else {
			// 클릭 시 쥐
			g_isMouseDown = true;
			g_mouse.x = (float)mx;
			g_mouse.y = (float)my;
		}
	}
		break;
	case WM_MOUSEMOVE:
		if (g_isMouseDown) {
			g_mouse.x = LOWORD(lParam);
			g_mouse.y = HIWORD(lParam);
		}
		break;
	case WM_LBUTTONUP:
		g_isMouseDown = false;
		if (!g_cat.isLocked) {
			if (g_cat.state != CatState::ATTACK) {
				g_cat.state = CatState::IDLE;
			}
		}
		break;
	case WM_RBUTTONDOWN:
		// 이미 먹이가 있거나 특수 애니메이션 중이면 무시
		if (g_food.exists || g_cat.isLocked) return 0;

		g_food.x = (float)LOWORD(lParam);
		g_food.y = (float)HIWORD(lParam);
		g_food.exists = true;

		// 먹이를 먹기 전까지 다른 행동 차단
		g_cat.isLocked = true;
		g_cat.state = CatState::CHASE;
		g_cat.targetX = g_food.x;
		g_cat.targetY = g_food.y;
		g_cat.motionRow = 39;
		break;
	case WM_TIMER:
		UpdateCatMotion(hWnd);
		g_mouse.frame = (g_mouse.frame + 1) % 2;
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}