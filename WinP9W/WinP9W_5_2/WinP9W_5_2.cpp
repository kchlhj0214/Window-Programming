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

struct RECTS { int boardPos; RECT pic; POINT target; bool isMoving;
};

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"My Window Class";
LPCTSTR lpszWindowName = L"windows program 2";

bool IsA = false;
int bmW, bmH;
int pictureDivision = 1;
int selected = 0;
vector<RECTS> divRects;
bool IsR = false;
int board[BORAR_SIZE][BORAR_SIZE] = {0};
vector<POINT> pos;
int emptyBoard;

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
	WndClass.lpszMenuName = MAKEINTRESOURCE("IDR_MENU1");
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

void UpdateRects(int mode, int w, int h) {
	pos.clear();
	divRects.clear();
	int rows = mode;
	int cols = mode;

	int cellW = w / cols;
	int cellH = h / rows;

	int p = 0;

	for (int r = 0; r < rows; r++) {
		for (int c = 0; c < cols; c++) {
			RECTS rct;
			POINT pt;
			if ((r != rows - 1) || (c != cols - 1))
				rct = { p++, { c * cellW, r * cellH, (c + 1) * cellW, (r + 1) * cellH }};
			pt = { c * cellW, r * cellH };
			pos.push_back(pt);
			divRects.push_back(rct);
		}
	}
}

void ShuffleRects(vector<RECTS>& r, int mode) {
	vector<int> v;
	for (int i = 0; i < mode * mode; ++i)
		v.push_back(i);

	shuffle(v.begin(), v.end(), g);
	emptyBoard = v.back();
	v.pop_back();

	for (int i = 0; i < r.size(); ++i) {
		if (i < v.size()) {
			r[i].boardPos = v[i];
		}
	}

	for (int i = 0; i < BORAR_SIZE; i++)
		for (int j = 0; j < BORAR_SIZE; j++) 
			board[i][j] = 0;

	// 1차원으로 표현된 빈 보드를 2차원으로 전환
	int row = emptyBoard / mode;
	int col = emptyBoard % mode;

	board[row][col] = 1;
}

