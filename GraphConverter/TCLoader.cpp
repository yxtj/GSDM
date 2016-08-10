#include "stdafx.h"
#include "TCLoader.h"

using namespace std;

std::vector<Subject> TCLoader::getAllSubjects(
	std::vector<Subject>& vldList, const std::string& root)
{
	return move(vldList);
}

std::string TCLoader::padID2Head(std::string & id, const int nDig, const char PAD)
{
	if(id.size() == nDig)
		return id;
	string temp(nDig - id.size(), PAD);
	return temp + id;
}

std::string TCLoader::padID2Tail(std::string & id, const int nDig, const char PAD)
{
	if(id.size() == nDig)
		return id;
	string temp(nDig - id.size(), PAD);
	return id + temp;
}
