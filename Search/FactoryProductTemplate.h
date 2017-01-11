#pragma once
#include <vector>
#include <string>

class FactoryProductTemplate
{
public:
	virtual bool parse(const std::vector<std::string>& param) = 0;
	virtual ~FactoryProductTemplate() = default;

	// throw an exception if fails, the first one of param should be the name
	void checkParam(const std::vector<std::string>& param, size_t reqired, const std::string& name);
	void checkParam(const std::vector<std::string>& param, size_t reqiredMin, size_t requiredMax, const std::string& name);
private:
	void checkNumber(size_t required, size_t given);
	void checkNumber(size_t requiredMin, size_t requiredMax, size_t given);
	void checkName(const std::vector<std::string>& param, const std::string& name);
};
