#pragma once
#include "QCChecker.h"
class CheckerNone :
	public QCChecker
{
public:
	CheckerNone(const int n);
	virtual bool needMore() const;
	virtual bool result() const;

	virtual void input(const bool b);
	virtual void input();
};

