#include <iostream>
#include <string>
#include <cctype>
#include <vector>
#include <algorithm>
#include <random>
#include <windows.h>

using namespace std;
default_random_engine dre{};
uniform_int_distribution<> uid{ 0, 9 };

int main()
{
	vector<int> v(100);
	for (int i = 0; i < 100; ++i) {
		v[i] = uid(dre);
		cout << v[i] << endl;
	}
}