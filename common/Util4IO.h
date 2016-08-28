#pragma once
#include <string>
#include "SubjectInfo.h"
class Util4IO
{
public:
	// filename format: <type>-<subject Id>-<scan Id>.txt
	static bool checkFilename2Subject(const std::string & fn);
	static SubjectInfo parseFilename2Subject(const std::string& fn) noexcept(false);
	static bool checknParseFilename2Subject(const std::string& fn, SubjectInfo* pRes) noexcept;

};

