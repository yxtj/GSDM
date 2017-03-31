#pragma once
#include <vector>
#include <string>

struct TCCutterParam
{
	std::string method;
	// M-each use: nEach
	// M-total use: nTotal
	// M-slide use: nEach, nStep
	int nEach = -1;
	int nTotal = -1;
	int nStep = -1;
public:
	static const std::string name;
	static const std::string usage;

	virtual bool parse(const std::vector<std::string>& params);
private:
	bool parseWhole(const std::vector<std::string>& params);
	bool parseEach(const std::vector<std::string>& params);
	bool parseTotal(const std::vector<std::string>& params);
	bool parseSlide(const std::vector<std::string>& params);
	void numberCheck(int required, int given);
};

