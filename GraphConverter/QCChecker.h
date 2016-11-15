#pragma once
/*
Checker for the Quality Control information of phenotypic files
*/
class QCChecker
{
public:
	QCChecker() = default;
	QCChecker(const int n);
	virtual bool needMore() const = 0;
	virtual bool result() const = 0;

	virtual void input(const bool b) = 0;
	virtual void input() = 0;
};

