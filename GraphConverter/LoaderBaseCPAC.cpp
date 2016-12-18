#include "stdafx.h"
#include "LoaderBaseCPAC.h"

using namespace std;

std::vector<SubjectInfo> LoaderBaseCPAC::pruneAndAddScanViaScanFile(
	std::vector<SubjectInfo>& vldList, const std::string & root)
{
	using namespace boost::filesystem;
	vector<SubjectInfo> res;
	res.reserve(vldList.size());
	regex reg("^rest_(\\d+)\\.1D$");
	for(SubjectInfo& s : vldList) {
		path base(root + "/" + s.id);
		if(!exists(base))
			continue;
		for(auto it = directory_iterator(base); it != directory_iterator(); ++it) {
			smatch m;
			string fn = it->path().filename().string();
			regex_search(fn, m, reg);
			int scanNum = stoi(m[1].str());
			res.push_back(s);
			res.back().seqNum = scanNum;
		}
	}
	return res;
}

std::string LoaderBaseCPAC::getFilePath(const SubjectInfo & sub)
{
	return sub.id + "/" + "scan_" + to_string(sub.seqNum) + ".1D";
}

tc_t LoaderBaseCPAC::loadTimeCourse(const std::string & fn)
{
	return loadTimeCourse1D(fn);
}
