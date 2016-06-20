#include "stdafx.h"
#include "Subject.h"

Subject::Subject(const std::string & id, const int type)
	:id(id), type(type)
{
}

Subject::Subject(std::string && id, const int type)
	:id(move(id)), type(type)
{
}
