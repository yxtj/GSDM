#include "stdafx.h"
#include "Stat.h"

void Stat::merge(const Stat & other)
{
	nMotifExplored += other.nMotifExplored;
	nMotifGenerated += other.nMotifGenerated;

	nGraphChecked += other.nGraphChecked;
	nSubjectChecked += other.nSubjectChecked;

	nFreqPos += other.nFreqPos;
	nFreqNeg += other.nFreqNeg;

	// network & schedule related
	nMotifSend += other.nMotifSend;
	nMotifRecv += other.nMotifRecv;
	byteSend += other.byteSend;
	byteRecv += other.byteRecv;

	boundSend += other.boundSend;
	topkSend += other.topkSend;

	timeNetwork += other.timeNetwork; //in MS
	timeWait += other.timeWait; //in MS
}
