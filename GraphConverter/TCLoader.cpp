#include "stdafx.h"
#include "TCLoader.h"

using namespace std;

std::vector<SubjectInfo> TCLoader::pruneAndAddScanViaScanFile(
	std::vector<SubjectInfo>& vldList, const std::string& root)
{
	return move(vldList);
}

tc_t TCLoader::loadTimeCourse1D(const std::string & fn)
{
	std::vector<std::vector<double>> res;
	ifstream fin(fn);
	if(!fin) {
		//return res;
		throw invalid_argument("Cannot open the file: " + fn);
	}
	string line;
	// there is a header line
	getline(fin, line);
	int nNodes = 0;
	for(size_t p = line.find('\t'); p != string::npos; p = line.find('\t', p + 1)) {
		++nNodes;
	}
	nNodes -= 2;
	while(getline(fin, line)) {
		if(line.empty())
			break;
		vector<double> row;
		row.reserve(nNodes);
		size_t plast, p;
		// first column: File
		p = line.find('\t');
		// second column: Sub-brick
		p = line.find('\t', p + 1);
		// valid data: start from the third column
		plast = p + 1;
		p = line.find('\t', plast);
		if(p == string::npos)
			continue;
		while(p != string::npos) {
			double t = stod(line.substr(plast, p));
			row.push_back(t);
			plast = p + 1;
			p = line.find('\t', plast);
		}
		res.push_back(move(row));
	}
	fin.close();
	return res;
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
