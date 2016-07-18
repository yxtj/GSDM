#include "stdafx.h"
#include "StrategyDUG.h"

using namespace std;

const std::string StrategyDUG::name("DUG");
const std::string StrategyDUG::usage("Discriminative Uncertain Graph\n"
	"Usage: " + StrategyDUG::name + " <top k>\n");


bool StrategyDUG::parse(const std::vector<std::string>& param)
{
	try {
		checkParam(param, 1, name);
		k = stoi(param[1]);
	} catch(exception& e) {
		cerr << e.what() << endl;
		return false;
	}
	return true;
}

std::vector<Motif> StrategyDUG::search(const Option & opt,
	const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg)
{
	return std::vector<Motif>();
}

