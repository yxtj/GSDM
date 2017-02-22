#include "stdafx.h"
#include "GroupGenerator.h"
#include <iterator>

using namespace std;

const std::string GroupGenerator::name("grpGen");
const std::string GroupGenerator::usage(
	"Method used to generate groups.Parameters:\n"
	"  none/no/trivial/single/<empty>: not use group"
	"  comb <k>: combinations of all the motifs.\n"
	"  topk <k>: enumeration of top-1, top-2, ..., until the number given by testGroupSize."
);


GroupGenerator::GroupGenerator(const std::vector<std::string>& param)
{
	if(param.size() > 2) {
		throw invalid_argument("Wrong number of sub-options.");
	}
	const string& method = param[0];
	if(method.empty() || method == "none" || method == "no" 
		|| method == "trivial" || method == "single") {
		fun = &GroupGenerator::gen_single;
		k = 1; // trivial
	} else if(method == "comb") {
		fun = &GroupGenerator::gen_comb;
		k = stoi(param[1]);
	} else if(method == "topk") {
		fun = &GroupGenerator::gen_topk;
		k = stoi(param[1]);
	} else {
		throw invalid_argument("Unsupported method for " + name);
	}

}

std::vector<std::vector<int>> GroupGenerator::generate(const int n)
{
	return (this->*fun)(n);
}

bool GroupGenerator::trivial() const
{
	return k != 1;
}

// ------------ method comb ------------

std::vector<std::vector<int>> GroupGenerator::gen_single(const int n)
{
	std::vector<std::vector<int>> res(n);
	for(int i = 1; i <= n; ++i) {
		res[i].push_back(i - 1);
	}
	return res;
}

std::vector<std::vector<int>> GroupGenerator::gen_comb(const int n)
{
	vector<int> used;
	return _gen_comb_dfs(min(k, n), used, n);
}

std::vector<std::vector<int>> GroupGenerator::_gen_comb_dfs(
	const int p, std::vector<int>& used, const int n)
{
	vector<vector<int>> res;
	if(p <= 0) {
		res.push_back(used);
		return res;
	}
	int end = n - p;
	for(int i = used.empty() ? 0 : used.back() + 1; i <= end; ++i) {
		used.push_back(i);
		auto t = _gen_comb_dfs(p - 1, used, n);
		used.pop_back();
		move(t.begin(), t.end(), back_inserter(res));
	}
	return res;
}

// ------------ method top-k ------------

std::vector<std::vector<int>> GroupGenerator::gen_topk(const int n)
{
	int end = min(n, k);
	std::vector<std::vector<int>> res;
	res.reserve(end);
	vector<int> temp;
	for(int i = 1; i <= end; ++i) {
		temp.push_back(i);
		res.push_back(temp);
	}
	return res;
}

