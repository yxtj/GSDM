#pragma once
#include "TypeDef.h"
#include <string>

/************************************************************************/
/* Time Course Loader                                                   */
/************************************************************************/
class TCLoader
{
public:
	virtual std::vector<std::pair<std::string, int>> loadValidList(const std::string& fn) = 0;

	virtual tc_t loadTimeCourse(const std::string& fn) = 0;
};

