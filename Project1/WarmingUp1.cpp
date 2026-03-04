#include<iostream>
#include<string>
#include<cctype>
#include<vector>
#include<algorithm>
using namespace std;

int main()
{
	string str;
	getline(cin, str, '.');			// 문장 입력
	int modechange{};					// 3,4 명령어에서 원본/수정본 구분용

	while (1) {
		char cmd{};						// 커맨드 입력

		cout << "Command : ";
		cin >> cmd;

		if (cmd == '0')				// 0 입력 시 프로그램 종료
			break;

		if (cmd >= 'a' && cmd <= 'z') {							// 대소문자 변환
			for (int i = 0; i < str.size(); ++i) {
				if (str[i] == cmd)
					str[i] = toupper(str[i]);
				else if (str[i] == cmd - 32)
					str[i] = tolower(str[i]);
			}
			if (str.size() > 40)
				cout << str.substr(0, 40) << '.' << endl;		// 40개 문자만 출력
			else
				cout << str << '.' << endl;
		}

		if (cmd == '1') {
			string nextstr = "";
			for (int i = 0; i < str.size(); ++i) {							// nextstr에 빈칸 제거한 문장 저장 후 str로 옮기기
				if (str[i] == ' ') {
					nextstr += ' ';
					while (i + 1 < str.size() && str[i + 1] == ' ') {		// 빈칸수만큼 저장(나중 삭제)
						nextstr += ' ';
						++i;
					}
					if (!nextstr.empty() && nextstr.back() == ' ')
						nextstr.pop_back();
				}
				else
					nextstr += str[i];
			}
			str = nextstr;
			if (str.size() > 40)
				cout << str.substr(0, 40) << '.' << endl;		// 40개 문자만 출력
			else
				cout << str << '.' << endl;
		}

		if (cmd == '2') {
			string nextstr = "";
			for (int i = 0; i < str.size(); ++i) {
				if (str[i] == ' ') {
					int sc = 0;
					while (i < str.size() && str[i] == ' ') {		// 현재 빈칸 수 만큼 문장 저장
						if (sc < 5) {
							nextstr += ' ';
							++sc;
						}
						++i;
					}
					if (sc < 5) nextstr += ' ';						// 빈칸 추가
					--i;
				}
				else {
					nextstr += str[i];
				}
			}
			str = nextstr;
			if (str.size() > 40)
				cout << str.substr(0, 40) << '.' << endl;		// 40개 문자만 출력
			else
				cout << str << '.' << endl;
		}

		if (cmd == '3') {
			if (modechange) {
				if (str.size() > 40)
					cout << str.substr(0, 40) << '.' << endl;		// 40개 문자만 출력
				else
					cout << str << '.' << endl;
				modechange = 0;
			}
			else {
				vector<int> v(52);
				char alpha = 96;
				char ALPHA = 64;
				int oc{};
				for (auto a : str) {
					if (a >= 'a' && a <= 'z')
						++v[a - 97];					// v[0~25] > a~z 개수
					else if (a >= 'A' && a <= 'Z')
						++v[a - 39];					// v[26~51] > A~Z 개수
				}
				for (int i = 0; i < 26; ++i) {
					++alpha;
					if (v[i] > 0) {
						cout << alpha << v[i];
						oc = oc + 2;
						if (v[i] > 9)
							++oc;
						if (oc > 40)
							break;
					}
				}
				for (int i = 26; i < 51; ++i) {
					++ALPHA;
					if (v[i] > 0) {
						cout << ALPHA << v[i];
						oc = oc + 2;
						if (v[i] > 9)
							++oc;
						if (oc > 40)
							break;
					}
				}
				cout << '.' << endl;
				modechange = 1;
			}
		}
		if (cmd == '4') {
			if (modechange) {
				if (str.size() > 40)
					cout << str.substr(0, 40) << '.' << endl;		// 40개 문자만 출력
				else
					cout << str << '.' << endl;
				modechange = 0;
			}
			else {
				vector<string> words;
				string temp = "";
				for (char c : str) {
					if (c != ' ') temp += c;
					else if (!temp.empty()) {
						words.push_back(temp);
						temp = "";
					}
				}
				if (!temp.empty()) words.push_back(temp);

				sort(words.begin(), words.end(), [](const string& a, const string& b) {
					if (a.size() != b.size()) return a.size() < b.size();					// 길이 비교
					return a < b;															// 길이가 같다면 알파벳 순서 비교
					});

				for (auto a : words)
					cout << a << " ";
				cout << '\n';
				modechange = 1;
			}
		}
	}
}