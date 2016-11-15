#include "stdafx.h"
#include "CheckerAll.h"


CheckerAll::CheckerAll(const int n)
	: total(n), curr(0), res(true)
{
}

bool CheckerAll::needMore() const
{
	return res==true && curr<total;
}

bool CheckerAll::result() const
{
	return res;
}

void CheckerAll::input(const bool b)
{
	res = res&&b;
	++curr;
}

void CheckerAll::input()
{
	res = false;
	++curr;
}

