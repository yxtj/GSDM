#pragma once
#include "TypeDef.h"
#include "Option.h"
#include "TCCutterParam.h"
#include <string>

class TCCutter
{
	tc_t& data;
public:
	//TCCutter() = default;
	TCCutter(tc_t& data, const TCCutterParam& parm);

	bool haveNext() const;
	tc_t getNext();

private:
	int nNode;
	int pos, step, size;

	corr_t cut();
	void movePointer();

	// functions of initializing step and size;
	void initTotal(const int nTotal);
	void initEach(const int nEach);
	void initSlideWindow(const int winSize, const int step);
};

