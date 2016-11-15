#pragma once
#include "QCChecker.h"
class CheckerAll :
	public QCChecker
{
	int total;
	int curr;
	bool res;
public:
	CheckerAll(const int n);
	virtual bool needMore() const;
	virtual bool result() const;

	virtual void input(const bool b);
	virtual void input();

};

