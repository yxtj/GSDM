#include "stdafx.h"
#include "Subject.h"

Subject::Subject(const std::string & id, const int type, const int scanNum)
	:id(id), type(type), scanNum(scanNum)
{
}

Subject::Subject(std::string && id, const int type, const int scanNum)
	:id(move(id)), type(type), scanNum(scanNum)
{
}
