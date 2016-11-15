#pragma once
#include "QCChecker.h"
#include <string>

class CheckerFactory
{
public:
	CheckerFactory() = delete;
	static QCChecker* generate(const std::string& name, const int n);
};

