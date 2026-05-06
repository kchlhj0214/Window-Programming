#include <windows.h>
#include <tchar.h>
#include <random>
#include <vector>
#include <math.h>
#include <algorithm>
#include "resource.h"

#define LEN 1000
#define HEI 1000
#define PLUS 10
#define MINUS -10
#define BORAR_SIZE 5
#define SPEED 10

using namespace std;
random_device rd;
mt19937 g(rd());
uniform_int_distribution<> uid_rgb{ 0, 255 };
uniform_int_distribution<> uid_speed{ 1, 10 };
uniform_int_distribution<> uid_pos3{ 0, 7 };
uniform_int_distribution<> uid_pos4{ 0, 14 };
uniform_int_distribution<> uid_pos5{ 0, 23 };

struct RECTS { int boardPos; RECT curPos;  RECT pic; POINT target; bool isMoving; };

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"My Window Class";
LPCTSTR lpszWindowName = L"windows program 2";

bool IsS = false;
bool IsF = false;
bool IsM = false;
int bmW, bmH;
int pictureDivision = 1;	// 행렬 3 4 5
vector<RECTS> divRects;
int board[BORAR_SIZE][BORAR_SIZE] = {0};
vector<POINT> pos;
int emptyBoard; // 빈 공간의 1차원 좌표
int curPic = 1;	// 현재 사진 1 2
int g_dirX = 0;	// 이동방향 1 0 -1
int g_dirY = 0;
int srtmx, srtmy, endmx, endmy;
int cellW, cellH;

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

void UpdateRects(int mode, int w, int h, HWND hWnd) {
	RECT clientRt;
	GetClientRect(hWnd, &clientRt);

	pos.clear();
	divRects.clear();
	cellW = clientRt.right / mode;
	cellH = clientRt.bottom / mode;

	int imgCellW = w / mode;
	int imgCellH = h / mode;

	int p = 0;
	for (int r = 0; r < mode; r++) {
		for (int c = 0; c < mode; c++) {
			// 좌표 저장
			pos.push_back({ c * cellW, r * cellH });

			if (mode == 1 || p < (mode * mode) - 1) {
				RECTS rct;
				rct.boardPos = p;
				// 초기 그리기 위치
				rct.curPos = { c * cellW, r * cellH, (c + 1) * cellW, (r + 1) * cellH };
				// 원본 이미지에서 잘라낼 영역
				rct.pic = { c * imgCellW, r * imgCellH, (c + 1) * imgCellW, (r + 1) * imgCellH };
				rct.isMoving = false;
				divRects.push_back(rct);
			}
			p++;
		}
	}
	if (mode == 1)
		emptyBoard = -1;
	else
		emptyBoard = mode * mode - 1;
}

void ShuffleRects(vector<RECTS>& r, int mode) {
	vector<int> v;
	for (int i = 0; i < mode * mode; ++i)
		v.push_back(i);

	shuffle(v.begin(), v.end(), g);

	emptyBoard = v.back();
	v.pop_back();

	for (int i = 0; i < r.size(); i++) {
		r[i].boardPos = v[i];

		r[i].curPos.left = pos[v[i]].x;
		r[i].curPos.top = pos[v[i]].y;
		r[i].curPos.right = pos[v[i]].x + cellW;
		r[i].curPos.bottom = pos[v[i]].y + cellH;
	}
}

void MoveRects(HWND hWnd, vector<RECTS>& r, int mode, int dirX, int dirY) {
	if (IsM) return;

	// 현재 빈 칸의 2차원 좌표 계산
	int eRow = emptyBoard / mode;
	int eCol = emptyBoard % mode;

	int targetRow = eRow - dirY;
	int targetCol = eCol - dirX;

	if (targetRow < 0 || targetRow >= mode || targetCol < 0 || targetCol >= mode) {
		return;
	}

	int targetBoardPos = targetRow * mode + targetCol;
	int targetIdx = -1;

	// 해당 위치에 있는 조각 찾기
	for (int i = 0; i < r.size(); ++i) {
		if (r[i].boardPos == targetBoardPos) {
			targetIdx = i;
			break;
		}
	}

	// 조각을 찾았다면 이동 처리
	if (targetIdx != -1) {
		r[targetIdx].isMoving = true;

		// 목표 좌표는 현재 빈 칸의 화면 좌표
		r[targetIdx].target.x = pos[emptyBoard].x;
		r[targetIdx].target.y = pos[emptyBoard].y;

		r[targetIdx].boardPos = emptyBoard;
		emptyBoard = targetBoardPos;

		IsM = true;
		SetTimer(hWnd, 1, 10, NULL);
	}
}

