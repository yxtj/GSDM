#pragma once
#include "MTesterBase.h"
#include "MTesterSingle.h"

class MTesterGroup :
	public MTesterBase
{
	std::vector<MTesterSingle> mts;
	enum class Type { ANY, SOME, ALL };
	Type type;
	double pmr;
	int nmr; // minimum number of motifs required to be positive

	std::vector<std::string> param4Single;
public:
	static const std::string name;
	static const std::string usage;

//	MTesterGroup() = default;
	MTesterGroup(const std::vector<std::string>& params);
	MTesterGroup(const std::vector<std::string>& params, const std::vector<std::string>& singleTesterParam);

	virtual void set(const std::vector<Motif>& ms);
	void setParam4Single(const std::vector<std::string>& singleTesterParam);

	void set(const std::vector<MTesterSingle>& mts, const std::vector<std::string>& singleTesterParam = {});
	void set(std::vector<MTesterSingle>&& mts, const std::vector<std::string>& singleTesterParam = {});

	virtual bool testSubject(const SubjectData& sub) const;

private:
	bool parse(const std::string& method);
	void setnmr();
};

