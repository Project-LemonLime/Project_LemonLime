#include <csignal>
#include <cstring>
#include <iostream>

using namespace std;

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
			std::raise(SIGSEGV);
			break;
		default:
			break;
	}
}