//void MoveRects(HWND hWnd, vector<RECTS>& r, int mode, int dirX, int dirY) {
//	int eRow = emptyBoard / mode;
//	int eCol = emptyBoard % mode;
//
//	vector<int> targets;
//
//	int currRow = eRow - dirY;
//	int currCol = eCol - dirX;
//
//	while (currRow >= 0 && currRow < mode && currCol >= 0 && currCol < mode) {
//		int targetIdx = -1;
//		int targetBoardPos = currRow * mode + currCol;
//
//		for (int i = 0; i < r.size(); ++i) {
//			if (r[i].boardPos == targetBoardPos) {
//				targetIdx = i;
//				break;
//			}
//		}
//
//		if (targetIdx != -1) {
//			targets.push_back(targetIdx);
//		}
//
//		currRow -= dirY;
//		currCol -= dirX;
//	}
//
//	if (targets.empty()) return;
//
//	int cellW = bmW / mode;
//	int cellH = bmH / mode;
//
//	for (int idx : targets) {
//		r[idx].isMoving = true;
//		r[idx].target.x = pos[r[idx].boardPos + (dirY * mode + dirX)].x;
//		r[idx].target.y = pos[r[idx].boardPos + (dirY * mode + dirX)].y;
//
//		r[idx].boardPos += (dirY * mode + dirX);
//	}
//
//	emptyBoard = eRow * mode + eCol + (targets.size() * ((-dirY) * mode + (-dirX)));
//
//	SetTimer(hWnd, 1, 10, NULL);
//}

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
		UpdateRects(1, bmp1.bmWidth, bmp1.bmHeight, hWnd);
		break;
	case WM_KEYDOWN:
		GetClientRect(hWnd, &rt);
		if (wParam == 'S') {
			IsS = true;
			ShuffleRects(divRects, pictureDivision);

			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'F') {
			IsF = !IsF;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'V') {
			
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'H') {
			
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'Q' || wParam == VK_ESCAPE) {
			DeleteObject(hBitmap1);
			DeleteObject(hBitmap2);
			PostQuitMessage(0);
			break;
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
		GetClientRect(hWnd, &rt);
		{
			// 백버퍼 생성
			HDC memDC = CreateCompatibleDC(hDC);
			HBITMAP hBit = CreateCompatibleBitmap(hDC, rt.right, rt.bottom);
			HBITMAP oldBit = (HBITMAP)SelectObject(memDC, hBit);

			// 배경 지우기 (더블 버퍼링의 핵심: 하얀색으로 백버퍼를 채움)
			FillRect(memDC, &ps.rcPaint, (HBRUSH)GetStockObject(WHITE_BRUSH));
			//------------------------------------------------------------------
			int sourceW;
			int sourceH;

			HDC hMemDC = CreateCompatibleDC(hDC);
			if (curPic == 1) {
				SelectObject(hMemDC, hBitmap1); 
				sourceW = bmp1.bmWidth; 
				sourceH = bmp1.bmHeight;
			}
			else if (curPic == 2) {
				SelectObject(hMemDC, hBitmap2);
				sourceW = bmp2.bmWidth; 
				sourceH = bmp2.bmHeight;
			}
			if (pictureDivision == 1) {
				StretchBlt(memDC, 0, 0, rt.right, rt.bottom,
					hMemDC, 0, 0, sourceW, sourceH, SRCCOPY);
			}
			else {
				for (int i = 0; i < divRects.size(); ++i) {
					StretchBlt(memDC, divRects[i].curPos.left, divRects[i].curPos.top, cellW, cellH,
						hMemDC, divRects[i].pic.left, divRects[i].pic.top, (divRects[i].pic.right - divRects[i].pic.left), (divRects[i].pic.bottom - divRects[i].pic.top), SRCCOPY);
				}
			}

			DeleteDC(hMemDC);
			//------------------------------------------------------------------
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
		case ID_PICTURE_PICTURE1:
			curPic = 1;
			UpdateRects(1, bmp1.bmWidth, bmp1.bmHeight, hWnd);
			pictureDivision = 1;
			if(IsS)
				ShuffleRects(divRects, pictureDivision);
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case ID_PICTURE_PICTURE2:
			curPic = 2;
			UpdateRects(1, bmp2.bmWidth, bmp2.bmHeight, hWnd);
			pictureDivision = 1;
			if(IsS)
				ShuffleRects(divRects, pictureDivision);
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case ID_PICTUREDIVISION_3:
			pictureDivision = 3;
			if(curPic == 1)
				UpdateRects(pictureDivision, bmp1.bmWidth, bmp1.bmHeight, hWnd);
			else if(curPic == 2)
				UpdateRects(pictureDivision, bmp2.bmWidth, bmp2.bmHeight, hWnd);
			break;
		case ID_PICTUREDIVISION_4:
			pictureDivision = 4;
			if (curPic == 1)
				UpdateRects(pictureDivision, bmp1.bmWidth, bmp1.bmHeight, hWnd);
			else if (curPic == 2)
				UpdateRects(pictureDivision, bmp2.bmWidth, bmp2.bmHeight, hWnd);
			break;
		case ID_PICTUREDIVISION_5:
			pictureDivision = 5;
			if (curPic == 1)
				UpdateRects(pictureDivision, bmp1.bmWidth, bmp1.bmHeight, hWnd);
			else if (curPic == 2)
				UpdateRects(pictureDivision, bmp2.bmWidth, bmp2.bmHeight, hWnd);
			break;
		case ID_GAME_GAMESTART:
			IsS = true;
			ShuffleRects(divRects, pictureDivision);

			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case ID_GAME_SHOWPICTURE:
			break;
		case ID_GAME_GAMEEND:
			break;
		}
		break;
	case WM_ERASEBKGND:
		return 1;

	case WM_LBUTTONDOWN:
		if(!IsM){
			mx = LOWORD(lParam);
			my = HIWORD(lParam);

			srtmx = mx; srtmy = my;

			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;
	case WM_LBUTTONUP:
		if(!IsM){
			mx = LOWORD(lParam);
			my = HIWORD(lParam);

			endmx = mx; endmy = my;

			if (abs(endmx - srtmx) > abs(endmy - srtmy)) {
				g_dirY = 0;
				if ((endmx - srtmx) >= 0)
					g_dirX = 1;
				else
					g_dirX = -1;
			}
			else {
				g_dirX = 0;
				if ((endmy - srtmy) >= 0)
					g_dirY = 1;
				else
					g_dirY = -1;
			}
			MoveRects(hWnd, divRects, pictureDivision, g_dirX, g_dirY);
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;
	case WM_TIMER:
		if(wParam == 1){
			bool allArrived = true;

			for (auto& rect : divRects) {
				if (rect.isMoving) {
					allArrived = false;

					int moveX = g_dirX * SPEED;
					int moveY = g_dirY * SPEED;

					rect.curPos.left += g_dirX * SPEED;
					rect.curPos.right += g_dirX * SPEED;
					rect.curPos.top += g_dirY * SPEED;
					rect.curPos.bottom += g_dirY * SPEED;

					if ((g_dirX > 0 && rect.curPos.left >= rect.target.x) ||
						(g_dirX < 0 && rect.curPos.left <= rect.target.x)) {
						int w = rect.curPos.right - rect.curPos.left;
						rect.curPos.left = rect.target.x;
						rect.curPos.right = rect.target.x + w;
					}

					if ((g_dirY > 0 && rect.curPos.top >= rect.target.y) ||
						(g_dirY < 0 && rect.curPos.top <= rect.target.y)) {
						int h = rect.curPos.bottom - rect.curPos.top;
						rect.curPos.top = rect.target.y;
						rect.curPos.bottom = rect.target.y + h;
					}

					// 정지 판정
					if (rect.curPos.left == rect.target.x && rect.curPos.top == rect.target.y) {
						rect.isMoving = false;
						IsM = false;
					}
				}
			}

			InvalidateRect(hWnd, NULL, FALSE);
			if (allArrived) {
				IsM = false;
				KillTimer(hWnd, 1);
			}
		}
		break;
	case WM_DESTROY:
		DeleteObject(hBitmap1);
		DeleteObject(hBitmap2);
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}