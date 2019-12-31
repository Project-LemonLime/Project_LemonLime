# 自定义校验器 (Special Judge)

LemonLime 的校验器和其它版本的 Lemon 通用。

你的校验器需要接收6个参数流：

argv[1] : （输入）标准输入文件

argv[2] : （输入）选手的输出文件

argv[3] : （输入）答案文件

argv[4] : （输入）这个测试点的满分

argv[5] : （输出）得到的分数（一个整数）

argv[6] : （输出）额外信息

下面是一个例子。

```cpp
#include<bits/stdc++.h>
using namespace std;

ifstream fin, fout, fstd;
ofstream fscore, freport;

inline void quit(int score, const char *msg)
{
	fscore << score;
	freport << msg;

	fin.close();
	fout.close();
	fstd.close();
	fscore.close();
	freport.close();
	
	exit(0);
}

int main(int argc, char **argv)
{
	fin.open(argv[1]);
	fout.open(argv[2]);
	fstd.open(argv[3]);
	int fullScore = atoi(argv[4]);
	fscore.open(argv[5]);
	freport.open(argv[6]);

	string ps, js;
	fout >> ps;
	fstd >> js;
	
	int P = js.length();
	int Q = ps.length();

	int K = 0;
	for (int i = 0; i < min(P, Q); i++)
		if (js[i] == ps[i]) K++;

	if (Q > P) K -= Q - P;

	if (K >= P)
	{
		quit(fullScore, "Success: All Charming! Pure Memory! Million Master! YOU WILL AK IOI!");
		return 0;
	}

	double ratio = 1.00 * K / P;

	if (ratio >= 0.9999) { quit(0.9 * fullScore, "Almost Success: 90%!"); return 0; }
	if (ratio >= 0.999) { quit(0.8 * fullScore, "Nearly Success: 80%!"); return 0; }
	if (ratio >= 0.99) { quit(0.7 * fullScore, "Partly Success: 70%!"); return 0; }
	if (ratio >= 0.95) { quit(0.6 * fullScore, "Passed Success: 60%!"); return 0; }
	if (ratio >= 0.9) { quit(0.5 * fullScore, "Half Success: 50%!"); return 0; }
	if (ratio >= 0.6) { quit(0.4 * fullScore, "Great: 40%!"); return 0; }
	if (ratio >= 0.4) { quit(0.3 * fullScore, "Good: 30%!"); return 0; }
	if (ratio >= 0.1) { quit(0.2 * fullScore, "Poor: 20%."); return 0; }
	if (ratio >= 0.0000001145141919810) { quit(0.1 * fullScore, "Miss: 10%."); return 0; }

	quit(0, "Failed: @_@????????");

	return 0;
}

```

（你猜这是什么题）

## testlib

在 LemonLime 源代码可以找到 `testlib_for_lemons.h` 。

具体使用参照其它的资料。

把上面的 spj 重写后如下所示：

```cpp
#include<bits/stdc++.h>
#include"testlib_for_lemons.h"

using namespace std;

int main(int argc, char **argv)
{
	registerLemonChecker(argc, argv);
	int fullScore = perfectScore;

	string ps = ouf.readToken();
	string js = ans.readToken();

	int P = js.length();
	int Q = ps.length();

	int K = 0;
	for (int i = 0; i < min(P, Q); i++)
		if (js[i] == ps[i]) K++;

	if (Q > P) K -= Q - P;

	if (K >= P)
	{
		quitf(_ok, "Success: All Charming! Pure Memory! Million Master! YOU WILL AK IOI!");
		return 0;
	}

	double ratio = 1.00 * K / P;

	if (ratio >= 0.9999) { quitp(0.9 * fullScore, "Almost Success: 90%%!"); return 0; }
	if (ratio >= 0.999) { quitp(0.8 * fullScore, "Nearly Success: 80%%!"); return 0; }
	if (ratio >= 0.99) { quitp(0.7 * fullScore, "Partly Success: 70%%!"); return 0; }
	if (ratio >= 0.95) { quitp(0.6 * fullScore, "Passed Success: 60%%!"); return 0; }
	if (ratio >= 0.9) { quitp(0.5 * fullScore, "Half Success: 50%%!"); return 0; }
	if (ratio >= 0.6) { quitp(0.4 * fullScore, "Great: 40%%!"); return 0; }
	if (ratio >= 0.4) { quitp(0.3 * fullScore, "Good: 30%%!"); return 0; }
	if (ratio >= 0.1) { quitp(0.2 * fullScore, "Poor: 20%%."); return 0; }
	if (ratio >= 0.0000001145141919810) { quitp(0.1 * fullScore, "Miss: 10%%."); return 0; }

	quitf(_wa, "Failed: @_@????????");

	return 0;
}

```

