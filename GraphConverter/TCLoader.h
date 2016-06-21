#pragma once
#include "TypeDef.h"
#include "Subject.h"
#include <string>

/************************************************************************/
/* Time Course Loader                                                   */
/************************************************************************/
class TCLoader
{
public:
	// read a description file and load all valid subjects
	// input: the path of description file or the folder of that file(s)
	// post-condition: subject.id and subject.type is valid, subject.scanNum is undefined
	virtual std::vector<Subject> loadValidList(const std::string& fn) = 0;

	// post-condition: <id, type, scan #>
	// useful for the dataset where scan # is not included in the description file
	// default version: return the input
	virtual std::vector<Subject> getAllSubjects(std::vector<Subject>& vldList, const std::string& root);

	// return the name relative to the root tcPath (i.e. option.tcPath)
	virtual std::string getFilePath(const Subject& sub) = 0;
	virtual tc_t loadTimeCourse(const std::string& fn) = 0;
};

