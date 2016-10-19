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

int MotifEnumerator::nEdgeNext() const
{
	return 0;
}

int MotifEnumerator::nNodeNext() const
{
	return 0;
}
