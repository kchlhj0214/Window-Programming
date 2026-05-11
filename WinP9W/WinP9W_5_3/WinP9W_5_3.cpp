#include <windows.h>
#include <tchar.h>
#include <random>
#include <vector>
#include <math.h>
#include <algorithm>
#include "resource.h"

#define LEN 1000
#define HEI 1000
#define GRIP_SIZE 10
#define MAX_ZOOM 2.0f
#define MIN_ZOOM 0.5f

using namespace std;
random_device rd;
mt19937 g(rd());
uniform_int_distribution<> uid_rgb{ 0, 255 };
uniform_int_distribution<> uid_speed{ 1, 10 };
uniform_int_distribution<> uid_pos3{ 0, 7 };
uniform_int_distribution<> uid_pos4{ 0, 14 };
uniform_int_distribution<> uid_pos5{ 0, 23 };



HINSTANCE g_hInst;
LPCTSTR lpszClass = L"My Window Class";
LPCTSTR lpszWindowName = L"windows program 2";

POINT startPT, endPT;
RECT readingGlasses;
bool isRectExist = false;
bool isMakingRect = false;
bool isMovingRect = false;
int grabSide = 0;
POINT lastMousePos;
float fixedAspect = 1.0f;
bool isA = false;
int curPic = 1;
RECT viewRect;
float curZoom = 1.0f;

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
void UpdateViewRect(int winW, int winH, int bmpW, int bmpH) {
	// 화면상 사각형의 중심점 계산 (윈도우 기준)
	int screenCenterX = (readingGlasses.left + readingGlasses.right) / 2;
	int screenCenterY = (readingGlasses.top + readingGlasses.bottom) / 2;

	// 화면 중심점을 원본 이미지상의 중심점으로 변환
	int imgCenterX = screenCenterX * bmpW / winW;
	int imgCenterY = screenCenterY * bmpH / winH;

	// 돋보기 사각형이 원본 이미지에서 차지해야 할 가로/세로 크기 계산
	int viewW = (int)((readingGlasses.right - readingGlasses.left) * bmpW / (winW * curZoom));
	int viewH = (int)((readingGlasses.bottom - readingGlasses.top) * bmpH / (winH * curZoom));

	// 원본 이미지 범위를 벗어나지 않도록 viewW, viewH 보정 (축소 제한)
	if (viewW > bmpW) {
		viewW = bmpW;
		viewH = (int)(viewW / fixedAspect);
	}
	if (viewH > bmpH) {
		viewH = bmpH;
		viewW = (int)(viewH * fixedAspect);
	}

	// 계산된 이미지 중심점을 기준으로 viewRect 설정
	viewRect.left = imgCenterX - viewW / 2;
	viewRect.top = imgCenterY - viewH / 2;
	viewRect.right = viewRect.left + viewW;
	viewRect.bottom = viewRect.top + viewH;

	// 이미지 경계 밖으로 나가지 않게 최종 클램핑(Clamping)
	if (viewRect.left < 0) { viewRect.left = 0; viewRect.right = viewW; }
	if (viewRect.top < 0) { viewRect.top = 0; viewRect.bottom = viewH; }
	if (viewRect.right > bmpW) { viewRect.right = bmpW; viewRect.left = bmpW - viewW; }
	if (viewRect.bottom > bmpH) { viewRect.bottom = bmpH; viewRect.top = bmpH - viewH; }
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC;
	HPEN hPen, oldPen;
	HBRUSH hBrush, oldBrush;
	static RECT rt;
	static int mx, my;
	static BITMAP bmp1;
	static BITMAP bmp2;
	static HBITMAP hBitmap1, hBitmap2;

	switch (uMsg) {
	case WM_CREATE:
		hBitmap1 = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP1));
		hBitmap2 = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP2));
		GetObject(hBitmap1, sizeof(BITMAP), &bmp1);
		GetObject(hBitmap2, sizeof(BITMAP), &bmp2);
		break;
	case WM_KEYDOWN:
		GetClientRect(hWnd, &rt);
		if (wParam == 'A') {
			isA = !isA;
		}
		else if (wParam == '1') {
			curPic = 1;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == '2') {
			curPic = 2;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'E') {
			curZoom *= 1.1f;
			if (curZoom > MAX_ZOOM) curZoom = 1.0f; 

			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'S') {
			curZoom *= 0.9f;
			if (curZoom < MIN_ZOOM) curZoom = 1.0f;

			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'B') {
			curZoom = 1.0f;

			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'C') {
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'P') {
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'F') {
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'H') {
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'V') {
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'M') {
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'N') {
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'I') {
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'R') {
			isRectExist = false;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'Q' || wParam == VK_ESCAPE) {
			DeleteObject(hBitmap1);
			DeleteObject(hBitmap2);
			PostQuitMessage(0);
			break;
		}
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
			//------------------------------------------------------------------
			HDC hImgDC = CreateCompatibleDC(hDC);
			if (curPic == 1) SelectObject(hImgDC, hBitmap1);
			else if (curPic == 2) SelectObject(hImgDC, hBitmap2);

			int sourceW, sourceH;
			if (curPic == 1) {
				sourceW = bmp1.bmWidth;
				sourceH = bmp1.bmHeight;
			}
			else {
				sourceW = bmp2.bmWidth;
				sourceH = bmp2.bmHeight;
			}
			StretchBlt(memDC, 0, 0, rt.right, rt.bottom, hImgDC, 0, 0, sourceW, sourceH, SRCCOPY);

			if (isRectExist) {
				UpdateViewRect(rt.right, rt.bottom, sourceW, sourceH);

				StretchBlt(memDC,
					readingGlasses.left, readingGlasses.top,
					readingGlasses.right - readingGlasses.left,
					readingGlasses.bottom - readingGlasses.top,
					hImgDC,
					viewRect.left, viewRect.top,
					viewRect.right - viewRect.left,
					viewRect.bottom - viewRect.top,
					SRCCOPY);

				hPen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0)); // 빨간색 펜
				oldPen = (HPEN)SelectObject(memDC, hPen);

				oldBrush = (HBRUSH)SelectObject(memDC, GetStockObject(NULL_BRUSH));

				Rectangle(memDC, readingGlasses.left, readingGlasses.top,
					readingGlasses.right, readingGlasses.bottom);

				SelectObject(memDC, oldPen);
				SelectObject(memDC, oldBrush);
				DeleteObject(hPen);
			}
			DeleteDC(hImgDC);
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
		mx = LOWORD(lParam);
		my = HIWORD(lParam);

		if (isRectExist) {
			RECT rc = readingGlasses;
			grabSide = 0;

			// 1. 꼭짓점 판별 (비율 유지)
			if (abs(mx - rc.left) < GRIP_SIZE && abs(my - rc.top) < GRIP_SIZE) grabSide = 5;      // 좌상
			else if (abs(mx - rc.right) < GRIP_SIZE && abs(my - rc.top) < GRIP_SIZE) grabSide = 6; // 우상
			else if (abs(mx - rc.left) < GRIP_SIZE && abs(my - rc.bottom) < GRIP_SIZE) grabSide = 7; // 좌하
			else if (abs(mx - rc.right) < GRIP_SIZE && abs(my - rc.bottom) < GRIP_SIZE) grabSide = 8; // 우하

			// 2. 테두리 판별 (단방향)
			else if (abs(my - rc.top) < GRIP_SIZE && mx > rc.left && mx < rc.right) grabSide = 1;    // 상
			else if (abs(my - rc.bottom) < GRIP_SIZE && mx > rc.left && mx < rc.right) grabSide = 2; // 하
			else if (abs(mx - rc.left) < GRIP_SIZE && my > rc.top && my < rc.bottom) grabSide = 3;   // 좌
			else if (abs(mx - rc.right) < GRIP_SIZE && my > rc.top && my < rc.bottom) grabSide = 4;  // 우

			// 3. 내부 판별 (이동)
			else if (PtInRect(&rc, { mx, my })) grabSide = 9;

			if (grabSide > 0) {
				isMovingRect = true;
				lastMousePos = { mx, my };
				// 현재 비율 저장 (꼭짓점 드래그 시 사용)
				fixedAspect = (float)(rc.right - rc.left) / (rc.bottom - rc.top);
				return 0;
			}
			else {
				return 0;
			}
		}
		else {
			startPT = { mx, my };
			isMakingRect = true;
			isRectExist = true;
			readingGlasses = { mx, my, mx, my };
		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	case WM_MOUSEMOVE:
		mx = LOWORD(lParam);
		my = HIWORD(lParam);

		if (isMovingRect) {
			int dx = mx - lastMousePos.x;
			int dy = my - lastMousePos.y;
			RECT& rc = readingGlasses;
			float aspect = fixedAspect;

			// 1~4: 테두리 조절 (최소 크기 10)
			if (grabSide == 1) { // 상단
				rc.top = min(my, rc.bottom - GRIP_SIZE * 2);
			}
			else if (grabSide == 2) { // 하단
				rc.bottom = max(my, rc.top + GRIP_SIZE * 2);
			}
			else if (grabSide == 3) { // 좌측
				rc.left = min(mx, rc.right - GRIP_SIZE * 2);
			}
			else if (grabSide == 4) { // 우측
				rc.right = max(mx, rc.left + GRIP_SIZE * 2);
			}

			// 5~8: 꼭짓점 조절
			else if (grabSide >= 5 && grabSide <= 8) {
				if (isA) { // 자유 변형
					if (grabSide == 5) { rc.left = min(mx, rc.right - GRIP_SIZE * 2); rc.top = min(my, rc.bottom - GRIP_SIZE * 2); }
					else if (grabSide == 6) { rc.right = max(mx, rc.left + GRIP_SIZE * 2); rc.top = min(my, rc.bottom - GRIP_SIZE * 2); }
					else if (grabSide == 7) { rc.left = min(mx, rc.right - GRIP_SIZE * 2); rc.bottom = max(my, rc.top + GRIP_SIZE * 2); }
					else if (grabSide == 8) { rc.right = max(mx, rc.left + GRIP_SIZE * 2); rc.bottom = max(my, rc.top + GRIP_SIZE * 2); }
				}
				else { // 비율 유지
					int newWidth, newHeight;
					if (grabSide == 5 || grabSide == 7) { // 왼쪽 라인
						newWidth = max(GRIP_SIZE * 2, abs(rc.right - (rc.left + dx)));
						newHeight = (int)(newWidth / aspect);
						if (newWidth > GRIP_SIZE * 2) {
							rc.left = rc.right - newWidth;
							if (grabSide == 5) rc.top = rc.bottom - newHeight;
							else rc.bottom = rc.top + newHeight;
						}
					}
					else { // 오른쪽 라인
						newWidth = max(GRIP_SIZE * 2, abs((rc.right + dx) - rc.left));
						newHeight = (int)(newWidth / aspect);
						if (newWidth > GRIP_SIZE * 2) {
							rc.right = rc.left + newWidth;
							if (grabSide == 6) rc.top = rc.bottom - newHeight;
							else rc.bottom = rc.top + newHeight;
						}
					}
				}
			}
			// 9: 이동
			else if (grabSide == 9) {
				OffsetRect(&rc, dx, dy);

				if (rc.left < 0) OffsetRect(&rc, -rc.left, 0);
				if (rc.right > rt.right) OffsetRect(&rc, rt.right - rc.right, 0);

				if (rc.top < 0) OffsetRect(&rc, 0, -rc.top);
				if (rc.bottom > rt.bottom) OffsetRect(&rc, 0, rt.bottom - rc.bottom);
			}

			lastMousePos = { mx, my };
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (isMakingRect) {
			readingGlasses.right = mx;
			readingGlasses.bottom = my;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;
	case WM_LBUTTONUP:
		if (isMakingRect || isMovingRect) {
			if (readingGlasses.left > readingGlasses.right) swap(readingGlasses.left, readingGlasses.right);
			if (readingGlasses.top > readingGlasses.bottom) swap(readingGlasses.top, readingGlasses.bottom);

			if (abs(readingGlasses.right - readingGlasses.left) < 2 && abs(readingGlasses.bottom - readingGlasses.top) < 2) {
				isRectExist = false;
			}
		}
		isMovingRect = false;
		isMakingRect = false;
		grabSide = 0;

		InvalidateRect(hWnd, NULL, FALSE);
		break;
	break;
	case WM_LBUTTONDBLCLK:
	{
		
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}
	
	case WM_TIMER:
		
	case WM_DESTROY:
		DeleteObject(hBitmap1);
		DeleteObject(hBitmap2);
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}