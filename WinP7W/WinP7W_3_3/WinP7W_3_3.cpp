#include <windows.h>
#include <tchar.h>
#include <random>
#include <vector>
#include <math.h>

using namespace std;
random_device rd;
mt19937 g(rd());
uniform_int_distribution<> uid_movemode{ 1, 5 };
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

static int board[40][40] = { 0 };
static COLORREF colorBoard[40][40];

void spawn_item() {
	int ix, iy;
	do {
		ix = uid_drawBoard(g);
		iy = uid_drawBoard(g);
	} while (board[ix][iy] != 0);

	board[ix][iy] = 2;
	colorBoard[ix][iy] = RGB(uid_rgb(g), uid_rgb(g), uid_rgb(g));
}

typedef struct {
	COLORREF color;
	int r;
	int primaryDir;	// 0 우 / 1 하 / 2 좌 / 3 상
	int secondaryDir;
	bool shape; // true 원 false 삼각형
	bool jump;
	int x, y;
	int prevX, prevY;
	int pos;	// 원의 순서
	int movemode;	// 꼬리원 이동 로직
	bool bigger;
	int case3_count;
	int targetX, targetY; // 좌클릭시 지정됨 목적지 좌표
	bool isMovingToTarget; // 목적지로 이동 중인지 여부
} CIRCLE;

void init_setting(CIRCLE &c) {
	c.color = uid_rgb(g);
	c.r = 10;
	c.primaryDir = 0;
	c.secondaryDir = 1;
	c.shape = true;
	c.jump = false;
	c.x = uid_drawBoard(g);
	c.y = uid_drawBoard(g);
	int pos = 0;
	c.isMovingToTarget = false;
}

bool IsCollision(int x, int y) {
	if (x >= 0 && x < 40 && y >= 0 && y < 40) {
		if (board[x][y] != 1) {
			return false;
		}
	}
	return true;
}

void head_move(CIRCLE& head) {		// 머리 움직임
	head.prevX = head.x;
	head.prevY = head.y;

	if (head.isMovingToTarget) {
		// 목적지에 도달했는지 확인
		if (head.x == head.targetX && head.y == head.targetY) {
			head.isMovingToTarget = false;
		}
		else {
			// 목적지를 향해 X, Y축 중 하나씩 이동 (한 칸씩)
			if (head.x < head.targetX) head.x++;
			else if (head.x > head.targetX) head.x--;
			else if (head.y < head.targetY) head.y++;
			else if (head.y > head.targetY) head.y--;

			return;
		}
	}

	int px = head.x;
	int py = head.y;

	if (head.primaryDir == 0) px++;      // 우
	else if (head.primaryDir == 1) py++; // 하
	else if (head.primaryDir == 2) px--; // 좌
	else if (head.primaryDir == 3) py--; // 상

	if (!IsCollision(px, py)) {
		head.x = px;
		head.y = py;
	}
	else {
		int sx = head.x;
		int sy = head.y;

		if (head.secondaryDir == 0) sx++;
		else if (head.secondaryDir == 1) sy++;
		else if (head.secondaryDir == 2) sx--;
		else if (head.secondaryDir == 3) sy--;

		if (!IsCollision(sx, sy)) {
			head.x = sx;
			head.y = sy;
			head.primaryDir = (head.primaryDir + 2) % 4;
		}
		else {
			int osDir = (head.secondaryDir + 2) % 4;
			int ox = head.x;
			int oy = head.y;

			if (osDir == 0) ox++;
			else if (osDir == 1) oy++;
			else if (osDir == 2) ox--;
			else if (osDir == 3) oy--;

			if (!IsCollision(ox, oy)) {
				head.x = ox;
				head.y = oy;
				head.primaryDir = (head.primaryDir + 2) % 4;
				head.secondaryDir = osDir;
			}
		}
	}
}

//void follow_move(CIRCLE& cur, const CIRCLE& prev) {		// 따라가는 꼬리 움직임
//	cur.prevX = cur.x;
//	cur.prevY = cur.y;
//
//	cur.x = prev.prevX;
//	cur.y = prev.prevY;
//}

void follow_move(CIRCLE& cur, const CIRCLE& prev) {
	cur.x = prev.prevX;
	cur.y = prev.prevY;
}

// case 3 순환 이동 로직을 위한 변수
const int dx[] = { 1, 0, -1, 0 }; // 우 하 좌 상
const int dy[] = { 0, 1, 0, -1 };

