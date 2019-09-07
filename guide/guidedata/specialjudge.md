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
ifstream fin,fout,fstd,fper;
ofstream fsco,fre;
int main(int argc,char *argv[])
{
	fin.open(argv[1]),fout.open(argv[2]),fstd.open(argv[3]),fper.open(argv[4]),fsco.open(argv[5]),fre.open(argv[6]);
	int scr;
	double a,b,x;
	scr=atoi(argv[4]);
	fout>>a,fstd>>b;
	if(isinf(a) || isnan(a))scr*=0.0,fsco<<scr,fre<<"It won't work!";
	else
	{
		x=fabs(a-b)*100;
		x=x/(b+1e-7);
		if(x>1.00)scr*=0.0,fsco<<scr,fre<<"The diff has reached "<<setprecision(2)<<fixed<<x<<"%. Failed!";
		else scr*=1.0,fsco<<scr,fre<<"The diff is "<<setprecision(2)<<fixed<<x<<"%. Deja vu!";
	}
	fin.close(),fout.close(),fstd.close(),fper.close(),fsco.close(),fre.close();
	return 0;
}
```

（你猜这是什么题）

## testlib

在 LemonLime 源代码可以找到 `testlib_for_lemons.h` 。

具体使用参照其它的资料。

你的 spj 需要以下的操作：

```cpp
#include"testlib_for_lemons.h"

/**/


int main(int argc,char **argv)
{
	registerLemonChecker(argc,argv);

	/**/
```

