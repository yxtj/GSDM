#include "stdafx.h"
#include "TCCutter.h"

using namespace std;


TCCutter::TCCutter(tc_t & data, const TCCutterParam& parm)
	: data(data)
{
	if("total" == parm.method)
		initTotal(parm.nTotal);
	else if("each" == parm.method)
		initEach(parm.nEach);
	else if("slide" == parm.method)
		initSlideWindow(parm.nEach, parm.nStep);
	else {
		throw invalid_argument("given cutting method is not given or not supported");
	}
	nNode = data.empty() ? 0 : data.front().size();
}

bool TCCutter::haveNext() const
{
	return static_cast<size_t>(pos + size) <= data.size();
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
	auto itend = static_cast<size_t>(pos + size) >= data.size()
		? data.end() : data.begin() + (pos + size);
	if(reuse) {
		copy(data.begin() + pos, itend, back_inserter(res));
	} else {
		copy(make_move_iterator(data.begin() + pos), make_move_iterator(itend), back_inserter(res));
	}
	return res;
}

void TCCutter::movePointer()
{
	pos += step;
}

void TCCutter::initTotal(const int nTotal)
{
	pos = 0;
	step = data.size() / nTotal;
	size = step;
	reuse = false;
}

void TCCutter::initEach(const int nEach)
{
	pos = 0;
	step = nEach;
	size = nEach;
	reuse = false;
}

void TCCutter::initSlideWindow(const int winSize, const int step)
{
	pos = 0;
	size = winSize;
	this->step = step;
	reuse = true;
}
