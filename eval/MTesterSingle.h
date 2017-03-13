#pragma once
#include "MTesterBase.h"
class MTesterSingle :
	public MTesterBase
{
	enum class Type { FREQ, PROB };
	Type type;
	double thre;
	const Motif* pm;
public:
	static const std::string name;
	static const std::string usage;

//	MTesterSingle() = default;
	MTesterSingle(const std::vector<std::string>& params);
	MTesterSingle(const std::string& method, const double threshold);

	virtual void set(const Motif& m);

	virtual bool testSubject(const SubjectData& sub) const;

	virtual ~MTesterSingle() = default;
private:
	bool parse(const std::string& method, const double threshold);

	bool _testFreq(const std::vector<Graph>& gs) const;
	bool _testProb(const GraphProb& gp) const;
};

