#pragma once
#include "../../common/Motif.h"
#include <string>
#include <vector>

class MotifEnumerator
{
	int nNode;
public:
	MotifEnumerator();
	MotifEnumerator(const int n);
	virtual ~MotifEnumerator();

	virtual bool parse(const std::vector<std::string>& param);

	// start from the first motif, end when everything is enumerated
	virtual void setStartPoint() = 0;
	virtual void setEndPoint() = 0;
	// start from given motif, end before the given motif
	virtual void setStartPoint(const Motif& m);
	virtual void setEndPoint(const Motif& m);
	// start as the given part, used for the case given total number of shards
	virtual void setStartPoint(const int shardID);
	virtual void setEndPoint(const int shardID);

	virtual bool hasNext()const = 0;
	virtual Motif next() = 0;
	virtual int nEdgeNext() const;
	virtual int nNodeNext() const;

	virtual bool isStable() const;
	virtual bool isParallelizable() const;
	virtual bool isUniqueWhenPara() const;
	virtual bool isIndependentWhenPara() const;
};

