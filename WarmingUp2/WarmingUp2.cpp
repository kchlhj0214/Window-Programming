#include <iostream>
#include <string>
#include <cctype>		// toupper, tolower
#include <vector>
#include <algorithm>	// swap, shuffle
#include <random>
#include <windows.h>
#include <conio.h>		// getch

using namespace std;
random_device rd;
mt19937 g(rd());
uniform_int_distribution<> uid{ 0, 9 };

int main()
{
	vector<vector<int>> ary(10, vector<int>(10));
	int x{ uid(g) }, y{ uid(g) };			// 커서 위치
	int enter{};							// 엔터 명령어 수행 시 숫자를 더할 지 원복할지 결정
	int bluesave{-1};						// 현재 파란색으로 칠해진 숫자를 저장(만약 0으로 초기화 하면 첫 행렬 출력에 0이 파란색으로 칠해짐)
	int entersave{};						// ex) 6 > enter > 8 > enter 입력 시 더한값이 60이지만 뺀 값이 80이 되어 행렬에 음수값이 들어감을 방지하기 위한 변수
	int curpossave[2];						// ex) 3 > enter > a >> enter 입력 시 숫자가 더해진 위치가 아닌 새로운 커서의 위치의 숫자가 감소함을 방지하기 위한 변수

	for (int i = 0; i < 10; ++i) {			// 초기 행렬 생성
		vector<int> row = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		shuffle(row.begin(), row.end(), g);
		ary[i] = row;
		
		for (int j = 0; j < 10; ++j) {
			if (i == x && j == y) {
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 11);
				cout << ary[i][j] << " ";
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
			}
			else
				cout << ary[i][j] << " ";
		}
		cout << endl;
	}

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);			// hConsole에 콘솔창을 다루는 권한을 줌

	while (1) {
		COORD pos = { 0, 0 };
		SetConsoleCursorPosition(hConsole, pos);				// 콘솔을 pos위치로 이동

		for (int i = 0; i < 10; ++i) {					// 콘솔 창 새로 그리기
			for (int j = 0; j < 10; ++j) {
				if (i == x && j == y) {
					SetConsoleTextAttribute(hConsole, 12); // 커서: 빨강
				}
				else if (ary[i][j] == bluesave) {
					SetConsoleTextAttribute(hConsole, 9);  // 찾은 숫자: 파랑 (밝은 파랑은 9, 일반은 1)
				}
				else {
					SetConsoleTextAttribute(hConsole, 15); // 기본: 흰색
				}
				cout << ary[i][j] << " ";
			}
			cout << endl;
		}

		if (_kbhit()) {
			int key = _getch();

			if (key >= '0' && key <= '9') {
				int CharToInt = key - '0';
				if (bluesave == CharToInt)
					bluesave = -1;
				else
					bluesave = CharToInt;
			}
			
			// wasd 커서 위치 이동
			switch (key) {
			case 'w': if (x > 0) --x; break; // 위
			case 's': if (x < 9) ++x; break; // 아래
			case 'a': if (y > 0) --y; break; // 왼쪽
			case 'd': if (y < 9) ++y; break; // 오른쪽
			case 'q':  return 0;             // q 누르면 프로그램 종료
			}

			// 엔터 명령어
			if (key == 13) {
				if (enter == 0) {
					if (bluesave != -1) {
						entersave = bluesave;
						curpossave[0] = x;
						curpossave[1] = y;
						ary[x][y] += 10 * entersave;
						++enter;
					}
				}
				else if (enter == 1) {
					ary[curpossave[0]][curpossave[1]] -= 10 * entersave;
					--enter;
				}
			}

			// 방향키 행렬 이동
			if (key == 224) {		// 특수키인지 확인
				key = _getch();
				switch (key) {
				case 72: 
					if (x > 0) {
						swap(ary[x], ary[x - 1]);
						if (enter == 1) {
							if (curpossave[0] == x) curpossave[0]--;
							else if (curpossave[0] == x - 1) curpossave[0]++;
						}
						--x;
					}
					else if(x == 0) {
						swap(ary[0], ary[9]);
						if (enter == 1) {
							if (curpossave[0] == 0) curpossave[0] = 9;
							else if (curpossave[0] == 9) curpossave[0] = 0;
						}
						x = 9;
					}
					break;
				case 80: 
					if (x < 9) {
						swap(ary[x], ary[x + 1]);
						if (enter == 1) {
							if (curpossave[0] == x) curpossave[0]++;
							else if (curpossave[0] == x + 1) curpossave[0]--;
						}
						++x;
					}
					else if (x == 9) {
						swap(ary[9], ary[0]);
						if (enter == 1) {
							if (curpossave[0] == 9) curpossave[0] = 0;
							else if (curpossave[0] == 0) curpossave[0] = 9;
						}
						x = 0;
					}
					break;
				case 75: 
					if (y > 0) {
						for (int i = 0; i < 10; ++i)
							swap(ary[i][y], ary[i][y - 1]);
						if (enter == 1) {
							if (curpossave[1] == y) curpossave[1]--;
							else if (curpossave[1] == y - 1) curpossave[1]++;
						}
						--y;
					}
					else if (y == 0) {
						for (int i = 0; i < 10; ++i)
							swap(ary[i][0], ary[i][9]);
						if (enter == 1) {
							if (curpossave[1] == 0) curpossave[1] = 9;
							else if (curpossave[1] == 9) curpossave[1] = 0;
						}
						y = 9;
					}
					break;
				case 77: 
					if (y < 9) {
						for (int i = 0; i < 10; ++i)
							swap(ary[i][y], ary[i][y + 1]);
						if (enter == 1) {
							if (curpossave[1] == y) curpossave[1]++;
							else if (curpossave[1] == y + 1) curpossave[1]--;
						}
						++y;
					}
					else if (y == 9) {
						for (int i = 0; i < 10; ++i)
							swap(ary[i][9], ary[i][0]);
						if (enter == 1) {
							if (curpossave[1] == 9) curpossave[1] = 0;
							else if (curpossave[1] == 0) curpossave[1] = 9;
						}
						y = 0;
					}
					break;
				}
			}

			// 행렬 리셋
			if (key == 'r') {
				for (int i = 0; i < 10; ++i) {
					vector<int> row = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
					shuffle(row.begin(), row.end(), g);
					ary[i] = row;
				}

				bluesave = -1;
				enter = 0;
				entersave = 0;
				x = uid(g);
				y = uid(g);
			}
		}
	}
}