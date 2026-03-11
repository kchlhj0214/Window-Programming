#include<iostream>
#include<vector>
#include<string>
#include <iomanip>
#include <random>
#include <windows.h>

using namespace std;
random_device rd;
mt19937 g(rd());
uniform_int_distribution<> uid{ 100, 999 };		// 예약번호 세자릿수 랜덤 저장 > 두자릿수 정수로 예약번호 배정 시 89개 이상의 예약이 생길 경우 예약번호가 중복되는 문제 발생(총 좌석 수 = 900)

struct MovieTheater {
	int TheaterNum{};
	string MovieName;
	int MovieTime[3]{};
	int MovieSeats[3][10][10];
};

MovieTheater movie[3];

void initValue() {
	movie[0].TheaterNum = 1;
	movie[0].MovieName = { "Avengers" };
	movie[0].MovieTime[0] = 1100;
	movie[0].MovieTime[1] = 1400;
	movie[0].MovieTime[2] = 1700;


	movie[1].TheaterNum = 2;
	movie[1].MovieName = { "Joker" };
	movie[1].MovieTime[0] = 1100;
	movie[1].MovieTime[1] = 1400;
	movie[1].MovieTime[2] = 1700;

	movie[2].TheaterNum = 3;
	movie[2].MovieName = { "IronMan" };
	movie[2].MovieTime[0] = 1100;
	movie[2].MovieTime[1] = 1400;
	movie[2].MovieTime[2] = 1700;

	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			for (int n = 0; n < 10; ++n) {
				for (int m = 0; m < 10; ++m)
					movie[i].MovieSeats[j][n][m] = 0;
			}
		}
	}
}

void MovieInfo() {
	for (int i = 0; i < 3; ++i) {
		cout << "          - " << movie[i].TheaterNum << " " << left << setw(12) << movie[i].MovieName;
		for (int j = 0; j < 3; ++j) {
			cout << " " << movie[i].MovieTime[j];
		}
		cout << endl;
	}
}

void PrintSeat(string theaternum) {
	int n;
	for (int i = 0; i < 3; ++i)
		if (theaternum == to_string(movie[i].TheaterNum) || theaternum == movie[i].MovieName)
			n = i;
	for (int i = 0; i < 3; ++i) {
		cout << "          - " << movie[n].MovieName << " " << movie[n].MovieTime[i] << " :" << endl;
		for (int k = 0; k < 10; ++k) {
			cout << "                    ";
			for (int j = 0; j < 10; ++j) {
				if (movie[n].MovieSeats[i][k][j] == 0)
					cout << "--- ";
				else
					cout << movie[n].MovieSeats[i][k][j] << " ";
			}
			cout << endl;
		}
	}
}