void tail_move(CIRCLE& t) {		// 독립된 꼬리 움직임
	switch (t.movemode) {
	case 1:		// 좌우
		if (t.primaryDir == 0) {
			if (t.x >= 39 || board[t.x + 1][t.y] == 1) {
				t.primaryDir = 2;
				t.x--;
			}
			else
				t.x++;
		}
		else {
			if (t.x <= 0 || board[t.x - 1][t.y] == 1) {
				t.primaryDir = 0;
				t.x++;
			}
			else
				t.x--;
		}
		break;
	case 2:		// 상하
		if (t.primaryDir == 1) {
			if (t.y >= 39 || board[t.x][t.y + 1] == 1) {
				t.primaryDir = 3;
				t.y--;
			}
			else
				t.y++;
		}
		else {
			if (t.y <= 0 || board[t.x][t.y - 1] == 1) {
				t.primaryDir = 1;
				t.y++;
			}
			else
				t.y--;
		}
		break;
	case 3:		// 사각형 순환
	{
		if (t.case3_count >= 4) {
			t.case3_count = 0;
			t.primaryDir = (t.primaryDir + 1) % 4;
		}

		int nx = t.x + dx[t.primaryDir];
		int ny = t.y + dy[t.primaryDir];

		if (!IsCollision(nx, ny)) {
			t.x = nx;
			t.y = ny;
			t.case3_count++;
		}
		else {
			t.case3_count = 0;
			t.primaryDir = (t.primaryDir + 1) % 4;

			int tx = t.x + dx[t.primaryDir];
			int ty = t.y + dy[t.primaryDir];
			if (IsCollision(tx, ty)) {
				t.primaryDir = (t.primaryDir + 1) % 4;
			}
		}
		break;
	}
	case 5:
	{
		if (t.r >= 30) t.bigger = false;
		else if (t.r <= 1) t.bigger = true;

		if (t.bigger) t.r++;
		else t.r--;
		break;
	}
	}	
}

void item_to_circle(vector<CIRCLE>& c, COLORREF ccolor) {
	CIRCLE newTail;

	newTail.color = ccolor;
	newTail.r = 10;
	newTail.movemode = uid_movemode(g);
	newTail.bigger = true;
	newTail.case3_count = 0;

	do {
		newTail.x = uid_drawBoard(g);
		newTail.y = uid_drawBoard(g);
	} while (board[newTail.x][newTail.y] != 0);

	newTail.prevX = newTail.x;
	newTail.prevY = newTail.y;

	if (newTail.movemode == 1 || newTail.movemode == 3)
		newTail.primaryDir = 0;
	else if (newTail.movemode == 2)
		newTail.primaryDir = 1;

	c.push_back(newTail);
}

//void tailtail_collision(vector<CIRCLE>& t) {
//	for (int i = 0; i < (int)t.size(); ++i) {
//		if (t[i].movemode == 0) continue;
//
//		for (int j = 0; j < (int)t.size(); ++j) {
//			if (i == j) continue;
//
//			if (t[i].x == t[j].x && t[i].y == t[j].y) {
//				CIRCLE tempJ = t[j];
//				tempJ.movemode = 0;
//				tempJ.color = t[i].color;
//
//				t.erase(t.begin() + j);		// 독립원 순서 재배치
//
//				int insertPos = -1;
//				for (int k = 0; k < (int)t.size(); ++k) {
//					if (&t[k] == &t[i]) { // 리더의 현재 위치 확인
//						insertPos = k + 1;
//						// 리더 뒤에 이미 붙어있는 추종자(movemode==0)들을 건너뜀
//						while (insertPos < (int)t.size() && t[insertPos].movemode == 0) {
//							insertPos++;
//						}
//						break;
//					}
//				}
//
//				if (insertPos != -1) {
//					// 삽입될 원의 위치를 앞 원의 prev 위치로 일단 초기화
//					tempJ.x = t[insertPos - 1].prevX;
//					tempJ.y = t[insertPos - 1].prevY;
//					t.insert(t.begin() + insertPos, tempJ);
//				}
//				return;
//			}
//		}
//	}
//}
void tailtail_collision(vector<CIRCLE>& t) {
	for (int i = 0; i < (int)t.size(); ++i) {
		if (t[i].movemode == 0) continue;
		for (int j = 0; j < (int)t.size(); ++j) {
			if (i == j) continue;

			if (t[i].x == t[j].x && t[i].y == t[j].y) {
				CIRCLE tempJ = t[j];
				tempJ.movemode = 0;

				// 리더가 이동하기 전 위치로 즉시 이동시켜 겹침 방지
				tempJ.x = t[i].prevX;
				tempJ.y = t[i].prevY;
				tempJ.prevX = tempJ.x;
				tempJ.prevY = tempJ.y;

				t.erase(t.begin() + j);
				int insertPos = (j < i) ? i : i + 1;
				t.insert(t.begin() + insertPos, tempJ);
				return;
			}
		}
	}
}

