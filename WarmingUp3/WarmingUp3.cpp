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
		cout << "          - " <<  movie[i].TheaterNum << " " << left << setw(12) << movie[i].MovieName;
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
	int mn{-1};
	for (int i = 0; i < 3; ++i) {
		if (str == movie[i].MovieName || str == to_string(movie[i].TheaterNum))
			mn = i;
	}
	if (mn == '-1') {
		cout << str << " is not found" << endl;
		return;
	}

	cout << "          - What Time : ";
	int time;
	cin >> time;
	int mt{-1};
	for (int i = 0; i < 3; ++i) {
		if (time == movie[mn].MovieTime[i])
			mt = i;
	}
	if (mt == '-1') {
		cout << time << " is wrong time" << endl;
	}

	cout << "          - Seat You Want : ";
	int sa, sb;
	cin >> sa >> sb;
	--sa;
	--sb;
	int reservNum{};
	if (sa >= 0 && sa <= 10 && sb >= 0 && sb <= 10 && movie[mn].MovieSeats[mt][sa][sb] == 0) {
		reservNum = uid(g);
		for (int i = 0; i < 3; ++i) {
			for (int j = 0; j < 3; ++j) {
				for (int n = 0; n < 10; ++n) {
					for (int m = 0; m < 10; ++m) {
						if (movie[i].MovieSeats[j][n][m] == reservNum) {		// 예약번호가 중복되지 않을 때까지 랜덤 값 부여
							i = 0;
							j = 0;
							n = 0;
							m = 0;
							reservNum = uid(g);
						}
					}
				}
			}
		}
		movie[mn].MovieSeats[mt][sa][sb] = reservNum;
	}
	else {
		cout << sa + 1 << ", " << sb + 1 << " is already selected or does not exist" << endl;
		return;
	}

	cout << "          - Reservation :" << endl;
	cout << "                    - Movie Name : " << movie[mn].MovieName << endl << "                    - Movie Time : " << movie[mn].MovieTime[mt] << endl <<
		"                    - Seat Number : (" << sa + 1 << ", " << sb + 1 << ")" << endl << "                    - Your Reservation Number is ";
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 11);
	cout << movie[mn].MovieSeats[mt][sa][sb];
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
	cout << ". " << "Thank you your reservation!" << endl;
}

void Cancel() {
	cout << "          - Enter your Reservation Number : ";
	int rn;
	cin >> rn;
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			for (int n = 0; n < 10; ++n) {
				for (int m = 0; m < 10; ++m) {
					if (movie[i].MovieSeats[j][n][m] == rn) {
						movie[i].MovieSeats[j][n][m] = 0;
						cout << "          - Please check your reservation information :" << endl << "                    - Movie Name : " << movie[i].MovieName << endl << "                    - Movie TIme : " << 
							movie[i].MovieTime[j] << endl << "                    - Seat Number : (" << n + 1 << ", " << m + 1 << ")" << endl << "                    - Your reservation is cancelled. Please, come again!" << endl;
						return;
					}
				}
			}
		}
	}
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
		else if(cmd == 'd')
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