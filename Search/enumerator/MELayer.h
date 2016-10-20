#pragma once
#include "MotifEnumerator.h"
struct _MELayerData;
class MELayer :
	public MotifEnumerator
{
	_MELayerData* data;
public:
	MELayer();
	virtual ~MELayer();
	virtual bool parse(const std::vector<std::string>& param);

	// start from the first motif, end when everything is enumerated
	virtual void setStartPoint();
	virtual void setEndPoint();
	// start from given motif, end before the given motif
	virtual void setStartPoint(const Motif& m);
	virtual void setEndPoint(const Motif& m);
	// start as the given part, used for the case given total number of shards
	virtual void setStartPoint(const int shardID);
	virtual void setEndPoint(const int shardID);

	virtual bool hasNext()const;
	virtual Motif next();
	virtual int nEdgeNext() const;
	virtual int nNodeNext() const;

	virtual bool isStable() const;
};

