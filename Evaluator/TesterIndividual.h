#pragma once
#include "TesterBase.h"
class TesterIndividual :
	public TesterBase
{
	enum class Type { FREQ, PROB };
	Type type;
	double thre;
	Motif m;
public:
	static const std::string name;
	static const std::string usage;

	TesterIndividual();
	TesterIndividual(const std::vector<std::string>& params);

	virtual void set(const Motif& m);

	virtual bool testSubject(const SubjectData& sub);

	virtual ~TesterIndividual();
private:
	bool parse(const std::vector<std::string>& params);

	bool _testFreq(const std::vector<Graph>& gs);
	bool _testProb(const std::vector<Graph>& gs);
};

