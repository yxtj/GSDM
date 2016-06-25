#include "stdafx.h"
#include "ComplexParamBase.h"

using namespace std;

void ComplexParamBase::reg(Option & opt, const std::string& name, const std::string & des)
{
	using boost::program_options::value;
	auto& desc = opt.getDesc();
	desc.add_options()
		(name.c_str(), value<vector<string>>(&param)->multitoken(), des.c_str());
}
