#pragma once
#include "TypeDef.h"
#include "../common/SubjectInfo.h"
#include <string>

/************************************************************************/
/* Time Course Loader                                                   */
/************************************************************************/
class TCLoader
{
public:
	// read a description file and load all valid subjects
	// input: the path of description file or the folder of that file(s), maximum number of subjects (non-positive means all)
	// post-condition: subject.id and subject.type is valid, subject.scanNum is undefined
	virtual std::vector<SubjectInfo> loadSubjectsFromDescFile(
		const std::string& fn, const std::string& qcMethod, const int nSubject = -1) = 0;

	// post-condition: <id, type, scan #>
	// useful for the dataset where scan # is not included in the description file
	// default version: return the input
	virtual std::vector<SubjectInfo> pruneAndAddScanViaScanFile(std::vector<SubjectInfo>& vldList, const std::string& root);

	// return the name relative to the root tcPath (i.e. option.tcPath)
	virtual std::string getFilePath(const SubjectInfo& sub) = 0;
	virtual tc_t loadTimeCourse(const std::string& fn) = 0;

	tc_t loadTimeCourse1D(const std::string& fn);
public:
	std::string padID2Head(std::string& id, const int nDig, const char PAD);
	std::string padID2Tail(std::string& id, const int nDig, const char PAD);
};

