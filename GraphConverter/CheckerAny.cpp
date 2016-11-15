#include "stdafx.h"
#include "CheckerAny.h"


CheckerAny::CheckerAny(const int n)
	:total(n)
{
	reset();
}

void CheckerAny::reset()
{
	curr = 0;
	res = false;
}

bool CheckerAny::needMore() const
{
	return res!=true && curr<total;
}

bool CheckerAny::result() const
{
	return res;
}

void CheckerAny::input(const bool b)
{
	res = res || b;
	++curr;
}

void CheckerAny::input()
{
	++curr;
}
