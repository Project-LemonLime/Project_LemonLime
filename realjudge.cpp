/***************************************************************************
    This file is part of Project Lemon
    Copyright (C) 2011 Zhipeng Jia

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
***************************************************************************/
/**
 * realjudge.cpp @Project LemonPt
 * Update 2019 iotang
 **/

#include <cstdio>
#include <math.h>

int main(int argc, char *argv[])
{
	FILE *contestantOutputFile = fopen(argv[1], "r");

	if(contestantOutputFile == NULL)
	{
		printf("Cannot open contestant\'s output file\n");
		return 0;
	}

	FILE *standardOutputFile = fopen(argv[2], "r");

	if(standardOutputFile == NULL)
	{
		printf("Cannot open standard output file\n");
		fclose(contestantOutputFile);
		return 0;
	}

	int realPrecision, i;
	sscanf(argv[3], "%d", &realPrecision);
	double eps = 1;

	for(i = 0; i < realPrecision; i ++)
		eps *= 0.1;

	double a, b;

	while(1)
	{
		int cnt1 = fscanf(contestantOutputFile, "%lf", &a);
		int cnt2 = fscanf(standardOutputFile, "%lf", &b);

		if(cnt1 == 0)
		{
			printf("Wrong answer\nInvalid characters found\n");
			fclose(contestantOutputFile);
			fclose(standardOutputFile);
			return 0;
		}

		if(cnt2 == 0)
		{
			printf("Invalid characters in standard output file\n");
			fclose(contestantOutputFile);
			fclose(standardOutputFile);
			return 0;
		}

		if(cnt1 == EOF && cnt2 == EOF) break;

		if(cnt1 == EOF && cnt2 == 1)
		{
			printf("Wrong answer\nShorter than standard output\n");
			fclose(contestantOutputFile);
			fclose(standardOutputFile);
			return 0;
		}

		if(cnt1 == 1 && cnt2 == EOF)
		{
			printf("Wrong answer\nLonger than standard output\n");
			fclose(contestantOutputFile);
			fclose(standardOutputFile);
			return 0;
		}

		if(fabs(a - b) > eps || (isinf(a)^isinf(b)) || (isnan(a)^isnan(b)))
		{
			printf("Wrong answer\nRead %.10lf but expect %.10lf\n", a, b);
			fclose(contestantOutputFile);
			fclose(standardOutputFile);
			return 0;
		}
	}

	printf("Correct answer\n");
	fclose(contestantOutputFile);
	fclose(standardOutputFile);
	return 0;
}
