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
bool IsM = false;
bool IsHint = false;
bool IsV = false;
bool IsH = false;
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
bool IsHavePic = false;
int tempEmptyBoard = -1;	// 마우스 우클릭으로 생긴 임시 빈 공간
RECTS tempRect;
bool IsVH = false;
int dragIdx = -1; // 드래그 중인 조각의 인덱스
POINT dragOffset; // 클릭한 시점의 마우스 좌표와 조각 좌상단의 차이

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

	vector<int> blanks;
	blanks.push_back(emptyBoard);
	if (tempEmptyBoard != -1) blanks.push_back(tempEmptyBoard);

	bool movedAny = false;

	for (int blankPos : blanks) {
		int eRow = blankPos / mode;
		int eCol = blankPos % mode;

		int targetRow = eRow - dirY;
		int targetCol = eCol - dirX;

		if (targetRow < 0 || targetRow >= mode || targetCol < 0 || targetCol >= mode) continue;

		int targetBoardPos = targetRow * mode + targetCol;

		// 해당 위치에 있는 조각 찾기
		for (int i = 0; i < r.size(); ++i) {
			// 이미 다른 빈 공간에 의해 이동 중인 조각은 제외
			if (r[i].boardPos == targetBoardPos && !r[i].isMoving) {
				r[i].isMoving = true;
				r[i].target.x = pos[blankPos].x;
				r[i].target.y = pos[blankPos].y;

				// 데이터 스왑
				r[i].boardPos = blankPos;
				if (blankPos == emptyBoard) emptyBoard = targetBoardPos;
				else tempEmptyBoard = targetBoardPos;

				movedAny = true;
				break;
			}
		}
	}

	if (movedAny) {
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

void UpdateRectsVH(int mode, bool isVertical, int w, int h, HWND hWnd) {
	RECT clientRt;
	GetClientRect(hWnd, &clientRt);
	pos.clear();
	divRects.clear();

	int rows;
	int cols;
	if (!isVertical) {
		rows = mode;
		cols = 1;
	}
	else {
		rows = 1;
		cols = mode;
	}

	cellW = clientRt.right / cols;
	cellH = clientRt.bottom / rows;
	int imgCellW = w / cols;
	int imgCellH = h / rows;

	for (int r = 0; r < rows; r++) {
		for (int c = 0; c < cols; c++) {
			pos.push_back({ c * cellW, r * cellH });
			RECTS rct;
			rct.boardPos = (int)divRects.size();
			rct.curPos = { c * cellW, r * cellH, (c + 1) * cellW, (r + 1) * cellH };
			rct.pic = { c * imgCellW, r * imgCellH, (c + 1) * imgCellW, (r + 1) * imgCellH };
			rct.isMoving = false;
			divRects.push_back(rct);
		}
	}
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
		UpdateRects(1, bmp1.bmWidth, bmp1.bmHeight, hWnd);
		break;
	case WM_KEYDOWN:
		GetClientRect(hWnd, &rt);
		if (wParam == 'S') {
			IsVH = false;
			IsV = false;
			IsH = false;
			if (pictureDivision <= 1) {
				MessageBox(hWnd, L"등분을 먼저 선택해주세요!", L"알림", MB_OK);
				break;
			}

			if (curPic == 1) UpdateRects(pictureDivision, bmp1.bmWidth, bmp1.bmHeight, hWnd);
			else if (curPic == 2) UpdateRects(pictureDivision, bmp2.bmWidth, bmp2.bmHeight, hWnd);

			IsS = true;
			IsM = false;
			IsHint = false;
			ShuffleRects(divRects, pictureDivision);

			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'F') {
			IsHint = !IsHint;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		if (wParam == 'V' || wParam == 'H') {
			IsVH = true;
			IsS = false;
			IsV = (wParam == 'V');
			IsH = (wParam == 'H');
			if (curPic == 1) 
				UpdateRectsVH(pictureDivision, IsV, bmp1.bmWidth, bmp1.bmHeight, hWnd);
			else 
				UpdateRectsVH(pictureDivision, IsV, bmp2.bmWidth, bmp2.bmHeight, hWnd);
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
			HDC hImgDC = CreateCompatibleDC(hDC);
			if (curPic == 1) SelectObject(hImgDC, hBitmap1);
			else if(curPic == 2) SelectObject(hImgDC, hBitmap2);

			if (IsVH) {
				HPEN hBlackPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
				HPEN hOldPen = (HPEN)SelectObject(memDC, hBlackPen);
				SelectObject(memDC, GetStockObject(NULL_BRUSH));

				for (int i = 0; i < (int)divRects.size(); ++i) {
					if (i == dragIdx) continue;

					StretchBlt(memDC, divRects[i].curPos.left, divRects[i].curPos.top,
						divRects[i].curPos.right - divRects[i].curPos.left,
						divRects[i].curPos.bottom - divRects[i].curPos.top,
						hImgDC, divRects[i].pic.left, divRects[i].pic.top,
						(divRects[i].pic.right - divRects[i].pic.left),
						(divRects[i].pic.bottom - divRects[i].pic.top), SRCCOPY);

					Rectangle(memDC, divRects[i].curPos.left, divRects[i].curPos.top,
						divRects[i].curPos.right, divRects[i].curPos.bottom);
				}

				if (dragIdx != -1) {
					StretchBlt(memDC, divRects[dragIdx].curPos.left, divRects[dragIdx].curPos.top,
						cellW, cellH, hImgDC, divRects[dragIdx].pic.left, divRects[dragIdx].pic.top,
						(divRects[dragIdx].pic.right - divRects[dragIdx].pic.left),
						(divRects[dragIdx].pic.bottom - divRects[dragIdx].pic.top), SRCCOPY);

					HPEN hDragPen = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
					SelectObject(memDC, hDragPen);
					Rectangle(memDC, divRects[dragIdx].curPos.left, divRects[dragIdx].curPos.top,
						divRects[dragIdx].curPos.right, divRects[dragIdx].curPos.bottom);
					DeleteObject(hDragPen);
				}
				SelectObject(memDC, hOldPen);
				DeleteObject(hBlackPen);
			}
			else {
				// 일반 모드 힌트 출력
				if (IsHint) {
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
				}
				else {
					for (int i = 0; i < (int)divRects.size(); ++i) {
						StretchBlt(memDC, divRects[i].curPos.left, divRects[i].curPos.top, cellW, cellH,
							hImgDC, divRects[i].pic.left, divRects[i].pic.top,
							(divRects[i].pic.right - divRects[i].pic.left),
							(divRects[i].pic.bottom - divRects[i].pic.top), SRCCOPY);
					}
				}
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
			IsS = false;
			IsM = false;
			IsHint = false;
			UpdateRects(1, bmp2.bmWidth, bmp2.bmHeight, hWnd);
			pictureDivision = 1;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case ID_PICTUREDIVISION_3:
			IsVH = false;
			IsV = false;
			IsH = false;
			pictureDivision = 3;
			if(curPic == 1)
				UpdateRects(pictureDivision, bmp1.bmWidth, bmp1.bmHeight, hWnd);
			else if(curPic == 2)
				UpdateRects(pictureDivision, bmp2.bmWidth, bmp2.bmHeight, hWnd);
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case ID_PICTUREDIVISION_4:
			IsVH = false;
			IsV = false;
			IsH = false;
			pictureDivision = 4;
			if (curPic == 1)
				UpdateRects(pictureDivision, bmp1.bmWidth, bmp1.bmHeight, hWnd);
			else if (curPic == 2)
				UpdateRects(pictureDivision, bmp2.bmWidth, bmp2.bmHeight, hWnd);
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case ID_PICTUREDIVISION_5:
			pictureDivision = 5;
			IsVH = false;
			IsV = false;
			IsH = false;
			if (curPic == 1)
				UpdateRects(pictureDivision, bmp1.bmWidth, bmp1.bmHeight, hWnd);
			else if (curPic == 2)
				UpdateRects(pictureDivision, bmp2.bmWidth, bmp2.bmHeight, hWnd);
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case ID_GAME_GAMESTART:
			IsVH = false;
			IsV = false;
			IsH = false;
			if (pictureDivision <= 1) {
				MessageBox(hWnd, L"등분을 먼저 선택해주세요!", L"알림", MB_OK);
				break;
			}

			if (curPic == 1) UpdateRects(pictureDivision, bmp1.bmWidth, bmp1.bmHeight, hWnd);
			else if(curPic == 2) UpdateRects(pictureDivision, bmp2.bmWidth, bmp2.bmHeight, hWnd);

			IsS = true;
			IsM = false;
			IsHint = false;
			ShuffleRects(divRects, pictureDivision);

			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case ID_GAME_SHOWPICTURE:
			IsHint = !IsHint;

			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case ID_GAME_GAMEEND:
			IsS = false;
			IsM = true;
			break;
		}
		break;
	case WM_ERASEBKGND:
		return 1;

	case WM_LBUTTONDOWN:
		mx = LOWORD(lParam);
		my = HIWORD(lParam);
		if (IsVH) {
			POINT pt = { mx, my };
			for (int i = 0; i < (int)divRects.size(); ++i) {
				if (PtInRect(&divRects[i].curPos, pt)) {
					dragIdx = i;
					dragOffset.x = mx - divRects[i].curPos.left;
					dragOffset.y = my - divRects[i].curPos.top;
					break;
				}
			}
		}
		else {
			if (!IsM) {
				srtmx = mx;
				srtmy = my;
			}
		}
		break;
	case WM_MOUSEMOVE:
		mx = LOWORD(lParam);
		my = HIWORD(lParam);
		if (IsVH) {
			if (dragIdx != -1) {
				int w = divRects[dragIdx].curPos.right - divRects[dragIdx].curPos.left;
				int h = divRects[dragIdx].curPos.bottom - divRects[dragIdx].curPos.top;
				divRects[dragIdx].curPos.left = mx - dragOffset.x;
				divRects[dragIdx].curPos.top = my - dragOffset.y;
				divRects[dragIdx].curPos.right = divRects[dragIdx].curPos.left + w;
				divRects[dragIdx].curPos.bottom = divRects[dragIdx].curPos.top + h;
				InvalidateRect(hWnd, NULL, FALSE);
			}
		}
		break;
	case WM_LBUTTONUP:
		mx = LOWORD(lParam);
		my = HIWORD(lParam);
		if (IsVH) {
			if (dragIdx != -1) {
				int minDist = 999999;
				int targetSlot = -1;
				for (int i = 0; i < (int)pos.size(); ++i) {
					int centerX = pos[i].x + (cellW / 2);
					int centerY = pos[i].y + (cellH / 2);
					int d = (int)(pow(mx - centerX, 2) + pow(my - centerY, 2));
					if (d < minDist) {
						minDist = d;
						targetSlot = i;
					}
				}

				int targetIdx = -1;
				for (int i = 0; i < (int)divRects.size(); ++i) {
					if (i != dragIdx) {
						if (divRects[i].boardPos == targetSlot) {
							targetIdx = i;
							break;
						}
					}
				}

				if (targetIdx != -1) {
					int tempPos = divRects[dragIdx].boardPos;
					divRects[dragIdx].boardPos = divRects[targetIdx].boardPos;
					divRects[targetIdx].boardPos = tempPos;

					divRects[targetIdx].curPos.left = pos[divRects[targetIdx].boardPos].x;
					divRects[targetIdx].curPos.top = pos[divRects[targetIdx].boardPos].y;
					divRects[targetIdx].curPos.right = divRects[targetIdx].curPos.left + cellW;
					divRects[targetIdx].curPos.bottom = divRects[targetIdx].curPos.top + cellH;
				}

				divRects[dragIdx].curPos.left = pos[divRects[dragIdx].boardPos].x;
				divRects[dragIdx].curPos.top = pos[divRects[dragIdx].boardPos].y;
				divRects[dragIdx].curPos.right = divRects[dragIdx].curPos.left + cellW;
				divRects[dragIdx].curPos.bottom = divRects[dragIdx].curPos.top + cellH;

				dragIdx = -1;
				InvalidateRect(hWnd, NULL, FALSE);
			}
		}
		else {
			if (!IsM) {
				int dx = abs(mx - srtmx);
				int dy = abs(my - srtmy);
				if (dx > 20 || dy > 20) {
					// [수정] 방향 결정 (if-else 사용)
					if (dx > dy) {
						g_dirY = 0;
						if (mx - srtmx > 0) g_dirX = 1;
						else g_dirX = -1;
					}
					else {
						g_dirX = 0;
						if (my - srtmy > 0) g_dirY = 1;
						else g_dirY = -1;
					}
					MoveRects(hWnd, divRects, pictureDivision, g_dirX, g_dirY);
				}
			}
		}
		break;
	case WM_LBUTTONDBLCLK:
	{
		if (IsM) break;
		if (IsVH) break;

		mx = LOWORD(lParam);
		my = HIWORD(lParam);
		POINT pt = { mx, my };

		for (auto& rect : divRects) {
			int clickedBoard = rect.boardPos;
			if (PtInRect(&rect.curPos, pt)) {
				rect.boardPos = emptyBoard;
				rect.curPos.left = pos[emptyBoard].x;
				rect.curPos.top = pos[emptyBoard].y;
				rect.curPos.right = pos[emptyBoard].x + cellW;
				rect.curPos.bottom = pos[emptyBoard].y + cellH;
				emptyBoard = clickedBoard;
				break;
			}
		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}
	case WM_RBUTTONDOWN:
	{
		if (IsVH) break;
		if (pictureDivision <= 1) break;

		mx = LOWORD(lParam);
		my = HIWORD(lParam);
		POINT pt = { mx, my };

		if (!IsHavePic) {
			int targetBoardPos = -1;
			for (int i = 0; i < pos.size(); ++i) {
				RECT cellRect = { pos[i].x, pos[i].y, pos[i].x + cellW, pos[i].y + cellH };
				if (PtInRect(&cellRect, pt)) {
					targetBoardPos = i;
					break;
				}
			}
			if (targetBoardPos == emptyBoard || targetBoardPos == tempEmptyBoard) {
				MessageBox(hWnd, L"저장된 사진이 없습니다!", L"에러", MB_OK);
			}
			// 사진 조각 들어내기
			for (auto it = divRects.begin(); it != divRects.end(); ++it) {
				if (PtInRect(&it->curPos, pt)) {
					tempRect = *it;
					tempEmptyBoard = it->boardPos;
					divRects.erase(it);
					IsHavePic = true;
					InvalidateRect(hWnd, NULL, FALSE);
					return 0;
				}
			}
		}
		else {
			// 들고 있는 조각 내려놓기
			int targetBoardPos = -1;
			for (int i = 0; i < pos.size(); ++i) {
				RECT cellRect = { pos[i].x, pos[i].y, pos[i].x + cellW, pos[i].y + cellH };
				if (PtInRect(&cellRect, pt)) {
					targetBoardPos = i;
					break;
				}
			}

			if (targetBoardPos == emptyBoard || targetBoardPos == tempEmptyBoard) {
				tempRect.boardPos = targetBoardPos;
				tempRect.curPos = { pos[targetBoardPos].x, pos[targetBoardPos].y,
									pos[targetBoardPos].x + cellW, pos[targetBoardPos].y + cellH };

				divRects.push_back(tempRect);

				if (targetBoardPos == emptyBoard) {	// 반대 경우엔 그냥 tempEmptyBoard만 초기화
					emptyBoard = tempEmptyBoard;
				}

				tempEmptyBoard = -1;
				IsHavePic = false;
			}
			else {
				MessageBox(hWnd, L"빈 공간에만 조각을 놓을 수 있습니다!", L"에러", MB_OK);
			}
		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}
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