#include "stdafx.h"
#include "SubjectInfo.h"

SubjectInfo::SubjectInfo(const std::string & id, const int type, const int sgId)
	:id(id), type(type), sgId(sgId)
{
}

SubjectInfo::SubjectInfo(std::string && id, const int type, const int sgId)
	:id(move(id)), type(type), sgId(sgId)
{
}
