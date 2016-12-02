#pragma once
#include <string>
#include <vector>
#include "TesterBase.h"
class TesterFactory
{
public:
	TesterFactory() = delete;
	static TesterBase* generate(const std::vector<std::string>& params);
};

