#pragma once
#include <vector>
#include <string>

class FactoryProductTemplate
{
public:
	virtual bool parse(const std::vector<std::string>& param) = 0;
	virtual ~FactoryProductTemplate() = default;
	void checkParam(const std::vector<std::string>& param, int reqired, const std::string& name)
		noexcept(false); // throw an exception if fails
private:
	void checkNumber(int required, size_t given);
	void checkName(const std::vector<std::string>& param, const std::string& name);
};
