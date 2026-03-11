#include <iostream>
#include <string>
#include <cctype>
#include <vector>
#include <algorithm>
#include <sstream> 
#include <iomanip>
using namespace std;

int main()
{
	string str;
	vector<string> words;
	string temp = "";
	int strLen{40};
	vector<char> temppp;

	vector<string> recovWords;
	
	while (true) {
		getline(cin, str, '.');

		

		if (str.find('\n') != string::npos) {
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			continue;
		}
		if (str.size() > strLen) {
			str = str.substr(0, strLen);
		}

		break;
	}

	if (!str.empty()) {				// 문장을 빈칸과 단어로 구분해 한 벡터에 저장
		temp += str[0];
		for (int i = 1; i < str.size(); ++i) {
			if ((str[i] == ' ') != (str[i - 1] == ' ')) {
				words.push_back(temp);
				temp = "";
			}
			temp += str[i];
		}
		words.push_back(temp);
		recovWords.push_back(temp);
	}
	temp = "";

	int space{};		// 벡터의 첫 원소가 공백으로 시작하면 0, 단어로 시작하면 1
	if (words[0][0] != ' ')
		space = 1;

	int modechange{};					// 3,4 명령어에서 원본/수정본 구분용

	while (1) {
		char cmd{};						// 커맨드 입력

		cout << "Command : ";
		cin >> cmd;

		if (cmd == '0')				// 0 입력 시 프로그램 종료
			break;

		if (cmd >= 'a' && cmd <= 'z') {							// 대소문자 변환
			string nextstr = "";

			for (string& s : words) {
				if (s[0] != ' ') {								// 현재 문자열(단어)가 공백이 아닌지 확인 후 공백이 아니라면 대소문자 변환
					for (int i = 0; i < s.size(); ++i) {
						if (s[i] == cmd)
							s[i] = toupper(s[i]);
						else if (s[i] == cmd - 32)
							s[i] = tolower(s[i]);
					}
				}
			}

			for (string& s : words)								// 수정된 words를 str에 넣고 출력(40자 제한을 위해 str에 복사함)
				nextstr += s;
			str = nextstr;

			if (str.size() > strLen)
				cout << str.substr(0, strLen) << '.' << endl;		// 40개 문자만 출력
			else
				cout << str << '.' << endl;
		}

		if (cmd == '1') {
			string nextstr = "";

			/*for (int i = 0; i < words.size(); ++i) {			// 실패작 1
				if (words[i][0] == ' ')
					words[i].pop_back();
				else if (words[i][0] == NULL && words[i-1][0] != NULL && i != 0) {
					words[i-1].pop_back();
				}
			}*/

			/*for (int i = 0; i < words.size(); ++i) {			// 실패작 2
				if (words[i][0] == NULL && i != 0) {
					int nulCheck{};
					for (int j = i - 1; j >= 0; --j) {
						if (words[j][0] == NULL) {
							++nulCheck;
						}
						else
							break;
					}
					if (nulCheck != i)
						words[i - nulCheck].pop_back();
				}
				else if (words[i][0] == ' ')
					words[i].pop_back();
			}*/

			int abc{};

			for (int i = 0; i < (int)words.size(); ++i) {
				if (words[i][0] == ' ') {
					if (!words[i].empty()) {
						words[i].pop_back();
					}
				}
				else {
					if (i + 1 < words.size() && words[i + 1].empty()) {
						if (!words[i].empty()) {
							words[i].pop_back();
						}
					}
				}
			}
			if (words[(int)words.size() - 1].size() > 1) {
				for (int i = 0; i < (int)words.size(); ++i) {
					if (words[i][0] == NULL)
						++abc;
				}

				if (abc == (int)words.size() - 1) {
					temppp.push_back(words[(int)words.size() - 1][0]);
					words[(int)words.size() - 1].erase(0, 1);
				}
			}

			for (string& s : words)
				nextstr += s;
			str = nextstr;

			if (str.size() > strLen)
				cout << str.substr(0, strLen) << '.' << endl;		// 40개 문자만 출력
			else
				cout << str << '.' << endl;
		}

		if (cmd == '2') {
			string nextstr = "";

			for (int i = 0; i < words.size(); ++i) {
				// 1. 공백 추가 로직
				if (i % 2 == space && words[i].size() < 20)
					words[i] += ' ';

				// 2. recovWords 복구 로직 (인덱스 안전 장치 추가)
				if (i % 2 == 0 && i < 3 && i < recovWords.size()) { // size 체크 추가
					if (!recovWords[i].empty()) {
						// i % 2 대신 i를 사용하여 일관성 유지
						char recoveryChar = recovWords[i].back();
						recovWords[i].pop_back();

						words[i].insert(0, 1, recoveryChar);
					}
				}
			}

			// 3. temppp 복구 (words가 비어있지 않을 때만)
			if (!temppp.empty() && !words.empty()) {
				char recoveryChar = temppp.back();
				temppp.pop_back();

				words.back().insert(0, 1, recoveryChar);
			}

			// 결과 합치기 및 출력
			for (const string& s : words)
				nextstr += s;
			str = nextstr;

			if (str.size() > strLen)
				cout << str.substr(0, strLen) << '.' << endl;
			else
				cout << str << '.' << endl;
		}

		if (cmd == '3') {
			if (modechange) {
				if (str.size() > strLen)
					cout << str.substr(0, strLen) << '.' << endl;		// 40개 문자만 출력
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
						if (oc > strLen)
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
						if (oc > strLen)
							break;
					}
				}
				cout << '.' << endl;
				modechange = 1;
			}
		}

		if (cmd == '4') {
			if (modechange) {
				if (str.size() > strLen)
					cout << str.substr(0, strLen) << '.' << endl;		// 40개 문자만 출력
				else
					cout << str << '.' << endl;
				modechange = 0;
			}
			else {
				vector<string> sortedWords;
				for (const string& s : words) {
					if (!s.empty() && s[0] != ' ') {
						sortedWords.push_back(s);
					}
				}

				sort(sortedWords.begin(), sortedWords.end(), [](const string& a, const string& b) {
					if (a.size() != b.size())
						return a.size() < b.size(); // 1순위: 알파벳 개수(길이) 짧은 순
					return a < b;                   // 2순위: 알파벳 사전 순서(오름차순)
					});

				for (auto a : sortedWords)
					cout << a << " ";
				cout << '\n';
				modechange = 1;
			}
		}

		if (cmd == '5') {
			int n;
			cin >> n;
			if (n >= 0) {
				int curLen = (int)str.size();
				if (curLen > n)					// 문자열 잘라내기
					str = str.substr(0, n);
				else if (curLen < n) {			// 문자열 공백으로 추가(40글자 제한 X)
					//int diff = n - curLen;
					//for (int i = 0; i < diff; ++i)
					strLen = n;
				}
			}

			words.clear();

			if (!str.empty()) {				// 문장을 빈칸과 단어로 구분해 한 벡터에 저장
				temp += str[0];
				for (int i = 1; i < str.size(); ++i) {
					if ((str[i] == ' ') != (str[i - 1] == ' ')) {
						words.push_back(temp);
						temp = "";
					}
					temp += str[i];
				}
				words.push_back(temp);
			}
			cout << str << "." << endl;
		}
	}
}