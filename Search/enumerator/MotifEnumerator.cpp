#include "MotifEnumerator.h"



MotifEnumerator::MotifEnumerator()
{
}

MotifEnumerator::MotifEnumerator(const int n)
	:nNode(n)
{
}


MotifEnumerator::~MotifEnumerator()
{
}

bool MotifEnumerator::parse(const std::vector<std::string>& param)
{
	return true;
}

void MotifEnumerator::setStartPoint(const Motif & m)
{
}

void MotifEnumerator::setEndPoint(const Motif & m)
{
}

void MotifEnumerator::setStartPoint(const int shardID)
{
}

void MotifEnumerator::setEndPoint(const int shardID)
{
}

int MotifEnumerator::nEdgeNext() const
{
	return 0;
}

int MotifEnumerator::nNodeNext() const
{
	return 0;
}

bool MotifEnumerator::isStable() const
{
	return false;
}

bool MotifEnumerator::isParallelizable() const
{
	return false;
}

bool MotifEnumerator::isUniqueWhenPara() const
{
	return false;
}

bool MotifEnumerator::isIndependentWhenPara() const
{
	return false;
}
