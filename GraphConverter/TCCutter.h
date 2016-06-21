#pragma once
#include "TypeDef.h"
#include <string>

class TCCutter
{
//	std::string method;
	const int parm;
	tc_t& data;
public:
	//TCCutter() = default;
	TCCutter(tc_t& data, const std::string& method, const int parm);

	bool haveNext() const;
	tc_t getNext();

private:
	int nNode;
	int pos, step, size;

	corr_t cut();
	void movePointer();

	// functions of initializing step and size;
	void init_nGraph();
	void init_nScan();
	//TODO:
	void init_slideWindow();
};

