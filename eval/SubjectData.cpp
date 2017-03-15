#include "stdafx.h"
#include "SubjectData.h"


SubjectData::SubjectData(const int type, const std::string & id)
	:type(type), id(id)
{
}

void SubjectData::initGP()
{
	gp.init(get());
}

