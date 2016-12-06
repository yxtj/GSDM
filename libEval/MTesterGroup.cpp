#include "stdafx.h"
#include "MTesterGroup.h"

using namespace std;

const std::string MTesterGroup::name("group");
const std::string MTesterGroup::usage("Test motif groups. Parameters:\n"
	"  any: true if any motif in the group shows up.\n"
	"  some-<portion>: true when no less than <prot> percent motifs show up.\n"
	"  all: true when all the motifs in the group show up."
);


MTesterGroup::MTesterGroup(const std::vector<std::string>& params)
{
	int nParam = params.size();
	checkNumParam(nParam, 1);
//	checkName(name, params[0]);
	if(!parse(params[0]))
		throw invalid_argument("Cannot parse the parameters of " + name);
}

MTesterGroup::MTesterGroup(const std::vector<std::string>& params, const std::vector<std::string>& singleTesterParam)
{
	int nParam = params.size();
	checkNumParam(nParam, 1);
	//	checkName(name, params[0]);
	if(!parse(params[0]))
		throw invalid_argument("Cannot parse the parameters of " + name);
	setParam4Single(singleTesterParam);
}

void MTesterGroup::set(const std::vector<Motif>& ms)
{
	if(!hasSet()) {
		throw invalid_argument("No motif inputted for a group tester.");
	}
	if(ms.empty()) {
		throw invalid_argument("No motif inputted for a group tester.");
	}
	mts.reserve(ms.size());
	for(auto& m : ms) {
		mts.emplace_back(param4Single);
		mts.back().set(m);
	}
	setnmr();
}

void MTesterGroup::set(const std::vector<MTesterSingle>& mts, const std::vector<std::string>& singleTesterParam)
{
	this->mts = mts;
	setnmr();
	if(!singleTesterParam.empty()) {
		setParam4Single(singleTesterParam);
	}
}

void MTesterGroup::set(std::vector<MTesterSingle>&& mts, const std::vector<std::string>& singleTesterParam)
{
	this->mts = move(mts);
	setnmr();
	if(!singleTesterParam.empty()) {
		setParam4Single(singleTesterParam);
	}
}

void MTesterGroup::setParam4Single(const std::vector<std::string>& singleTesterParam)
{
	param4Single = singleTesterParam;
	setMark();
}

bool MTesterGroup::testSubject(const SubjectData& sub) const
{
	int cnt = 0;
	for(auto& mt : mts) {
		if(mt.testSubject(sub)) {
			if(++cnt >= nmr)
				return true;
		}
	}
	return false;
}


bool MTesterGroup::parse(const std::string & method)
{
	if(method == "any") {
		type = Type::ANY;
	} else if(method == "all") {
		type = Type::ALL;
	} else if(method.find("some") == 0) {
		type = Type::SOME;
		pmr = stod(method.substr(method.find('-') + 1));
	} else {
		return false;
	}
	return true;
}

void MTesterGroup::setnmr()
{
	if(type == Type::ANY) {
		nmr = 1;
	} else if(type == Type::ALL) {
		nmr = mts.size();
	} else if(type== Type::SOME) {
		nmr = static_cast<int>(ceil(pmr*mts.size()));
	} 
}
