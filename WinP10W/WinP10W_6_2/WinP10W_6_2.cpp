#include <windows.h>
#include <tchar.h>
#include <random>
#include <vector>
#include <math.h>
#include <algorithm>
#include <atlimage.h>
#include "resource.h"

#define LEN 1000
#define HEI 1000

using namespace std;
random_device rd;
mt19937 g(rd());
uniform_int_distribution<> uid_rgb{ 0, 255 };
uniform_int_distribution<> disX{ 50, LEN - 50 };
uniform_int_distribution<> disY{ 50, HEI - 50 };
uniform_int_distribution<> disTime{ 1000, 3000 };

int rgb_r = 0;
int rgb_g = 0;
int rgb_b = 0;
int check_rgb[3] = { 0, 0, 0 };
bool isReverse = false;

HINSTANCE g_hInst;
HWND g_hDlg = NULL;
LPCTSTR lpszClass = L"My Window Class";
LPCTSTR lpszWindowName = L"windows program 2";

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK Dlalog_Proc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

bool isSin = false;
bool isHalfCircle = false;
bool isSpring = false;
bool isStairWay = false;
bool isMoveX = false;
bool isMoveY = false;
bool isStop = false;
bool isCircleMoving = false;
int radius = 125;
double circleX = 0.0;	// 타이머에서 원 이동 속도 관리
int circleRadius = 20;
double circleProgress = 0.0;
int finalCircleX = 0;
int finalCircleY = 0;
bool isHalfCircleForward = true;

bool isAnimX = false;
bool isAnimY = false;

int offsetX = 0;
double timeY = 0.0;

double scaleY = 1.0;

