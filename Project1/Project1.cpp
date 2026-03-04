#include<iostream>
#include<string>
#include<cctype>
using namespace std;

int main()
{
	string str;
	getline(cin, str, '.');
	char cmd{};				//Command 변수
	
	while (1) {
		cout << "Command : ";
		cin >> cmd;


		if (cmd == '0')				//0 입력 시 프로그램 종료
			break;

		if(cmd >= 'a' && cmd <= 'z') {
			for (int i = 0; i < str.size(); ++i) {
				if (str[i] == cmd)
					str[i] = toupper(str[i]);
				else if (str[i] == cmd - 32)
					str[i] = tolower(str[i]);
			}
			cout << str << '.' << endl;		// 40개 문자만 출력되게 나중에 바꿀 것
		}

		if (cmd == '1') {
			for (int i = 0; i < str.size(); ++i) {
				if (str[i] != ' ')
					cout << str[i];
				else {
					if(i !=0 && str[i-1] == ' ')
						cout << str[i];
				}

			}
			cout << endl;
		}
		
		cout << str << '.' << endl;
	}
}