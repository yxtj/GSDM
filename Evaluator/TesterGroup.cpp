#include "stdafx.h"
#include "TesterGroup.h"

using namespace std;

const std::string TesterGroup::name("group");
const std::string TesterGroup::usage("Test motif group. use \"freq <th>\" for individual testing. Parameters:\n"
	"  any <th>: true if any motif in the group shows up.\n"
	"  some <port> <th>: true when no less than <prot> percent motifs show up.\n"
	"  all <th>: true when all the motifs in the group show up."
);



TesterGroup::TesterGroup()
{
}

TesterGroup::TesterGroup(const std::vector<std::string>& params)
{
	int nParam = params.size();
	checkNumParam(nParam, 2, 3);
	checkName(name, params[0]);
	if(!parse(params))
		throw invalid_argument("Cannot parse the parameters of " + name);
}

void TesterGroup::set(const std::vector<Motif>& ms)
{
	if(ms.empty()) {
		throw invalid_argument("No motif inputted for a group tester.");
	}
	mts.reserve(ms.size());
	for(auto& m : ms) {
		mts.emplace_back(paramForInd);
		mts.back().set(m);
	}
	nmr = static_cast<int>(ceil(pmr*ms.size()));
	setMark();
}

bool TesterGroup::testSubject(const SubjectData& sub)
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


TesterGroup::~TesterGroup()
{
}

bool TesterGroup::parse(const std::vector<std::string>& params)
{
	if(params[1] == "any") {
		type = Type::ANY;
		paramForInd = { "freq", params[2] };
	} else if(params[1] == "some") {
		type = Type::SOME;
		pmr = stod(params[2]);
		paramForInd = { "freq", params[3] };
	} else if(params[1] == "all") {
		type = Type::ALL;
		paramForInd = { "freq", params[2] };
	} else {
		return false;
	}
	return true;
}
