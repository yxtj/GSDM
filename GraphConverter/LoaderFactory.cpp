#include "stdafx.h"
#include "LoaderFactory.h"
#include "LoaderADHD200.h"

using namespace std;

TCLoader * LoaderFactory::generate(const std::string & name)
{
	TCLoader* res = nullptr;
	if(name == "ADHD") {
		return new LoaderADHD200();
	} else {
		throw invalid_argument("Cannot generate data loader for dataset: " + name);
	}
	return res;
}
