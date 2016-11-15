#include "stdafx.h"
#include "CheckerFactory.h"
#include "CheckerAll.h"
#include "CheckerAny.h"
#include "CheckerNone.h"

QCChecker * CheckerFactory::generate(const std::string & name, const int n)
{
	if(name == "none")
		return new CheckerNone(n);
	else if(name == "any")
		return new CheckerAny(n);
	else if(name == "all")
		return new CheckerAll(n);
	else
		throw std::invalid_argument("Cannot generate QC Checker with name: " + name);
	return nullptr;
}