void tail_eaten_head(vector<CIRCLE>& circles, vector<CIRCLE>& tail_circles) {
	for (auto it = tail_circles.begin(); it != tail_circles.end(); )
	{
		bool isHit = false;
		int hx = circles[0].x;
		int hy = circles[0].y;
		int tx = it->x;	// 독립원의 중심 X칸
		int ty = it->y;	// 독립원의 중심 Y칸

		if (it->r >= 1 && it->r <= 10) {		// 원이 한 칸보다 작을때
			if (hx == tx && hy == ty) isHit = true;
		}
		else if (it->r >= 11 && it->r <= 18) {	// 원의 반지름이 14.14 + 4.14보다 작을때(한 칸의 대각선)
			if ((hx == tx && hy == ty) ||           // 중심
				(hx == tx + 1 && hy == ty) ||       // 우
				(hx == tx - 1 && hy == ty) ||       // 좌
				(hx == tx && hy == ty + 1) ||       // 하
				(hx == tx && hy == ty - 1))         // 상
				isHit = true;
		}
		else if (it->r >= 19 && it->r <= 30) {	// 원의 반지름이 14.14 + 4.14보다 클 때
			if (hx >= tx - 1 && hx <= tx + 1 && hy >= ty - 1 && hy <= ty + 1)
				isHit = true;
		}

		if (isHit)
		{
			CIRCLE newSegment = *it;
			newSegment.color = circles[0].color;
			newSegment.movemode = 0;
			int lastR = circles.back().r;
			newSegment.r = (lastR > 1) ? lastR - 1 : 1;
			circles.push_back(newSegment);

			it = tail_circles.erase(it);
		}
		else
		{
			++it;
		}
	}
}

