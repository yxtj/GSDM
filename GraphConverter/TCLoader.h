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
	virtual std::vector<Subject> loadValidList(const std::string& fn) = 0;

	virtual tc_t loadTimeCourse(const std::string& fn) = 0;
};

