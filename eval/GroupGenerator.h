#pragma once
#include <string>
#include <vector>
// generate group indexes using given method
class GroupGenerator
{
	using fun_t = std::vector<std::vector<int>>(GroupGenerator::*)(const int n);
	fun_t fun;
	int k;
public:
	static const std::string name;
	static const std::string usage;
	GroupGenerator(const std::vector<std::string>& param);

	std::vector<std::vector<int>> generate(const int n);
	bool trivial() const;
private:
	std::vector<std::vector<int>> gen_single(const int n);
	std::vector<std::vector<int>> gen_comb(const int n);
	std::vector<std::vector<int>> gen_topk(const int n);
private:
	std::vector<std::vector<int>> _gen_comb_dfs(const int p, std::vector<int>& used, const int n);
};

