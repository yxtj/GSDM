#pragma once
#include "TCLoader.h"
#include <string>

class LoaderFactory
{
public:
	static TCLoader* generate(const std::string& name);
};

