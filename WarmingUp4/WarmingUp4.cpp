#include <iostream>		// AB★■□
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
uniform_int_distribution<> uid{ 0, 19 };

int main()
{	
	RESET_ALL:
	vector<vector<int>> ary(20, vector<int>(20));
	vector<vector<int>> originAry(20, vector<int>(20)); // 원본 맵 저장용
	
	int obsPos[3][2];		// 장애물 3개의 좌측 상단 모서리 좌표

	

	REPOS_ONE:				// 3x4
	obsPos[0][0] = uid(g);	// 행
	obsPos[0][1] = uid(g);	// 열
	if (obsPos[0][0] < 17 && obsPos[0][1] < 18) {
		for (int i = obsPos[0][0]; i < obsPos[0][0] + 4; ++i) {
			for (int j = obsPos[0][1]; j < obsPos[0][1] + 3; ++j) {
				ary[i][j] = 1;
			}
		}
	}
	else
		goto REPOS_ONE;

	REPOS_TWO:				// 5x2
	obsPos[1][0] = uid(g);
	obsPos[1][1] = uid(g);
	if (obsPos[1][0] < 19 && obsPos[1][1] < 16) {
		for (int i = obsPos[1][0]; i < obsPos[1][0] + 2; ++i) {
			for (int j = obsPos[1][1]; j < obsPos[1][1] + 5; ++j) {
				if (ary[i][j] == 1)
					goto REPOS_TWO;
			}
		}
	}
	else
		goto REPOS_TWO;
	for (int i = obsPos[1][0]; i < obsPos[1][0] + 2; ++i) {
		for (int j = obsPos[1][1]; j < obsPos[1][1] + 5; ++j)
			ary[i][j] = 1;
	}

	REPOS_THREE:			// 4x4
	obsPos[2][0] = uid(g);
	obsPos[2][1] = uid(g);
	if (obsPos[2][0] < 17 && obsPos[2][1] < 17) {
		for (int i = obsPos[2][0]; i < obsPos[2][0] + 4; ++i) {
			for (int j = obsPos[2][1]; j < obsPos[2][1] + 4; ++j) {
				if (ary[i][j] == 1)
					goto REPOS_THREE;
			}
		}
	}
	else
		goto REPOS_THREE;
	for (int i = obsPos[2][0]; i < obsPos[2][0] + 4; ++i) {
		for (int j = obsPos[2][1]; j < obsPos[2][1] + 4; ++j)
			ary[i][j] = 1;
	}

	int itemCnt{};			// 아이템 생성
	while (itemCnt < 10) {
		int x, y;
		x = uid(g);
		y = uid(g);
		if (ary[x][y] == 0) {
			ary[x][y] = 2;
			++itemCnt;
		}
	}

	int player[2][2];		// 플레이어의 현 위치 저장소
	int originPlayer[2][2]; // 원본 플레이어 위치 저장용
	int player1{}, player2{}; // 플레이어 아이템 점수
	while (1) {
		int x, y;
		x = uid(g);
		y = uid(g);
		if (ary[x][y] == 0) {
			player[0][0] = x;
			player[0][1] = y;
			break;
		}
	}
	while (1) {
		int x, y;
		x = uid(g);
		y = uid(g);
		if (ary[x][y] == 0) {
			player[1][0] = x;
			player[1][1] = y;
			break;
		}
	}

	for (int i = 0; i < 20; ++i) {			// 첫 보드판 출력
		for (int j = 0; j < 20; ++j) {
			if(i == player[0][0] && j == player[0][1])
				cout << "# ";
			else if(i == player[1][0] && j == player[1][1])
				cout << "@ ";
			else {
				if (ary[i][j] == 0)
					cout << "□ ";
				else if (ary[i][j] == 1)
					cout << "■ ";
				else if (ary[i][j] == 2)
					cout << "★ ";
			}
		}
		cout << endl;
	}

	originAry = ary;
	originPlayer[0][0] = player[0][0];
	originPlayer[0][1] = player[0][1];
	originPlayer[1][0] = player[1][0];
	originPlayer[1][1] = player[1][1];

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);			// hConsole에 콘솔창을 다루는 권한을 줌
	CONSOLE_CURSOR_INFO cursorInfo;
	GetConsoleCursorInfo(hConsole, &cursorInfo);
	cursorInfo.bVisible = FALSE;
	SetConsoleCursorInfo(hConsole, &cursorInfo);

	bool isPlayer1Turn = true; // true = #, false = @ / 본인 턴에만 움직일 수 있게 할 때

	while (1) {
		COORD pos = { 0, 0 };
		SetConsoleCursorPosition(hConsole, pos);				// 콘솔을 pos위치로 이동

		for (int i = 0; i < 20; ++i) {			// 보드판 다시 그리기
			for (int j = 0; j < 20; ++j) {
				if (i == player[0][0] && j == player[0][1])
					cout << "# ";
				else if (i == player[1][0] && j == player[1][1])
					cout << "@ ";
				else {
					if (ary[i][j] == 0)
						cout << "□ ";
					else if (ary[i][j] == 1)
						cout << "■ ";
					else if (ary[i][j] == 2)
						cout << "★ ";
				}
			}
			cout << endl;
		}

		cout << "Player # Score : " << player1 << "          " << endl << "Player @ Score : " << player2 << "          " << endl;

		if (player1 + player2 == 10) {
			if (player1 > player2)
				cout << "Player # wins!!" << endl;
			else if (player1 < player2)
				cout << "Player @ wins!!" << endl;
			else if (player1 == player2)
				cout << "Draw...." << endl;
			return 0;
		}

		if (_kbhit()) {
			int key = _getch();

			if (key == 'r') {
				system("cls");
				goto RESET_ALL;
			}

			int nextR = player[0][0];
			int nextC = player[0][1];
			
			int nextX = player[1][0];
			int nextY = player[1][1];
////-----------------------------------------(자신의 차례에만 돌 움직이기 가능)-----------------------------------------//
//			bool actionTaken = false;
//
//			if (isPlayer1Turn) {
//				if (key == 'w') { nextR--; actionTaken = true; }
//				else if (key == 's') { nextR++; actionTaken = true; }
//				else if (key == 'a') { nextC--; actionTaken = true; }
//				else if (key == 'd') { nextC++; actionTaken = true; }
//			}
//			else {
//				if (key == 'i') { nextX--; actionTaken = true; }
//				else if (key == 'k') { nextX++; actionTaken = true; }
//				else if (key == 'j') { nextY--; actionTaken = true; }
//				else if (key == 'l') { nextY++; actionTaken = true; }
//			}
//
//			if (key == 'q') {
//				cout << "# : " << player1 << endl << "@ : " << player2 << endl;
//				if (player1 > player2)
//					cout << "# wins!!" << endl;
//				else if (player1 < player2)
//					cout << "@ wins!!" << endl;
//				else if (player1 == player2)
//					cout << "Draw...." << endl;
//				return 0;
//			}
//
//			if (actionTaken) {
//				if (isPlayer1Turn) {
//					if (nextR < 0) nextR = 19; else if (nextR > 19) nextR = 0;
//					if (nextC < 0) nextC = 19; else if (nextC > 19) nextC = 0;
//
//					if (ary[nextR][nextC] != 1 && !(nextR == player[1][0] && nextC == player[1][1])) {
//						if (ary[nextR][nextC] == 2) { ary[nextR][nextC] = 1; ++player1; }
//						player[0][0] = nextR; player[0][1] = nextC;
//						isPlayer1Turn = false;
//					}
//				}
//				else {
//					if (nextX < 0) nextX = 19; else if (nextX > 19) nextX = 0;
//					if (nextY < 0) nextY = 19; else if (nextY > 19) nextY = 0;
//
//					if (ary[nextX][nextY] != 1 && !(nextX == player[0][0] && nextY == player[0][1])) {
//						if (ary[nextX][nextY] == 2) { ary[nextX][nextY] = 1; ++player2; }
//						player[1][0] = nextX; player[1][1] = nextY;
//						isPlayer1Turn = true;
//					}
//				}
//			}
//-----------------------------------------(연속으로 돌 움직이기 가능)-----------------------------------------//
			switch (key) {
				case 'w': nextR--; break; // 위: 행 감소
				case 's': nextR++; break; // 아래: 행 증가
				case 'a': nextC--; break; // 왼쪽: 열 감소
				case 'd': nextC++; break; // 오른쪽: 열 증가
				case 'i': nextX--; break; // 위: 행 감소
				case 'k': nextX++; break; // 아래: 행 증가
				case 'j': nextY--; break; // 왼쪽: 열 감소
				case 'l': nextY++; break; // 오른쪽: 열 증가
				case 'q': 
					cout << "# : " << player1 << endl << "@ : " << player2 << endl;
					if (player1 > player2)
						cout << "# wins!!" << endl;
					else if (player1 < player2)
						cout << "@ wins!!" << endl;
					else if (player1 == player2)
						cout << "Draw...." << endl;
					return 0;
			}
//---------------------------------------------(플레이어 1 움직이기)---------------------------------------------//
			if (nextR < 0) 
				nextR = 19;
			else if (nextR > 19) 
				nextR = 0;

			if (nextC < 0) 
				nextC = 19;
			else if (nextC > 19) 
				nextC = 0;

			if (ary[nextR][nextC] != 1 && !(nextR == player[1][0] && nextC == player[1][1])) {
				if (ary[nextR][nextC] == 2) {
					ary[nextR][nextC] = 1;
					++player1;
				}
				player[0][0] = nextR;
				player[0][1] = nextC;
			}
//---------------------------------------------(플레이어 2 움직이기)---------------------------------------------//
			if (nextX < 0)
				nextX = 19;
			else if (nextX > 19)
				nextX = 0;

			if (nextY < 0)
				nextY = 19;
			else if (nextY > 19)
				nextY = 0;

			if (ary[nextX][nextY] != 1 && !(nextX == player[0][0] && nextY == player[0][1])) {
				if (ary[nextX][nextY] == 2) {
					ary[nextX][nextY] = 1;
					++player2;
				}
				player[1][0] = nextX;
				player[1][1] = nextY;
			}
		}
	}
}