#include "stdafx.h"
#include "IOfunctions.h"
#include "LoaderFactory.h"
#include "TCCutter.h"

using namespace std;


std::string genCorrFilename(const Subject & sub, const int scanId)
{
	return to_string(sub.type) + "-" + sub.id + "-" + to_string(scanId) + ".txt";
}

bool checkCorrFilename(const string & fn)
{
	if(fn.empty())
		return false;
	size_t p1 = fn.find('-');
	size_t p2 = fn.find('-', p1 + 1);
	size_t pend = fn.rfind(".txt", string::npos, 4);
	return p1 != 0 && p1 != string::npos && p2 != p1 && p2 != string::npos && pend != string::npos;
}

std::pair<Subject, int> getInfoFromCorrFilename(const std::string & fn)
{
	if(fn.empty())
		throw invalid_argument("filename '" + fn + "' is not a valid correlation file name");
	size_t p1 = fn.find('-');
	size_t p2 = fn.find('-', p1 + 1);
	size_t pend = fn.rfind(".txt", string::npos, 4);
	Subject s{ fn.substr(0, p1), stoi(fn.substr(p1 + 1, p2)) };
	return make_pair(move(s), stoi(fn.substr(p2 + 1, pend)));
}

std::multimap<Subject, tc_t> loadInputTC(const Option & opt)
{
	using namespace boost::filesystem;
	path root(opt.tcPath);
	if(!is_directory(root)) {
		throw invalid_argument("Given time course path is invalid");
	}

	TCLoader* loader = LoaderFactory::generate(opt.dataset);
	multimap<Subject, tc_t> res;

	vector<Subject> validList = loader->loadValidList(opt.tcPath);
	// get all sub-folders:
	vector<path> ids;
	for(auto it = directory_iterator(root); it != directory_iterator(); ++it) {
		if(is_directory(*it))
			ids.push_back(*it);
	}

	for(auto& id : ids) {
		string leaf = id.filename().string();
		//TODO: generalize to other dataset
		//TODO: some folder may have several files
		tc_t tc = loader->loadTimeCourse(opt.tcPath + leaf + "/sfnwmrda"
			+ leaf + "_session_1_rest_1_aal_TCs.1D");
		//TODO: add cutter here
		res.emplace(move(validList[0]), move(tc));
	}
	delete loader;

	return res;
}

std::multimap<Subject, corr_t> loadInputCorr(const Option& opt)
{
	using namespace boost::filesystem;
	path root(opt.corrPath);
	if(!is_directory(root)) {
		throw invalid_argument("Given correlation path is invalid");
	}

	std::multimap<Subject, corr_t> res;
	for(auto it = directory_iterator(root); it != directory_iterator(); ++it) {
		string fn = it->path().filename().string();
		if(is_regular_file(*it) && checkCorrFilename(fn)) {
			pair<Subject, int> p = getInfoFromCorrFilename(fn);
			res.emplace(move(p.first), _loadCorr(fn));
		}
	}
	return res;
}

corr_t _loadCorr(const std::string & fn)
{
	ifstream fin(fn);
	if(!fin) {
		throw invalid_argument("cannot open correlation file " + fn);
	}
	int n;
	fin >> n;
	corr_t res;
	res.reserve(n);
	for(int i = 0; i < n; ++i) {
		vector<double> temp(n);
		//TODO: change to parsing a line, to speed up.
		for(int j = 0; j < n; ++j) {
			fin >> temp[j];
		}
		res.push_back(move(temp));
	}
	return res;
}

