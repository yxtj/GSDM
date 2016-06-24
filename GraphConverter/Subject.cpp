#include "stdafx.h"
#include "Subject.h"

Subject::Subject(const std::string & id, const int type, const int sgId)
	:id(id), type(type), sgId(sgId)
{
}

Subject::Subject(std::string && id, const int type, const int sgId)
	:id(move(id)), type(type), sgId(sgId)
{
}
