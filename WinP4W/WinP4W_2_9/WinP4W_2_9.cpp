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
	bool f1_on = false;
	bool f2_on = false;
	bool f3_on = false;
	bool f4_on = false;
	bool f5_on = false;
	bool f6_on = false;
	bool f7_on = false;
	bool f8_on = false;

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
		else if (wParam == VK_F1) {
			if (f1_on == false)
				f1_on = true;
			else
				f1_on = false;
		}
		else if (wParam == VK_F2) {
			if (f2_on == false)
				f2_on = true;
			else
				f2_on = false;
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == VK_F3) {
			if (f3_on == false)
				f3_on = true;
			else
				f3_on = false;
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == VK_F4) {
			if (f4_on == false)
				f4_on = true;
			else
				f4_on = false;
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == VK_F5) {
			if (f5_on == false)
				f5_on = true;
			else
				f5_on = false;
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == VK_F6) {
			if (f6_on == false)
				f6_on = true;
			else
				f6_on = false;
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == VK_F7) {
			if (f7_on == false)
				f7_on = true;
			else
				f7_on = false;
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else if (wParam == VK_F8) {
			if (f8_on == false)
				f8_on = true;
			else
				f8_on = false;
			InvalidateRect(hWnd, NULL, TRUE);
		}

		InvalidateRect(hWnd, NULL, TRUE);
		break;
	case WM_CHAR:
		hDC = GetDC(hWnd);

		if (wParam == VK_BACK) {
			if (count > 0) {
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
				TCHAR carryChar;
				/*if (f1_on) {
					if (wParam >= 'a' && wParam <= 'z')
						carryChar = (TCHAR)wParam - 32;
				}
				else
					carryChar = (TCHAR)wParam;*/
				carryChar = (TCHAR)wParam;

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
					/*if (f1_on) {
						str[line][count++] = wParam - 32;
						if (count > saveLen[line]) {
							saveLen[line] = count;
						}
					}
					else {
						str[line][count++] = wParam;
						if (count > saveLen[line]) {
							saveLen[line] = count;
						}
					}*/
					str[line][count++] = wParam;
					if (count > saveLen[line]) {
						saveLen[line] = count;
					}
				}
				else {
					if (line < 9) {
						/*if (f1_on) {
							saveLen[line] = count;
							count = 0;
							++line;
							str[line][count++] = wParam - 32;
						}
						else {
							saveLen[line] = count;
							count = 0;
							++line;
							str[line][count++] = wParam;
						}*/
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
		//hDC = BeginPaint(hWnd, &ps);

		//SelectObject(hDC, GetStockObject(SYSTEM_FIXED_FONT));		// 출력되는 모든 글자의 간격을 동일하게 만듦

		//for (int i = 0; i < 10; ++i)
		//	TextOut(hDC, 0, i * 20, str[i], lstrlen(str[i]));

		//GetTextExtentPoint32(hDC, str[line], count, &size);
		//SetCaretPos(size.cx, line * 20);
		//EndPaint(hWnd, &ps);
		//break;

		hDC = BeginPaint(hWnd, &ps);

		SelectObject(hDC, GetStockObject(SYSTEM_FIXED_FONT));		// 출력되는 모든 글자의 간격을 동일하게 만듦

		static TCHAR copystr[10][150];

		for (int i = 0; i < 10; ++i)
			_tcscpy_s(copystr[i], 150, str[i]);

		if (f1_on) {
			for (int i = 0; i < 10; ++i) {
				for (int j = 0; j < 150; ++j) {
					if (copystr[i][j] >= 'a' && copystr[i][j] <= 'z')
						copystr[i][j] -= 32;
				}
			}
		}
		else {
			for (int i = 0; i < 10; ++i)
				TextOut(hDC, 0, i * 20, str[i], lstrlen(str[i]));
		}

		if (f2_on) {
			for (int i = 0; i < 10; ++i) {
				int j = 0;
				while (copystr[i][j] != NULL) {
					if (copystr[i][j] >= '0' && copystr[i][j] >= '9') {
						for (int k = lstrlen(copystr[i]); k >= j; --k)
							copystr[i][k + 4] = copystr[i][k];
						copystr[i][j - 3] = '*';
						copystr[i][j - 2] = '*';
						copystr[i][j - 1] = '*';
						copystr[i][j] = '*';
					}
					++j;
				}
				TextOut(hDC, 0, i * 20, copystr[i], lstrlen(copystr[i]));
			}
		}
		else {
			for (int i = 0; i < 10; ++i)
				TextOut(hDC, 0, i * 20, str[i], lstrlen(str[i]));
		}

		if (f3_on) {
			TCHAR flat[1511] = { 0 };
			int writeIdx = 0;
			int flatCursor = 0;

			for (int i = 0; i < 10; i++) {
				if (i < line) flatCursor += lstrlen(copystr[i]);
				else if (i == line) flatCursor += count;

				for (int j = 0; j < 30; j++) {
					if (copystr[i][j] != '\0') {
						flat[writeIdx++] = copystr[i][j];
					}
				}
			}
			flat[writeIdx] = '\0';

			int left = flatCursor;
			while (left > 0 && flat[left - 1] != ' ') 
				--left;



			int right = flatCursor;
			while (right < writeIdx && flat[right] != ' ' && flat[right] != '\0') 
				++right;
			if (right < writeIdx && flat[right] == ' ') ++right;

			for (int i = 0; i < 10; i++) memset(copystr[i], 0, sizeof(TCHAR) * 150);

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
		}

		for (int i = 0; i < 10; ++i)
			TextOut(hDC, 0, i * 20, copystr[i], lstrlen(copystr[i]));

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