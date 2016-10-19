#pragma once
#include "../../common/Motif.h"

class MotifEnumerator
{
	int nNode;
public:
	MotifEnumerator();
	MotifEnumerator(const int n);
	virtual ~MotifEnumerator();

	virtual bool hasNext()const = 0;
	virtual Motif next() = 0;
	virtual int nEdgeNext() const;
	virtual int nNodeNext() const;
};

