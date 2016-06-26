#include "stdafx.h"
#include "ComplexParamBase.h"
#include "Option.h"
#include <functional>

using namespace std;

void ComplexParamBase::reg(Option & opt, const std::string& name, const std::string & des)
{
	this->name = name;

	using boost::program_options::value;
	auto& desc = opt.getDesc();
	desc.add_options()
		(name.c_str(), value<vector<string>>(&param)->multitoken(), des.c_str());

	function<bool()> fun = [this]() { return this->parse(); };
	opt.addParser(fun);
}