void ReservMovie() {
	cout << "          - Which Movie : ";
	string str;
	cin >> str;
	int mn{ -1 };
	for (int i = 0; i < 3; ++i) {
		if (str == movie[i].MovieName || str == to_string(movie[i].TheaterNum))
			mn = i;
	}
	if (mn == -1) {
		cout << str << " is not found" << endl;
		return;
	}

	cout << "          - What Time : ";
	int time;
	cin >> time;
	int mt{ -1 };
	for (int i = 0; i < 3; ++i) {
		if (time == movie[mn].MovieTime[i])
			mt = i;
	}
	if (mt == -1) {
		cout << time << " is wrong time" << endl;
		return;
	}

	cout << "          - How many seats? : ";
	int seatscnt{};
	cin >> seatscnt;
	if (seatscnt > 4) 
		return;

	
	int sa[4]{-1, -1, -1, -1}, sb[4]{-1, -1, -1, -1};
	cout << "          - Seat You Want : ";
	cin >> sa[0] >> sb[0];
	--sa[0];
	--sb[0];
	if (movie[mn].MovieSeats[mt][sa[0]][sb[0]] == 0) {
		for (int i = 1; i < seatscnt; ++i) {
			sa[i] = sa[i - 1];
			sb[i] = sb[i - 1] + 1;
			//if (!(cin >> sa[i] >> sb[i])) {
			//	cin.clear();
			//	string input;
			//	cin >> input;

			//	if (input == "r") {				// 좌석 선택 도중 r 입력시 초기화 후 명령어 칸으로 이동(한번 좌석선택으로 넘어오면 좌석을 모두 선택 후 예매해야 다음으로 넘어가지는 현상 방지)
			//		return;
			//	}
			//	else {
			//		cin.ignore(1000, '\n');
			//		--i;
			//		continue;
			//	}
			//}
			//cin.ignore(1000, '\n');


			int r = sa[i];
			int c = sb[i];

			bool isInvalid = false;

			if (r < 0 || r > 9 || c < 0 || c > 9 || movie[mn].MovieSeats[mt][r][c] != 0) {
				cout << "          - " << sa[i] + 1 << ", " << sb[i] + 1 << " is already selected or does not exist" << endl;
				return;
			}

			/*if (!isInvalid) {
				for (int j = 0; j < i; ++j) {
					if (sa[j] == r && sb[j] == c) {
						cout << "          - You've already chosen your " << sa[i] << ", " << sb[i] << endl;
						isInvalid = true;
						break;
					}
				}
			}*/

			if (isInvalid) {
				--i;
				continue;
			}

			sa[i] = r;
			sb[i] = c;
		}
	}

	int reservNum{};
	int rtc{};
	for (int k = 0; k < seatscnt; ++k) {
		if (sa[k] >= 0 && sa[k] <= 9 && sb[k] >= 0 && sb[k] <= 9 && movie[mn].MovieSeats[mt][sa[k]][sb[k]] == 0) {
			REGEN_NUM:
			reservNum = uid(g);
			for (int i = 0; i < 3; ++i) {
				for (int j = 0; j < 3; ++j) {
					for (int n = 0; n < 10; ++n) {
						for (int m = 0; m < 10; ++m) {
							if (movie[i].MovieSeats[j][n][m] == reservNum) {		// 예약번호가 중복되지 않을 때까지 랜덤 값 부여
								goto REGEN_NUM;
							}
						}
					}
				}
			}
		}
		else {
			cout << sa[k] + 1 << ", " << sb[k] + 1 << " is already selected or does not exist" << endl;
			++rtc;
		}
	}
	if (rtc != 0) {
		return;
	}
	for(int i = 0; i < seatscnt; ++i)
		movie[mn].MovieSeats[mt][sa[i]][sb[i]] = reservNum;


	cout << "          - Reservation :" << endl;
	cout << "                    - Movie Name : " << movie[mn].MovieName << endl << "                    - Movie Time : " << movie[mn].MovieTime[mt] << endl <<
		"                    - Seat Number : "; 
	for (int i = 0; i < seatscnt; ++i) {
		cout << "(" << sa[i] + 1 << ", " << sb[i] + 1 << ")" << " ";
	}
	cout << endl << "                    - Your Reservation Number is ";
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 11);
	cout << movie[mn].MovieSeats[mt][sa[0]][sb[0]];
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
	cout << ". " << "Thank you your reservation!" << endl;
}

void Cancel() {
	cout << "          - Enter your Reservation Number : ";
	int rn;
	cin >> rn;
	int x[4]{ -1, -1, -1, -1 }, y[4]{ -1, -1, -1, -1 };
	int a, b, c{}, d{};
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			for (int n = 0; n < 10; ++n) {
				for (int m = 0; m < 10; ++m) {
					if (movie[i].MovieSeats[j][n][m] == rn) {
						movie[i].MovieSeats[j][n][m] = 0;
						a = i;
						b = j;
						x[c++] = n + 1;
						y[d++] = m + 1;
					}
				}
			}
		}
	}
	cout << "          - Please check your reservation information :" << endl << "                    - Movie Name : " << movie[a].MovieName << endl << "                    - Movie TIme : " <<
		movie[a].MovieTime[b] << endl << "                    - Seat Number : ";
	for (int i = 0; i < c; ++i) {
		cout << "(" << x[i] << ", " << y[i] << ")" << " ";
	}
	cout << endl << "                    - Your reservation is cancelled. Please, come again!" << endl;
	return;
}

void ReservPercent() {

	for (int i = 0; i < 3; ++i) {
		cout << "          - " << left << movie[i].TheaterNum << " " << setw(12) << movie[i].MovieName;
		for (int j = 0; j < 3; ++j) {
			int booked{};
			cout << " " << movie[i].MovieTime[j] << " : ";
			for (int n = 0; n < 10; ++n) {
				for (int m = 0; m < 10; ++m) {
					if (movie[i].MovieSeats[j][n][m] != 0)
						++booked;
					if (n == 9 && m == 9) {
						cout << " " << right << setw(3) << booked << "% ";
					}
				}
			}
		}
		cout << endl;
	}
}

int main()
{
	initValue();
	int reservNum = 100;

	char cmd;
	while (1) {
		cout << "명령어 : ";
		cin >> cmd;
		string checkMovie;
		if (cmd == 'q')
			break;
		else if (cmd == 'd')
			MovieInfo();
		else if (cmd == 'p') {
			cin >> checkMovie;
			PrintSeat(checkMovie);
		}
		else if (cmd == 'r') {
			ReservMovie();
		}
		else if (cmd == 'c') {
			Cancel();
		}
		else if (cmd == 'h') {
			ReservPercent();
		}
	}
}