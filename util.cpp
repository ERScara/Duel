#include "duel.h"
#include <stdlib.h>

int randInt(int low, int high)
{
	int range = high - low;
	int num = rand() % range;
	return (num + low);
}

double randDouble(double low, double high)
{
	double range = high - low;
	double num = range * (double)rand() / (double)RAND_MAX;
	return(num + low);
}

VOID dtrace(TCHAR* strFormat, ...)
{
	int  offset = 0;
	TCHAR strBuf[256];

	va_list ap;

	va_start(ap, strFormat);

	offset = wsprintf(strBuf, TEXT("DUEL: "));
	offset += wvsprintf(strBuf + offset, strFormat, ap);

	OutputDebugString(strBuf);

	va_end(ap);
}