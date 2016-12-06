#pragma once
#include <string>
#include <vector>
#include "MTesterBase.h"
class MTesterFactory
{
public:
	MTesterFactory() = delete;
	static MTesterBase* generate(const std::vector<std::string>& params);

	static std::string getName();
	static std::string getUsage();
};