// void spawn_item()										아이템 위치 설정
// void init_setting(CIRCLE& c)								머리원 값 세팅
// bool IsCollision(int x, int y)							충돌 여부(이동로직에 사용)
// void head_move(CIRCLE& head)								머리원 이동
// void follow_move(CIRCLE& cur, const CIRCLE& prev)		꼬리원 머리따라 이동
// void tail_move(CIRCLE& t)								독립된 꼬리원 5가지 이동 로직
// void item_to_circle(vector<CIRCLE>& c, COLORREF ccolor)	습득한 아이템을 독립된 꼬리원으로 전환
// void tailtail_collision(vector<CIRCLE>& t)				독립된 꼬리원끼리 부딪혔을 경우 처리
// void tail_eaten_head(vector<CIRCLE>& circles, vector<CIRCLE>& tail_circles) 머리가 꼬리를 먹었을 경우 

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC;
	HPEN hPen, oldPen;
	HBRUSH hBrush, oldBrush;
	TCHAR str[50];
	static vector<CIRCLE> circles;
	static vector<CIRCLE> tail_circles;
	static RECT rt;
	static int speed = 100;		// 50 ~ 200, default = 100, +-10
	static int mx, my;
	static bool game_start = false;
	static bool move_left = false;
	static bool move_right = false;
	static bool move_up = false;
	static bool move_down = false;
	static int obstacle = 0;

	switch (uMsg) {
	case WM_CREATE:
		CIRCLE head;
		init_setting(head);
		circles.push_back(head);
		for (int i = 0; i < 20; i++) spawn_item();
		SetTimer(hWnd, 1, speed, NULL);
		break;
	case WM_KEYDOWN:
		if (wParam == 'S') {
			game_start = true;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == VK_UP) {
			if(circles[0].primaryDir != 1)
				circles[0].secondaryDir = circles[0].primaryDir;
			circles[0].primaryDir = 3;

			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == VK_RIGHT) {
			if (circles[0].primaryDir != 2)
				circles[0].secondaryDir = circles[0].primaryDir;
			circles[0].primaryDir = 0;
			
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == VK_DOWN) {
			if (circles[0].primaryDir != 3)
				circles[0].secondaryDir = circles[0].primaryDir;
			circles[0].primaryDir = 1;
			
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == VK_LEFT) {
			if (circles[0].primaryDir != 0)
				circles[0].secondaryDir = circles[0].primaryDir;
			circles[0].primaryDir = 2;

			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == VK_OEM_PLUS || wParam == VK_ADD) {
			if (speed > 10) {
				speed -= 10;
				KillTimer(hWnd, 1);
				SetTimer(hWnd, 1, speed, NULL);
			}

			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == VK_OEM_MINUS || wParam == VK_SUBTRACT) {
			if (speed < 200) {
				speed += 10;
				KillTimer(hWnd, 1);
				SetTimer(hWnd, 1, speed, NULL);
			}

			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'J') {

			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'T') {

			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'A') {

			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == VK_ESCAPE || wParam == 'Q') {
			PostQuitMessage(0);
		}
		break;

	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);
		{
			// 백버퍼 생성
			HDC memDC = CreateCompatibleDC(hDC);
			HBITMAP hBit = CreateCompatibleBitmap(hDC, LEN, HEI);
			HBITMAP oldBit = (HBITMAP)SelectObject(memDC, hBit);

			// 배경 지우기 (더블 버퍼링의 핵심: 하얀색으로 백버퍼를 채움)
			FillRect(memDC, &ps.rcPaint, (HBRUSH)GetStockObject(WHITE_BRUSH));

			//-----------------------------기본 판 그리기
			hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
			oldPen = (HPEN)SelectObject(memDC, hPen);
			for (int i = 0; i < 40; ++i) {
				for (int j = 0; j < 40; ++j) {
					Rectangle(memDC, 20 + 20 * j, 20 + 20 * i, 40 + 20 * j, 40 + 20 * i);
				}
			}
			SelectObject(memDC, oldPen);
			DeleteObject(hPen);
			//-------------------------------장애물 그리기
			for (int i = 0; i < 40; ++i) {
				for (int j = 0; j < 40; ++j) {
					if (board[j][i] == 1) {
						hBrush = CreateSolidBrush(RGB(255, 0, 0));
						SelectObject(memDC, hBrush);
						Rectangle(memDC, 20 + 20 * j, 20 + 20 * i, 40 + 20 * j, 40 + 20 * i);
						DeleteObject(hBrush);
					}
				}
			}
			//--------------------------------아이템 그리기
			for (int i = 0; i < 40; ++i) {
				for (int j = 0; j < 40; ++j) {
					if (board[j][i] == 2) {
						hBrush = CreateSolidBrush(colorBoard[j][i]);
						SelectObject(memDC, hBrush);
						Rectangle(memDC, 25 + 20 * j, 25 + 20 * i, 35 + 20 * j, 35 + 20 * i);
						DeleteObject(hBrush);
					}
				}
			}
			//--------------------------------독립원 그리기
			for (int i = (int)tail_circles.size() - 1; i >= 0; --i) {
				hBrush = CreateSolidBrush(tail_circles[i].color);
				oldBrush = (HBRUSH)SelectObject(memDC, hBrush);

				int centerX = 20 + 20 * tail_circles[i].x + 10;
				int centerY = 20 + 20 * tail_circles[i].y + 10;

				Ellipse(memDC, centerX - tail_circles[i].r, centerY - tail_circles[i].r,
					centerX + tail_circles[i].r, centerY + tail_circles[i].r);

				SelectObject(memDC, oldBrush);
				DeleteObject(hBrush);
			}
			//------------------------주인공과 꼬리원 그리기
			for (int i = (int)circles.size() - 1; i >= 0; --i) {
				hBrush = CreateSolidBrush(circles[i].color);
				SelectObject(memDC, hBrush);

				int centerX = 20 + 20 * circles[i].x + 10;
				int centerY = 20 + 20 * circles[i].y + 10;

				if (circles[0].shape)
					Ellipse(memDC, centerX - circles[i].r, centerY - circles[i].r, centerX + circles[i].r, centerY + circles[i].r);
				else {
					POINT pt[3];
					pt[0].x = centerX;          
					pt[0].y = centerY - circles[i].r; // 위
					pt[1].x = centerX - circles[i].r;
					pt[1].y = centerY + circles[i].r; // 왼쪽 아래
					pt[2].x = centerX + circles[i].r;
					pt[2].y = centerY + circles[i].r; // 오른쪽 아래
					Polygon(memDC, pt, 3);
				}

				DeleteObject(hBrush);
			}
			
			SetBkMode(memDC, TRANSPARENT);
			SetTextColor(memDC, RGB(0, 0, 0));
			SetTextAlign(memDC, TA_CENTER | TA_TOP);
			_stprintf_s(str, L"Speed : %d", speed);
			TextOut(memDC, 50, 850, str, _tcslen(str));

			// 완성된 백버퍼를 실제 화면으로 한 번에 복사
			BitBlt(hDC, 0, 0, LEN, HEI, memDC, 0, 0, SRCCOPY);

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

		int boardX = (mx - 20) / 20;
		int boardY = (my - 20) / 20;

		if (boardX == circles[0].x && boardY == circles[0].y) {
			circles.erase(circles.begin() + 1, circles.end());
			circles[0].shape = !circles[0].shape;
		}
		else if (boardX >= 0 && boardX < 40 && boardY >= 0 && boardY < 40) {
			circles[0].targetX = boardX;
			circles[0].targetY = boardY;
			circles[0].isMovingToTarget = true;
		}


		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}
	case WM_RBUTTONDOWN:
		mx = LOWORD(lParam);
		my = HIWORD(lParam);

		{
			int boardX = (mx - 20) / 20;
			int boardY = (my - 20) / 20;

			if ((boardX >= 0 && boardX < 40 && boardY >= 0 && boardY < 40) && (board[boardX][boardY] == 0 || board[boardX][boardY] == 1)) {
				if (board[boardX][boardY] == 0) {
					board[boardX][boardY] = 1;
					obstacle++;
				}
				else if (board[boardX][boardY] == 1) {
					board[boardX][boardY] = 0;
					obstacle--;
				}
			}
		}

		InvalidateRect(hWnd, NULL, FALSE);
		break;
	//case WM_TIMER:
	//	switch (wParam) {
	//	case 1: // 메인 루프 타이머
	//		if (game_start) {
	//			// 1. 주인공과 꼬리들 이동
	//			head_move(circles[0]);
	//			for (int i = 1; i < (int)circles.size(); i++) {
	//				follow_move(circles[i], circles[i - 1]);
	//			}

	//			// 2. 독립 꼬리원끼리 충돌 체크
	//			tailtail_collision(tail_circles);

	//			// 3. 주인공과 독립원 충돌
	//			tail_eaten_head(circles, tail_circles);

	//			// 4. 독립 꼬리원 이동
	//			for (int i = 0; i < (int)tail_circles.size(); i++) {
	//				if (tail_circles[i].movemode == 0) {
	//					if (i > 0) {
	//						follow_move(tail_circles[i], tail_circles[i - 1]);
	//					}
	//				}
	//				else {
	//					tail_move(tail_circles[i]);
	//				}
	//			}

	//			// 5. 이중체크
	//			tail_eaten_head(circles, tail_circles);

	//			if (board[circles[0].x][circles[0].y] == 2) {
	//				COLORREF itemColor = colorBoard[circles[0].x][circles[0].y];
	//				for(int i = 0; i < circles.size(); ++i)
	//					circles[i].color = itemColor;

	//				board[circles[0].x][circles[0].y] = 0; // 아이템 제거
	//				item_to_circle(tail_circles, itemColor); // 새로운 독립 원 생성
	//			}
	//		}
	//		break;

	//	case 2:
	//		break;
	//	}
	//	InvalidateRect(hWnd, NULL, TRUE);
	//	UpdateWindow(hWnd);
	//	break;
	case WM_TIMER:
		if (game_start) {
			// [1] 모든 원(주인공군, 독립군)의 현재 위치를 prev에 저장
			for (int i = 0; i < (int)circles.size(); i++) {
				circles[i].prevX = circles[i].x;
				circles[i].prevY = circles[i].y;
			}
			for (int i = 0; i < (int)tail_circles.size(); i++) {
				tail_circles[i].prevX = tail_circles[i].x;
				tail_circles[i].prevY = tail_circles[i].y;
			}

			// [2] 실제 이동 수행
			// 주인공 머리 이동
			head_move(circles[0]);
			// 주인공 꼬리들
			for (int i = 1; i < (int)circles.size(); i++) {
				follow_move(circles[i], circles[i - 1]);
			}

			// 독립 꼬리원들 이동
			for (int i = 0; i < (int)tail_circles.size(); i++) {
				if (tail_circles[i].movemode == 0) {
					if (i > 0) follow_move(tail_circles[i], tail_circles[i - 1]);
				}
				else {
					tail_move(tail_circles[i]);
				}
			}

			// [3] 이동이 끝난 후 충돌 판정
			tailtail_collision(tail_circles);
			tail_eaten_head(circles, tail_circles);

			// [4] 아이템 체크
			if (board[circles[0].x][circles[0].y] == 2) {
				COLORREF itemColor = colorBoard[circles[0].x][circles[0].y];
				for (int i = 0; i < (int)circles.size(); ++i) circles[i].color = itemColor;
				board[circles[0].x][circles[0].y] = 0;
				item_to_circle(tail_circles, itemColor);
			}
		}
		InvalidateRect(hWnd, NULL, FALSE);
		UpdateWindow(hWnd);
		break;
	case WM_DESTROY:
		for (int i = 0; i < 4; ++i)
			KillTimer(hWnd, i);
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}