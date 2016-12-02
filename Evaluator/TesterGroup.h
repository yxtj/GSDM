#pragma once
#include "TesterBase.h"
#include "TesterIndividual.h"

class TesterGroup :
	public TesterBase
{
	std::vector<TesterIndividual> mts;
	enum class Type { ANY, SOME, ALL };
	Type type;
	double pmr;
	int nmr; // minimum number of motifs required to be positive

	std::vector<std::string> paramForInd;
public:
	static const std::string name;
	static const std::string usage;

	TesterGroup();
	TesterGroup(const std::vector<std::string>& params);

	virtual void set(const std::vector<Motif>& ms);

	virtual bool testSubject(const SubjectData& sub);

	~TesterGroup();
private:
	bool parse(const std::vector<std::string>& params);

};

