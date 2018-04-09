#include "stdafx.h"
#include "IOfunctions.h"
#include "LoaderFactory.h"
#include "TCCutter.h"

using namespace std;

//---------------------------- Time Course ---------------------------------

std::multimap<SubjectInfo, tc_t> loadInputTC(
	const std::string& phenoPath, const std::string& tcPath, const std::string& qcMethod,
	const std::string& dataset, const int nSubject, const int nSkip)
{
	using namespace boost::filesystem;
	path root(tcPath);
	if(!is_directory(root)) {
		throw invalid_argument("Given time course path is invalid");
	}

	TCLoader* loader = LoaderFactory::generate(dataset);
	multimap<SubjectInfo, tc_t> res;

	vector<SubjectInfo> slist;
	{
		// TODO: put the nSkip into loadSubjectsFromDescFile: nSubject is the # scanned, not guaranteed output #
		vector<SubjectInfo> validList = loader->loadSubjectsFromDescFile(phenoPath, qcMethod, nSubject, nSkip);
		slist = loader->pruneAndAddScanViaScanFile(validList, tcPath);
	}
// 	if(nSubject > 0 && slist.size() > static_cast<size_t>(nSubject)) {
// 		auto it = slist.begin() + nSubject;
// 		slist.erase(it, slist.end());
// 	}
	for(SubjectInfo& s : slist) {
		string fn = loader->getFilePath(s);
		try{
			tc_t tc = loader->loadTimeCourse(tcPath + fn);
			res.emplace(move(s), move(tc));
		}catch(exception& e){
			cerr << "skip: \"" << fn << "\" because:\n\t" << e.what() << endl;
		}
	}
	delete loader;

	return res;
}

//---------------------------- Correlation ---------------------------------

std::string genCorrFilename(const SubjectInfo & sub)
{
	return sub.genFilename();
}

bool checkCorrFilename(const string & fn)
{
	return SubjectInfo::checkFilename(fn);
}

SubjectInfo parseCorrFilename(const std::string & fn) noexcept(false)
{
	return SubjectInfo(fn);
}

bool checknParseCorrFilename(const std::string& fn, SubjectInfo* pRes)
{
	if(SubjectInfo::checkFilename(fn)) {
		if(pRes)
			pRes->parseFromFilename(fn);
		return true;
	}
	return false;
}

std::multimap<SubjectInfo, corr_t> loadInputCorr(const std::string& corrPath, const int nSubject, const int nSkip)
{
	using namespace boost::filesystem;
	path root(corrPath);
	if(!is_directory(root)) {
		throw invalid_argument("Given correlation path is invalid");
	}
	
	size_t limit = nSubject >= 0 ? nSubject : numeric_limits<size_t>::max();
	std::multimap<SubjectInfo, corr_t> res;
	int count = 0;
	for(auto it = directory_iterator(root); it != directory_iterator(); ++it) {
		if(++count <= nSkip)
			continue;
		string fn = it->path().filename().string();
		SubjectInfo sub;
		if(is_regular_file(*it) && checknParseCorrFilename(fn, &sub)) { 
			res.emplace(move(sub), readCorr(corrPath + fn));
		}
		if(res.size() >= limit)
			break;
	}
	return res;
}

corr_t readCorr(const std::string & fn)
{
	ifstream fin(fn);
	if(!fin) {
		throw invalid_argument("cannot open correlation file " + fn);
	}
//	int n;
// 	fin >> n;
// 	corr_t res;
// 	res.reserve(n);
	//TODO: change to parsing lines, to speed up.
// 	for(int i = 0; i < n; ++i) {
// 		vector<double> temp(n);
// 		for(int j = 0; j < n; ++j) {
// 			fin >> temp[j];
// 		}
// 		res.push_back(move(temp));
// 	}
//	return res;
	string line;
	getline(fin, line);
	int n = stoi(line);
	corr_t res;
	res.reserve(n);
	for(int i = 0; i < n; ++i) {
		getline(fin, line);
		vector<double> temp;
		temp.reserve(n);
		size_t plast = 0, p = line.find(' ');
		while(p != string::npos) {
			temp.push_back(stod(line.substr(plast, p - plast)));
			plast = p + 1;
			p = line.find(' ', plast);
		}
		res.push_back(move(temp));
	}
	return res;
}

void writeCorr(ostream& os, const corr_t& corr)
{
	os << corr.size() << "\n";
	for(auto& line : corr) {
		for(auto& v : line) {
			os << v << ' ';
		}
		os << "\n";
	}
}

//---------------------------- Graph ---------------------------------

std::string genGraphFilename(const SubjectInfo & sub)
{
	return sub.genFilename();
}

bool checknParseGraphFilename(const std::string & fn, SubjectInfo * pRes)
{
	return checknParseCorrFilename(fn, pRes);
}

