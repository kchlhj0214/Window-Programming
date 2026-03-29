#include <windows.h>
#include <tchar.h>
#include <random>
#include <vector>

using namespace std;
random_device rd;
mt19937 g(rd());
uniform_int_distribution<> uid{ 0, 600 };		// 800 600 기준 12단까지
random_device rd2;
mt19937 g2(rd2());
uniform_int_distribution<> uid2{ 0, 255 };
#define LEN 800
#define HEI 600

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
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
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
	static TCHAR str[10][31] = { 0 };
	static TCHAR tempstr[10][31] = { 0 };
	static int count = 0;
	static int tempcount = 0;
	static SIZE size;
	static int line = 0;
	static int templine = 0;
	static int repeat = 0;
	static int saveLen[10] = { 0 };
	static int insertCheck = 0;


	switch (uMsg) {
	case WM_CREATE:
		CreateCaret(hWnd, NULL, 2, 15);
		ShowCaret(hWnd);
		break;
	case WM_KEYDOWN:
		if (wParam == VK_LEFT && str[line][count - 1] != ' ' && str[line][count - 1] != '\0') {
			if (count > 0)
				--count;
		}
		else if (wParam == VK_UP) {
			/*if (line > 0 && count != 0) {
				if (count <= lstrlen(str[line - 1]) && str[line - 1][count - 1] != ' ' && str[line - 1][count - 1] != '\0')
					--line;
			}
			else if (line > 0 && count == 0) {
				if (count <= lstrlen(str[line - 1]) && str[line - 1][count] != ' ' && str[line - 1][count] != '\0')
					--line;
			}*/
			if (line > 0 && str[line - 1][count] != '\0' && str[line - 1][count] != ' ')
				--line;
			else if (line > 0 && count > 0 && str[line - 1][count - 1] != '\0' && str[line - 1][count - 1] != ' ')
				--line;
		}
		else if (wParam == VK_RIGHT) {
			if (count < lstrlen(str[line]) && str[line][count] != ' ' && str[line][count] != '\0')
				++count;
		}
		else if (wParam == VK_DOWN) {
			/*if (line < 9) {
				int downLen = lstrlen(str[line + 1]);
				if (count < lstrlen(str[line + 1]) && str[line + 1][count] != ' ' && str[line + 1][count] != '\0')
					++line;
				else if (downLen > 0 || lstrlen(str[line]) != 0) {
					++line;
					count = 0;
				}
			}
			if (count > lstrlen(str[line]))
				count = lstrlen(str[line]);*/
			if (line < 9) {
				if (str[line + 1][count] != '\0' && str[line + 1][count] != ' ')
					++line;
				else if (count > 0 && str[line + 1][count - 1] != '\0' && str[line + 1][count - 1] != ' ')
					++line;
				else if (lstrlen(str[line + 1]) == 0 && lstrlen(str[line]) != 0) {
					++line;
					count = 0;
				}
			}
		}
		else if (wParam == VK_HOME) {
			count = 0;
		}
		else if (wParam == VK_END) {
			if (lstrlen(str[line]) < 30) {
				count = lstrlen(str[line]);
			}
			else {
				++line;
				count = 0;
			}
		}
		else if (wParam == VK_PRIOR) {
			for (int i = 0; i < 3; ++i) {
				if (line > 0 && count != 0 && str[line - 1][count - 1] != '\0' && str[line - 1][count - 1] != ' ')
					--line;
				else if (line > 0 && count == 0 && str[line - 1][count] != '\0' && str[line - 1][count] != ' ')
					--line;
				else {
					line = 9;
					if (count != 0) {
						while (str[line][count - 1] == '\0' && str[line][count - 1] != ' ')
							--line;
					}
					else {
						while (str[line][count] == '\0' && str[line][count] != ' ')
							--line;
					}
				}
			}
		}
		else if (wParam == VK_NEXT) {
			for (int i = 0; i < 3; ++i) {
				if (line < 9 && count != 0 && str[line + 1][count - 1] != '\0' && str[line + 1][count - 1] != ' ')
					++line;
				else if (line < 9 && count == 0 && str[line + 1][count] != '\0' && str[line + 1][count] != ' ')
					++line;
				else {
					line = 0;
					if (count != 0) {
						while (str[line][count - 1] == '\0' && str[line][count - 1] != ' ')
							++line;
					}
					else {
						while (str[line][count] == '\0' && str[line][count] != ' ')
							++line;
					}
				}
			}
		}
		else if (wParam == VK_INSERT) {
			if (insertCheck == 0)
				insertCheck = 1;
			else
				insertCheck = 0;
		}
		/*else if (wParam == VK_DELETE) {
			int wordcount = 0;

			tempcount = count;
			templine = line;
			while (str[templine][tempcount] != ' ') {
				if (tempcount > 0) {
					--tempcount;
					++wordcount;
				}
				else {
					--templine;
					++wordcount;
					tempcount = lstrlen(str[templine]);
				}
			}

			tempcount = count;
			templine = line;
			while (str[templine][tempcount + 1] != ' ') {
				if (tempcount < 30) {
					++tempcount;
					++wordcount;
				}
				else {
					++templine;
					++wordcount;
					tempcount = 0;
				}
			}

			while (templine != 9 || tempcount != 30) {
				int tempwordcount = wordcount;
				int movecount = tempcount;
				int moveline = templine;
				while (tempwordcount != 0) {
					if (movecount > 0) {
						--movecount;
						--tempwordcount;
					}
					else if (movecount == 0 && moveline > 0) {
						--moveline;
						while (lstrlen(str[moveline]) == 0 && moveline > 0)
							--moveline;
						movecount = lstrlen(str[moveline]);
						--tempwordcount;
					}
					else if (movecount == 0 && moveline == 0)
						wordcount -= tempwordcount;
				}

				str[moveline][movecount] = str[templine][tempcount];

				if(tempcount < 30)
					++tempcount;
				else {
					++templine;
					while (lstrlen(str[templine]) == 0 && templine < 9) 
						++templine;
					tempcount = 0;
				}
			}
		}*/
		else if (wParam == VK_DELETE) {
			if (str[line][count] != ' ' && str[line][count] != '\0') {
				TCHAR flat[311] = { 0 };
				int writeIdx = 0;
				int flatCursor = 0;

				for (int i = 0; i < 10; i++) {
					if (i < line) flatCursor += lstrlen(str[i]);
					else if (i == line) flatCursor += count;

					for (int j = 0; j < 30; j++) {
						if (str[i][j] != '\0') {
							flat[writeIdx++] = str[i][j];
						}
					}
				}
				flat[writeIdx] = '\0';

				int left = flatCursor;
				while (left > 0 && flat[left - 1] != ' ') --left;

				int right = flatCursor;
				while (right < writeIdx && flat[right] != ' ' && flat[right] != '\0') ++right;
				if (right < writeIdx && flat[right] == ' ') ++right;

				int deleteLen = right - left;
				if (deleteLen > 0) {
					for (int i = left; i < writeIdx; i++) {
						flat[i] = (i + deleteLen < 311) ? flat[i + deleteLen] : '\0';
					}
				}

				for (int i = 0; i < 10; i++) memset(str[i], 0, sizeof(TCHAR) * 31);

				int flatReadIdx = 0;
				int currentTotalLen = lstrlen(flat);

				for (int i = 0; i < 10; i++) {
					for (int j = 0; j < saveLen[i]; j++) {
						if (flatReadIdx < currentTotalLen) {
							str[i][j] = flat[flatReadIdx++];
						}
					}
					saveLen[i] = lstrlen(str[i]);
				}

				int finalFlatPos = left;
				if (left >= currentTotalLen) {
					finalFlatPos = currentTotalLen;
					while (finalFlatPos > 0 && flat[finalFlatPos - 1] == ' ') {
						finalFlatPos--;
					}
				}

				int tempSum = 0;
				for (int i = 0; i < 10; i++) {
					if (finalFlatPos <= tempSum + saveLen[i]) {
						line = i;
						count = finalFlatPos - tempSum;
						break;
					}
					tempSum += saveLen[i];
				}

				InvalidateRect(hWnd, NULL, TRUE);
			}
		}
		else if (wParam == VK_ESCAPE) {
			PostQuitMessage(0);
		}

		InvalidateRect(hWnd, NULL, TRUE);
		break;
	case WM_CHAR:
		hDC = GetDC(hWnd);

		if (wParam == VK_BACK) {
			/*if (count == 0) {
				if (line > 0) {
					--line;
					count = saveLen[line];
				}
				InvalidateRect(hWnd, NULL, TRUE);
				break;
			}*/
			if (count > 0) {
				/*TCHAR flat[311] = { 0 };
				int writeIdx = 0;
				int flatCursor = 0;

				for (int i = 0; i < 10; i++) {
					if (i < line) flatCursor += lstrlen(str[i]);
					else if (i == line) flatCursor += count;

					for (int j = 0; j < 30; j++) {
						if (str[i][j] != '\0') {
							flat[writeIdx++] = str[i][j];
						}
					}
				}
				flat[writeIdx] = '\0';

				int deleteIdx = flatCursor - 1;

				if (deleteIdx >= 0) {
					for (int i = deleteIdx; i < writeIdx; i++) {
						if (i + 1 < 311)
							flat[i] = flat[i + 1];
						else
							flat[i] = '\0';
					}
				}

				for (int i = 0; i < 10; i++) memset(str[i], 0, sizeof(TCHAR) * 31);

				int flatReadIdx = 0;
				int currentTotalLen = lstrlen(flat);

				for (int i = 0; i < 10; i++) {
					for (int j = 0; j < saveLen[i]; j++) {
						if (flatReadIdx < currentTotalLen) {
							str[i][j] = flat[flatReadIdx++];
						}
					}
					saveLen[i] = lstrlen(str[i]);
				}

				int tempSum = 0;
				for (int i = 0; i < 10; i++) {
					if (deleteIdx <= tempSum + saveLen[i]) {
						line = i;
						count = deleteIdx - tempSum;
						break;
					}
					tempSum += saveLen[i];
				}

				InvalidateRect(hWnd, NULL, TRUE);*/
				for (int j = count - 1; j < 30; j++) {
					str[line][j] = (j + 1 < 31) ? str[line][j + 1] : '\0';
				}

				count--;
				saveLen[line] = lstrlen(str[line]);
			}
			else if (line > 0) {
				int prevLen = lstrlen(str[line - 1]);
				int curLen = lstrlen(str[line]);
				int spaceInPrev = 30 - prevLen;

				if (curLen > 0) {
					int moveCount;
					if (curLen < spaceInPrev) {
						moveCount = curLen;
					}
					else {
						moveCount = spaceInPrev;
					}

					_tcsncat(str[line - 1], str[line], moveCount);
					saveLen[line - 1] = lstrlen(str[line - 1]);

					for (int i = 0; i < 31; i++) {
						if (i + moveCount < 31) {
							str[line][i] = str[line][i + moveCount];
						}
						else {
							str[line][i] = '\0';
						}
					}
					saveLen[line] = lstrlen(str[line]);

					line--;
					count = prevLen;

					if (lstrlen(str[line + 1]) == 0) {
						for (int i = line + 1; i < 9; i++) {
							_tcscpy(str[i], str[i + 1]);
							saveLen[i] = lstrlen(str[i]);
						}
						memset(str[9], 0, sizeof(TCHAR) * 31);
						saveLen[9] = 0;
					}
				}
				else {
					for (int i = line; i < 9; i++) {
						_tcscpy(str[i], str[i + 1]);
						saveLen[i] = lstrlen(str[i]);
					}
					memset(str[9], 0, sizeof(TCHAR) * 31);
					saveLen[9] = 0;

					line--;
					count = prevLen;
				}
			}
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == VK_RETURN) {
			/*if (line < 9) {
				if (saveLen[line] < count)
					saveLen[line] = count;
				++line;
				count = 0;
			}*/

			if (lstrlen(str[9]) > 0)
				break;

			for (int i = 8; i > line; --i) {
				lstrcpy(str[i + 1], str[i]);
				saveLen[i + 1] = saveLen[i];
			}
			memset(str[line + 1], 0, sizeof(str[line + 1]));

			lstrcpy(str[line + 1], &str[line][count]);
			saveLen[line + 1] = lstrlen(str[line + 1]);

			for (int i = count; i < count + lstrlen(str[line + 1]); ++i)
				str[line][i] = '\0';
			saveLen[line] = lstrlen(str[line]);

			++line;
			count = 0;

			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == VK_TAB) {
			for (int k = 0; k < 4; ++k) {
				int charsAfterCursor = 0;
				for (int i = line; i < 10; i++) {
					int startCol = (i == line) ? count : 0;
					int lineLen = lstrlen(str[i]);
					if (lineLen > startCol) charsAfterCursor += (lineLen - startCol);
				}
				int cellsAfterCursor = (9 - line) * 30 + (30 - count);
				if (charsAfterCursor >= cellsAfterCursor) break;

				if (count >= 30) {
					if (line < 9) {
						line++;
						count = 0;
					}
					else {
						break;
					}
				}

				TCHAR flat[311] = { 0 };
				int writeIdx = 0;
				int flatCursor = 0;
				for (int i = 0; i < 10; i++) {
					if (i < line) flatCursor += lstrlen(str[i]);
					else if (i == line) flatCursor += count;

					for (int j = 0; j < 30; j++) {
						if (str[i][j] != '\0') flat[writeIdx++] = str[i][j];
					}
				}
				flat[writeIdx] = '\0';


				for (int i = writeIdx; i >= flatCursor; i--) {
					if (i + 1 < 310) flat[i + 1] = flat[i];
				}
				flat[flatCursor] = ' ';
				int newTotalLen = lstrlen(flat);

				for (int i = 0; i < 10; i++) memset(str[i], 0, sizeof(TCHAR) * 31);

				int flatReadIdx = 0;
				for (int i = 0; i < 10; i++) {
					int targetLen = saveLen[i];

					if (i >= line && targetLen < 30) {
						targetLen++;
					}

					for (int j = 0; j < targetLen; j++) {
						if (flatReadIdx < newTotalLen && j < 30) {
							str[i][j] = flat[flatReadIdx++];
						}
					}
					saveLen[i] = lstrlen(str[i]);
				}

				while (flatReadIdx < newTotalLen) {
					bool placed = false;
					for (int i = line + 1; i < 10; i++) {
						if (lstrlen(str[i]) < 30) {
							str[i][lstrlen(str[i])] = flat[flatReadIdx++];
							saveLen[i] = lstrlen(str[i]);
							placed = true;
							break;
						}
					}
					if (!placed) break;
				}

				count++;
				if (count > 30 && line < 9) { line++; count = 0; }

				InvalidateRect(hWnd, NULL, TRUE);
			}
		}
		else if (wParam == VK_SPACE) {
			int charsAfterCursor = 0;
			for (int i = line; i < 10; i++) {
				int startCol = (i == line) ? count : 0;
				int lineLen = lstrlen(str[i]);
				if (lineLen > startCol) {
					charsAfterCursor += (lineLen - startCol);
				}
			}

			int cellsAfterCursor = (9 - line) * 30 + (30 - count);

			if (charsAfterCursor >= cellsAfterCursor) break;

			if (count >= 30) {
				if (line < 9) { line++; count = 0; }
				InvalidateRect(hWnd, NULL, TRUE);
				break;
			}

			TCHAR flat[311] = { 0 };
			int writeIdx = 0;
			int flatCursor = 0;
			for (int i = 0; i < 10; i++) {
				if (i < line) flatCursor += lstrlen(str[i]);
				else if (i == line) flatCursor += count;

				for (int j = 0; j < 30; j++) {
					if (str[i][j] != '\0') flat[writeIdx++] = str[i][j];
				}
			}
			flat[writeIdx] = '\0';


			for (int i = writeIdx; i >= flatCursor; i--) {
				if (i + 1 < 310) flat[i + 1] = flat[i];
			}
			flat[flatCursor] = ' ';
			int newTotalLen = lstrlen(flat);

			for (int i = 0; i < 10; i++) memset(str[i], 0, sizeof(TCHAR) * 31);

			int flatReadIdx = 0;
			for (int i = 0; i < 10; i++) {
				int targetLen = saveLen[i];

				if (i >= line && targetLen < 30) {
					targetLen++;
				}

				for (int j = 0; j < targetLen; j++) {
					if (flatReadIdx < newTotalLen && j < 30) {
						str[i][j] = flat[flatReadIdx++];
					}
				}
				saveLen[i] = lstrlen(str[i]);
			}

			while (flatReadIdx < newTotalLen) {
				bool placed = false;
				for (int i = line + 1; i < 10; i++) {
					if (lstrlen(str[i]) < 30) {
						str[i][lstrlen(str[i])] = flat[flatReadIdx++];
						saveLen[i] = lstrlen(str[i]);
						placed = true;
						break;
					}
				}
				if (!placed) break;
			}

			count++;
			if (count > 30 && line < 9) { line++; count = 0; }

			InvalidateRect(hWnd, NULL, TRUE);
		}
		else {
			if (insertCheck == 1) {
				bool hasSpace = false;
				for (int i = line; i < 10; i++) {
					if (lstrlen(str[i]) < 30) {
						hasSpace = true;
						break;
					}
				}
				if (!hasSpace) break;

				if (count >= 30) {
					if (line < 9) {
						line++;
						count = 0;
					}
					else {
						break;
					}
				}

				TCHAR carryChar = (TCHAR)wParam; 

				for (int i = line; i < 10; i++) {
					int currentLen = lstrlen(str[i]);
					TCHAR nextCarry = '\0';

					if (currentLen >= 30) {
						nextCarry = str[i][29];
						str[i][29] = '\0';
					}

					int startIdx = (i == line) ? count : 0;

					for (int j = 29; j > startIdx; j--) {
						str[i][j] = str[i][j - 1];
					}

					str[i][startIdx] = carryChar;
					saveLen[i] = lstrlen(str[i]);

					if (nextCarry == '\0') break;
					carryChar = nextCarry;
				}

				count++;
				if (count >= 30 && line < 9) {
					line++;
					count = 0;
				}
			}
			else {
				if (count < 30) {
					str[line][count++] = wParam;
					if (count > saveLen[line]) {
						saveLen[line] = count;
					}
				}
				else {
					if (line < 9) {
						saveLen[line] = count;
						count = 0;
						++line;
						str[line][count++] = wParam;
					}
				}
			}
		}
		InvalidateRect(hWnd, NULL, TRUE);
		break;
	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);

		SelectObject(hDC, GetStockObject(SYSTEM_FIXED_FONT));		// 출력되는 모든 글자의 간격을 동일하게 만듦

		for (int i = 0; i < 10; ++i) 
			TextOut(hDC, 0, i * 20, str[i], lstrlen(str[i]));

		GetTextExtentPoint32(hDC, str[line], count, &size);
		SetCaretPos(size.cx, line * 20);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		HideCaret(hWnd);
		DestroyCaret();
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}