void UpdateCirclePosition()
{
	int centerY = HEI / 2;

	if (isSin) {
		int amplitude = 200;
		int periods = 3;
		double radian = (double)((int)circleX + offsetX) / LEN * (2 * 3.14159265358979323846) * periods;
		finalCircleX = (int)circleX;
		finalCircleY = centerY - (int)(sin(radian) * amplitude * scaleY);
	}

	else if (isHalfCircle) {
		int diameter = radius * 2;
		finalCircleX = (int)circleX;
		int scrollX = offsetX % diameter;

		if (isHalfCircleForward) {
			int virtualX = (finalCircleX - scrollX) % diameter;
			if (virtualX < 0) virtualX += diameter;
			double dx = virtualX - radius;
			double dySq = (double)radius * radius - dx * dx;
			finalCircleY = centerY;
			if (dySq >= 0) finalCircleY = centerY - (int)(sqrt(dySq) * scaleY);
		}
		else {
			int virtualX = (finalCircleX + scrollX) % diameter;
			if (virtualX < 0) virtualX += diameter;
			double dx = virtualX - radius;
			double dySq = (double)radius * radius - dx * dx;
			finalCircleY = centerY;
			if (dySq >= 0) finalCircleY = centerY + (int)(sqrt(dySq) * scaleY);
		}
	}

	else if (isSpring) {
		double speed = 20.0;
		int springRadius = 125;
		double theta = circleX * 0.05;

		finalCircleX = (int)(speed * theta + springRadius * cos(theta)) - offsetX;
		finalCircleY = centerY - (int)(springRadius * sin(theta) * scaleY);
	}

	else if (isStairWay) {
		int currentX = 0;
		int currentY = centerY;
		int step = 0;

		double targetLength = circleX;
		double accumulatedLength = 0.0;

		int baseX = 0;
		int baseY = centerY;

		while (currentX < LEN + 2000) {
			int state = step % 4;
			int nextX = currentX;
			int nextY = currentY;
			int segmentLength = 0;

			if (state == 0) {
				int segmentHeight = (step == 0) ? 200 : 400;
				nextY -= segmentHeight;
				segmentLength = segmentHeight;

				if (accumulatedLength + segmentLength >= targetLength) {
					double rem = targetLength - accumulatedLength;
					baseX = currentX; baseY = currentY - (int)rem;
					break;
				}
			}
			else if (state == 1) {
				nextX += 100; segmentLength = 100;
				if (accumulatedLength + segmentLength >= targetLength) {
					double rem = targetLength - accumulatedLength;
					baseX = currentX + (int)rem; baseY = currentY;
					break;
				}
			}
			else if (state == 2) {
				int segmentHeight = 400;
				nextY += segmentHeight;
				segmentLength = segmentHeight;

				if (accumulatedLength + segmentLength >= targetLength) {
					double rem = targetLength - accumulatedLength;
					baseX = currentX; baseY = currentY + (int)rem;
					break;
				}
			}
			else if (state == 3) {
				nextX += 100; segmentLength = 100;
				if (accumulatedLength + segmentLength >= targetLength) {
					double rem = targetLength - accumulatedLength;
					baseX = currentX + (int)rem; baseY = currentY;
					break;
				}
			}

			accumulatedLength += segmentLength;
			currentX = nextX; currentY = nextY; step++;
		}

		finalCircleX = baseX - offsetX;
		finalCircleY = centerY + (int)((baseY - centerY) * scaleY);

		if (finalCircleX < 0) {
			finalCircleX = (finalCircleX % LEN) + LEN;
		}
		finalCircleX = finalCircleX % LEN;
	}
	else {
		finalCircleX = (int)circleX;
		finalCircleY = centerY;
	}
}

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
	while (GetMessage(&Message, NULL, 0, 0))
	{
		if (g_hDlg == NULL || !IsDialogMessage(g_hDlg, &Message))
		{
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}
	}
	return Message.wParam;
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
		g_hDlg = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, (DLGPROC)Dlalog_Proc);

		if (g_hDlg != NULL) {
			RECT rcMain, rcDlg;
			GetWindowRect(hWnd, &rcMain);

			GetWindowRect(g_hDlg, &rcDlg);
			int dlgWidth = rcDlg.right - rcDlg.left;
			int dlgHeight = rcDlg.bottom - rcDlg.top;

			SetWindowPos(g_hDlg, NULL,
				rcMain.right + 10,
				rcMain.top,
				dlgWidth, dlgHeight,
				SWP_NOZORDER | SWP_NOSIZE | SWP_SHOWWINDOW);

			ShowWindow(g_hDlg, SW_SHOW);
		}
		
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
	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);
		{
			GetClientRect(hWnd, &rt);
			
			// 백버퍼 생성
			HDC memDC = CreateCompatibleDC(hDC);
			HBITMAP hBit = CreateCompatibleBitmap(hDC, rt.right, rt.bottom);
			HBITMAP oldBit = (HBITMAP)SelectObject(memDC, hBit);

			// 배경 지우기 (더블 버퍼링의 핵심: 하얀색으로 백버퍼를 채움)
			FillRect(memDC, &ps.rcPaint, (HBRUSH)GetStockObject(WHITE_BRUSH));

			//-------------------------------------
			hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
			oldPen = (HPEN)SelectObject(memDC, hPen);

			MoveToEx(memDC, LEN / 2, 0, NULL); LineTo(memDC, LEN / 2, rt.bottom);
			MoveToEx(memDC, 0, HEI / 2, NULL); LineTo(memDC, rt.right, HEI / 2);

			SelectObject(memDC, oldPen);
			DeleteObject(hPen);
			//-------------------------------------함수 그리기

			if (!isReverse) {
				hPen = CreatePen(PS_SOLID, 2, RGB(rgb_r, rgb_g, rgb_b));
				oldPen = (HPEN)SelectObject(memDC, hPen);
			}
			else {
				hPen = CreatePen(PS_SOLID, 2, RGB(255 - rgb_r, 255 - rgb_g, 255 - rgb_b));
				oldPen = (HPEN)SelectObject(memDC, hPen);
			}

			if(isSin){
				int centerX = LEN / 2;
				int centerY = HEI / 2;
				int amplitude = (int)(200 * scaleY);
				int periods = 3;

				for (int x = 0; x < LEN; ++x) {
					double radian = (double)(x + offsetX) / LEN * (2 * 3.14159265358979323846) * periods;

					int y = centerY - (int)(sin(radian) * amplitude);

					if (x == 0) {
						MoveToEx(memDC, x, y, NULL);
					}
					else {
						LineTo(memDC, x, y);
					}
				}
			}
			else if (isHalfCircle) {
				int centerY = HEI / 2;

				int currentRadius = (int)(radius * scaleY);
				int diameter = radius * 2;

				int scrollX = offsetX % diameter;

				for (int i = -1; i < 4; ++i) {
					int curX = (i * diameter) + scrollX;

					Arc(memDC,
						curX, centerY - currentRadius, curX + diameter, centerY + currentRadius,
						curX + diameter, centerY, curX, centerY);
				}

				for (int i = 0; i < 5; ++i) {
					int curX = (i * diameter) - scrollX;

					Arc(memDC,
						curX, centerY - currentRadius, curX + diameter, centerY + currentRadius,
						curX, centerY, curX + diameter, centerY);
				}
			}
			else if (isSpring) {
				int centerY = HEI / 2;
				int springRadius = 125;
				double speed = 20.0;

				int startPoint = (int)((offsetX - 300) / (speed * 0.1));
				int maxPoints = startPoint + 1000;

				bool isFirstPoint = true;

				for (int i = startPoint; i < maxPoints; ++i) {
					double theta = (double)i * 0.1;

					int x = (int)(speed * theta + springRadius * cos(theta)) - offsetX;
					int y = centerY - (int)(springRadius * sin(theta) * scaleY);

					if (x < -200) continue;
					if (x > LEN + 200) break;

					if (isFirstPoint) {
						MoveToEx(memDC, x, y, NULL);
						isFirstPoint = false;
					}
					else {
						LineTo(memDC, x, y);
					}
				}
			}
			else if (isStairWay) {
				int startX = -(offsetX % 200);
				int startY = HEI / 2;

				MoveToEx(memDC, startX, startY, NULL);

				int currentX = startX;
				int currentY = startY;
				int step = 0;

				while (currentX < LEN) {
					int state = step % 4;

					if (state == 0) {
						if (step == 0) {
							currentY -= (int)(200 * scaleY);
						}
						else {
							currentY -= (int)(400 * scaleY);
						}
						LineTo(memDC, currentX, currentY);
					}
					else if (state == 1) {
						currentX += 100;
						LineTo(memDC, currentX, currentY);
					}
					else if (state == 2) {
						currentY += (int)(400 * scaleY);
						LineTo(memDC, currentX, currentY);
					}
					else if (state == 3) {
						currentX += 100;
						LineTo(memDC, currentX, currentY);
					}

					step++;
				}
			}


			SelectObject(memDC, oldPen);
			DeleteObject(hPen);
			// ------------------------------------------------------- 원 그리기
			if (isCircleMoving) {
				UpdateCirclePosition();

				HBRUSH circleBrush = CreateSolidBrush(RGB(255, 255, 255));
				HBRUSH oldB = (HBRUSH)SelectObject(memDC, circleBrush);
				HPEN circlePen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
				HPEN oldP = (HPEN)SelectObject(memDC, circlePen);

				Ellipse(memDC, finalCircleX - circleRadius, finalCircleY - circleRadius,
					finalCircleX + circleRadius, finalCircleY + circleRadius);

				SelectObject(memDC, oldP); DeleteObject(circlePen);
				SelectObject(memDC, oldB); DeleteObject(circleBrush);

				SetBkMode(memDC, TRANSPARENT);
				SetTextColor(memDC, RGB(0, 0, 0));

				TCHAR text[] = _T("It's Moving");
				RECT textRect;
				textRect.left = finalCircleX - 100;
				textRect.right = finalCircleX + 100;
				textRect.top = finalCircleY - circleRadius - 25;
				textRect.bottom = finalCircleY - circleRadius;

				DrawText(memDC, text, -1, &textRect, DT_CENTER | DT_NOCLIP);
			}
			//-------------------------------------

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
		

		InvalidateRect(hWnd, NULL, FALSE);
		break;
	case WM_RBUTTONDOWN:
		

		InvalidateRect(hWnd, NULL, FALSE);
		break;
	case WM_TIMER:
		if (wParam == 1) {
			if (isAnimX) {
				offsetX += 5;
			}
			if (isAnimY) {
				timeY += 0.05;
				scaleY = 1.0 + sin(timeY);
			}

			if (isCircleMoving) {
				if (isSin) {
					circleX += 4.0;
					if (circleX > LEN) circleX = 0;
				}
				else if (isHalfCircle) {
					if (isHalfCircleForward) {
						circleX += 8.0;
						if (circleX > LEN) { circleX = LEN; isHalfCircleForward = false; }
					}
					else {
						circleX -= 8.0;
						if (circleX < 0) { circleX = 0; isHalfCircleForward = true; }
					}
				}
				else if (isSpring) {
					circleX += 1.5;

					double wrapAmount = 360.0 * 3.14159265358979323846;

					if (finalCircleX > LEN + 100) {
						circleX -= wrapAmount;
					}
					else if (finalCircleX < -100) {
						circleX += wrapAmount;
					}
				}
				else if (isStairWay) {
					circleX += 15.0;

					if (circleX >= 5200.0) {
						circleX -= 5000.0;
					}
				}
				else {
					circleX += 4.0;
					if (circleX > LEN) circleX = 0;
				}
			}

			if (!isAnimX && !isAnimY && !isCircleMoving) {
				KillTimer(hWnd, 1);
			}
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


BOOL CALLBACK Dlalog_Proc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	HWND hParentWnd = GetParent(hDlg);

	switch (iMsg) {
	case WM_INITDIALOG:
		CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO4, NULL);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_RADIO1:
			isSin = true;
			isHalfCircle = false;
			isSpring = false;
			isStairWay = false;
			InvalidateRect(hParentWnd, NULL, FALSE);
			break;
		case IDC_RADIO2:
			isSin = false;
			isHalfCircle = true;
			isSpring = false;
			isStairWay = false;
			InvalidateRect(hParentWnd, NULL, FALSE);
			break;
		case IDC_RADIO3:
			isSin = false;
			isHalfCircle = false;
			isSpring = true;
			isStairWay = false;
			InvalidateRect(hParentWnd, NULL, FALSE);
			break;
		case IDC_RADIO4:
			isSin = false;
			isHalfCircle = false;
			isSpring = false;
			isStairWay = true;
			InvalidateRect(hParentWnd, NULL, FALSE);
			break;
		case IDC_CHECK1:
			check_rgb[0] = 1 - check_rgb[0];
			rgb_r = check_rgb[0] * 255;
			InvalidateRect(hParentWnd, NULL, FALSE);
			break;
		case IDC_CHECK2:
			check_rgb[1] = 1 - check_rgb[1];
			rgb_g = check_rgb[1] * 255;
			InvalidateRect(hParentWnd, NULL, FALSE);
			break;
		case IDC_CHECK3:
			check_rgb[2] = 1 - check_rgb[2];
			rgb_b = check_rgb[2] * 255;
			InvalidateRect(hParentWnd, NULL, FALSE);
			break;
		case IDC_CHECK4:
			isReverse = !isReverse;
			InvalidateRect(hParentWnd, NULL, FALSE);
			break;
		case IDC_BUTTON1:
			isAnimX = !isAnimX;
			if (isAnimX || isAnimY) {
				SetTimer(hParentWnd, 1, 30, NULL);
			}
			break;
		case IDC_BUTTON2:
			isAnimY = !isAnimY;
			if (isAnimX || isAnimY) {
				SetTimer(hParentWnd, 1, 30, NULL);
			}
			break;
		case IDC_BUTTON3:
			isAnimX = false;
			isAnimY = false;
			KillTimer(hParentWnd, 1);

			InvalidateRect(hParentWnd, NULL, FALSE);
			break;
		case IDC_BUTTON4:
			isAnimX = false;
			isAnimY = false;
			isCircleMoving = false;
			circleX = 0;
			isHalfCircleForward = true;
			offsetX = 0;
			timeY = 0.0;
			scaleY = 1.0;

			if (hParentWnd != NULL) {
				KillTimer(hParentWnd, 1);
			}

			isSin = false;
			isHalfCircle = false;
			isSpring = false;
			isStairWay = false;

			rgb_r = 0;
			rgb_g = 0;
			rgb_b = 0;
			check_rgb[0] = 0;
			check_rgb[1] = 0;
			check_rgb[2] = 0;
			isReverse = false;

			CheckDlgButton(hDlg, IDC_RADIO1, BST_UNCHECKED);
			CheckDlgButton(hDlg, IDC_RADIO2, BST_UNCHECKED);
			CheckDlgButton(hDlg, IDC_RADIO3, BST_UNCHECKED);
			CheckDlgButton(hDlg, IDC_RADIO4, BST_UNCHECKED);

			CheckDlgButton(hDlg, IDC_CHECK1, BST_UNCHECKED);
			CheckDlgButton(hDlg, IDC_CHECK2, BST_UNCHECKED);
			CheckDlgButton(hDlg, IDC_CHECK3, BST_UNCHECKED);
			CheckDlgButton(hDlg, IDC_CHECK4, BST_UNCHECKED);

			InvalidateRect(hParentWnd, NULL, FALSE);
			break;
		case IDC_BUTTON5:
			isCircleMoving = !isCircleMoving;
			if (isCircleMoving) {
				isHalfCircleForward = true;

				if (isSin || isHalfCircle) {
					circleX = LEN / 2;
				}
				else {
					circleX = 0;
				}

				SetTimer(hParentWnd, 1, 30, NULL);
			}
			InvalidateRect(hParentWnd, NULL, FALSE);
			break;
		case IDOK:
			isAnimX = false;
			isAnimY = false;
			isCircleMoving = false;
			circleX = 0;
			isHalfCircleForward = true;
			offsetX = 0;
			timeY = 0.0;
			scaleY = 1.0;

			if (hParentWnd != NULL) {
				KillTimer(hParentWnd, 1);
			}

			isSin = false;
			isHalfCircle = false;
			isSpring = false;
			isStairWay = false;

			rgb_r = 0;
			rgb_g = 0;
			rgb_b = 0;
			check_rgb[0] = 0;
			check_rgb[1] = 0;
			check_rgb[2] = 0;
			isReverse = false;

			CheckDlgButton(hDlg, IDC_RADIO1, BST_UNCHECKED);
			CheckDlgButton(hDlg, IDC_RADIO2, BST_UNCHECKED);
			CheckDlgButton(hDlg, IDC_RADIO3, BST_UNCHECKED);
			CheckDlgButton(hDlg, IDC_RADIO4, BST_UNCHECKED);

			CheckDlgButton(hDlg, IDC_CHECK1, BST_UNCHECKED);
			CheckDlgButton(hDlg, IDC_CHECK2, BST_UNCHECKED);
			CheckDlgButton(hDlg, IDC_CHECK3, BST_UNCHECKED);
			CheckDlgButton(hDlg, IDC_CHECK4, BST_UNCHECKED);

			InvalidateRect(hParentWnd, NULL, FALSE);
			break;
		case IDCANCEL:
			EndDialog(hDlg, 0);
			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg, 0);
		break;
	}
	return 0;
}