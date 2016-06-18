#include "stdafx.h"
#include "TCCutter.h"

using namespace std;

TCCutter::TCCutter(tc_t & data, const std::string & method, const int parm)
	: data(data), method(method), parm(parm)
{
	if("nGraph" == method)
		init_nGraph();
	else if("nScan" == method)
		init_nScan();
	else if("slide" == method)
		init_slideWindow();
	else {
		throw invalid_argument("given cutting method is not supported");
	}
	nNode = data.empty() ? 0 : data.front().size();
}

bool TCCutter::haveNext() const
{
	return pos + size < data.size();
}

corr_t TCCutter::getNext()
{
	corr_t res = cut();
	movePointer();
	return res;
}

tc_t TCCutter::cut()
{
	tc_t res;
	res.reserve(size);
	auto itend = pos + size >= data.size() ? data.end() : data.begin() + size;
	copy(make_move_iterator(data.begin() + pos), make_move_iterator(itend), back_inserter(res));
	return res;
}

void TCCutter::movePointer()
{
	pos += step;
}

void TCCutter::init_nGraph()
{
	pos = 0;
	step = data.size() / parm;
	size = step;
}

void TCCutter::init_nScan()
{
	pos = 0;
	size = parm;
	step = data.size() / size;
}

void TCCutter::init_slideWindow()
{
	init_nScan();
}
