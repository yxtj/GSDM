#include "stdafx.h"
#include "CheckerNone.h"


CheckerNone::CheckerNone(const int n)
{
}

bool CheckerNone::needMore() const
{
	return false;
}

bool CheckerNone::result() const
{
	return true;
}

void CheckerNone::input(const bool b)
{
}

void CheckerNone::input()
{
}
