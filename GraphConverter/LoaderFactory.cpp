#include "stdafx.h"
#include "LoaderFactory.h"
#include "LoaderADHD200.h"
#include "LoaderABIDE.h"
#include "LoaderABIDE2.h"
#include "LoaderADNI.h"

using namespace std;

TCLoader * LoaderFactory::generate(const std::string & name)
{
	TCLoader* res = nullptr;
	if(name == "ADHD") {
		return new LoaderADHD200();
	} else if(name == "ABIDE") {
		return new LoaderABIDE();
	} else if(name == "ABIDE2") {
		return new LoaderABIDE2();
	} else if(name=="ADNI") {
		return new LoaderADNI();
	} else {
		throw invalid_argument("Cannot generate data loader for dataset: " + name);
	}
	return res;
}
