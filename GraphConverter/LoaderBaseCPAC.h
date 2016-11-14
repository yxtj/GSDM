#pragma once
#include "TCLoader.h"

/* the base class for loading the data from our sorted CPAC data.
*/
class LoaderBaseCPAC :
	public TCLoader
{
public:
	// virtual. wait until the detailed loader
	virtual std::vector<SubjectInfo> loadSubjectsFromDescFile(const std::string& fn, const int nSubject = -1) = 0;

	virtual std::vector<SubjectInfo> pruneSubjectsViaScanFile(std::vector<SubjectInfo>& vldList, const std::string& root);

	// return the name relative to the root tcPath (i.e. option.tcPath)
	virtual std::string getFilePath(const SubjectInfo& sub);
	virtual tc_t loadTimeCourse(const std::string& fn);

};

