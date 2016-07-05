#pragma once
#include "CandidateMethod.h"
#include "FactoryTemplate.h"
#include <string>

/************************************************************************/
/* extension:
	step 1, add a name for the new strategy in its own definition
	step 2, register the new strategy into this factory at init()
*/
/************************************************************************/ 

class CandidateMethodFactory
	: public FactoryTemplate
{
public:
	static void init();

	static CandidateMethod* generate(const std::string& name);
};
