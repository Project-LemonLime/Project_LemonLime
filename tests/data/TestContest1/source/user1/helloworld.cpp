#include <cstring>
#include <iostream>
#include <vector>

using namespace std;

int f(int n) {
	vector<int> a(1);
	for (int i = 0; i < n; i++) {
		a[i] = 100;
	}
}
int *p;
int main() {
	int s;
	cin >> s;
	switch (s) {
		case 1:
			cout << "AC" << endl;
			break;
		case 2:
			cout << "WA" << endl;
			break;
		case 3:
			for (int i = 0; i < 200; i++)
				p = new int[1 << 24], memset(p, 0x3f, sizeof(int) * (1 << 24));
			break;
		case 4:
			while (1)
				;
			break;
		case 5:
			f(100);
			break;
		default:
			break;
	}
}