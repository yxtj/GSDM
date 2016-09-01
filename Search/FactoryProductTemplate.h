#pragma once
#include <vector>
#include <string>

class FactoryProductTemplate
{
public:
	virtual bool parse(const std::vector<std::string>& param) = 0;
	virtual ~FactoryProductTemplate() = default;

	// throw an exception if fails, the first one of param should be the name
	void checkParam(const std::vector<std::string>& param, int reqired, const std::string& name);
	void checkParam(const std::vector<std::string>& param, int reqiredMin, int requiredMax, const std::string& name);
private:
	void checkNumber(int required, size_t given);
	void checkNumber(int requiredMin, int requiredMax, size_t given);
	void checkName(const std::vector<std::string>& param, const std::string& name);
};