void MoveRects(HWND hWnd, vector<RECTS>& r, int mode, int dirX, int dirY) {
	// 1. 현재 빈 칸의 2차원 좌표 계산
	int eRow = emptyBoard / mode;
	int eCol = emptyBoard % mode;

	// 2. 이동 대상 조각들 찾기
	// 예: 위로 드래그(dirY=-1) -> 빈 칸 아래(row > eRow)에 있는 같은 열(col == eCol) 조각들
	vector<int> targets;

	int currRow = eRow - dirY; // 빈 칸 바로 다음 칸부터 조사
	int currCol = eCol - dirX;

	// 보드 범위 안에서 같은 라인에 있는 조각들을 전부 수집
	while (currRow >= 0 && currRow < mode && currCol >= 0 && currCol < mode) {
		int targetIdx = -1;
		int targetBoardPos = currRow * mode + currCol;

		// 현재 보드 위치(targetBoardPos)를 차지하고 있는 조각 찾기
		for (int i = 0; i < r.size(); ++i) {
			if (r[i].boardPos == targetBoardPos) {
				targetIdx = i;
				break;
			}
		}

		if (targetIdx != -1) {
			targets.push_back(targetIdx);
		}

		currRow -= dirY; // 같은 방향으로 한 칸 더 이동하며 조사
		currCol -= dirX;
	}

	// 3. 수집된 조각들에게 이동 명령 하달
	if (targets.empty()) return;

	int cellW = bmW / mode;
	int cellH = bmH / mode;

	for (int idx : targets) {
		r[idx].isMoving = true;
		// 목표 좌표는 현재 위치에서 드래그 방향으로 한 칸 이동한 곳
		r[idx].target.x = pos[r[idx].boardPos + (dirY * mode + dirX)].x;
		r[idx].target.y = pos[r[idx].boardPos + (dirY * mode + dirX)].y;

		// 논리적 위치(boardPos) 업데이트
		r[idx].boardPos += (dirY * mode + dirX);
	}

	// 4. 빈 칸의 논리적 위치 업데이트 (가장 멀리 있던 조각의 원래 위치가 새로운 빈 칸이 됨)
	// 하지만 이 방식은 연속 이동이므로, 빈 칸은 한 칸씩만 이동하는 게 아니라 
	// 기차가 이동한 뒤 남은 끝자리가 됩니다.
	emptyBoard = eRow * mode + eCol + (targets.size() * ((-dirY) * mode + (-dirX)));

	// 5. 애니메이션 시작을 위한 타이머 실행
	SetTimer(hWnd, 1, 10, NULL);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC;
	HPEN hPen, oldPen;
	HBRUSH hBrush, oldBrush;
	static RECT rt;
	static int mx, my;
	static BITMAP bmp;
	static HBITMAP hBitmap1, hBitmap2;

	switch (uMsg) {
	case WM_CREATE:
		hBitmap1 = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP1));
		hBitmap2 = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP2));
		GetObject(hBitmap1, sizeof(BITMAP), &bmp1);
		GetObject(hBitmap2, sizeof(BITMAP), &bmp2);
		bmW = bmp.bmWidth;
		bmH = bmp.bmHeight;
		UpdateRects(1, bmW, bmH);
		break;
	case WM_KEYDOWN:
		GetClientRect(hWnd, &rt);
		if (wParam == 'S') {
			if (IsA) {
				bmW = bmp.bmWidth;
				bmH = bmp.bmHeight;
				IsA = !IsA;
			}
			else {
				bmW = rt.right;
				bmH = rt.bottom;
				IsA = !IsA;
			}
			UpdateRects(pictureDivision, bmW, bmH);
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'F') {
			if (bmW < rt.right && bmH < rt.bottom)
				bmW += PLUS; bmH += PLUS;
			if (bmW > rt.right) bmW = rt.right;
			if (bmH > rt.bottom) bmH = rt.bottom;
			UpdateRects(pictureDivision, bmW, bmH);
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'V') {
			if ((bmW > rt.right / 4) && (bmH > rt.bottom / 4))
				bmW += MINUS; bmH += MINUS;
			if (bmW < rt.right / 4) bmW = rt.right / 4;
			if (bmH < rt.bottom / 4) bmH = rt.bottom / 4;
			UpdateRects(pictureDivision, bmW, bmH);
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'H') {
			pictureDivision = wParam - '0';
			selected = 0;
			IsR = false;
			if (wParam == '1')
				UpdateRects(1, bmp.bmWidth, bmp.bmHeight);
			else {
				UpdateRects(wParam - '0', rt.right, rt.bottom);
				bmW = rt.right;
				bmH = rt.bottom;
			}
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'Q' || wParam == VK_ESCAPE) {
			DeleteObject(hBitmap);
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
			HDC hMemDC = CreateCompatibleDC(hDC);
			SelectObject(hMemDC, hBitmap);

			for (int i = 0; i < divRects.size(); i++) {
				int curW = divRects[i].right - divRects[i].left;
				int curH = divRects[i].bottom - divRects[i].top;

				if ((selected == i + 1) && IsR)
					StretchBlt(memDC, divRects[i].left, divRects[i].top, curW, curH, 
						hMemDC, divRects[i].left, divRects[i].top, divRects[i].right, divRects[i].bottom, NOTSRCCOPY);
				else
					StretchBlt(memDC, divRects[i].left, divRects[i].top, curW, curH, hMemDC, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);

				if (selected == i + 1) {
					HBRUSH hOldBrush = (HBRUSH)SelectObject(memDC, GetStockObject(NULL_BRUSH));
					HPEN hRedPen = CreatePen(PS_SOLID, 5, RGB(255, 0, 0));
					HPEN hOldPen = (HPEN)SelectObject(memDC, hRedPen);

					Rectangle(memDC, divRects[i].left, divRects[i].top, divRects[i].right, divRects[i].bottom);

					SelectObject(memDC, hOldBrush);
					SelectObject(memDC, hOldPen);
					DeleteObject(hRedPen);
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

	case WM_ERASEBKGND:
		return 1;

	case WM_LBUTTONDOWN:
	{
		mx = LOWORD(lParam);
		my = HIWORD(lParam);
		POINT pt = { mx, my };
		IsR = false;

		selected = 0;
		for (int i = 0; i < divRects.size(); i++) {
			if (PtInRect(&divRects[i], pt)) {
				selected = i + 1;
				break;
			}
		}
		InvalidateRect(hWnd, NULL, FALSE);
	}
	break;
	case WM_TIMER:
		if(wParam == 1){
			bool allArrived = true;
			for (auto& rect : divRects) {
				if (rect.isMoving) {
					// 현재 좌표와 목표 좌표의 차이 계산
					int dx = rect.target.x - rect.pic.left;
					int dy = rect.target.y - rect.pic.top;

					if (abs(dx) <= SPEED && abs(dy) <= SPEED) {
						// 도착 처리
						int w = rect.pic.right - rect.pic.left;
						int h = rect.pic.bottom - rect.pic.top;
						rect.pic.left = rect.target.x;
						rect.pic.top = rect.target.y;
						rect.pic.right = rect.pic.left + w;
						rect.pic.bottom = rect.pic.top + h;
						rect.isMoving = false;
					}
					else {
						// 부드럽게 이동
						OffsetRect(&rect.pic, (dx > 0 ? 1 : -1) * (dx == 0 ? 0 : SPEED),
							(dy > 0 ? 1 : -1) * (dy == 0 ? 0 : SPEED));
						allArrived = false;
					}
				}
			}

			InvalidateRect(hWnd, NULL, FALSE);
			if (allArrived) KillTimer(hWnd, 1);
		}
		break;
	case WM_DESTROY:
		DeleteObject(hBitmap);
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}