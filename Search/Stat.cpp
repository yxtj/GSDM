#include "stdafx.h"
#include "Stat.h"

void Stat::merge(const Stat & other)
{
	nEdgeChecked += other.nEdgeChecked;
	nMotifExplored += other.nMotifExplored;
	nMotifGenerated += other.nMotifGenerated;

	nGraphChecked += other.nGraphChecked;
	nSubjectChecked += other.nSubjectChecked;

	nFreqPos += other.nFreqPos;
	nFreqNeg += other.nFreqNeg;

	// network & schedule related
	nMotifSend += other.nMotifSend;
	nMotifRecv += other.nMotifRecv;
	netByteSend += other.netByteSend;
	netByteRecv += other.netByteRecv;

	boundSend += other.boundSend;
	topkSend += other.topkSend;

	timeTotal += other.timeTotal;
	timeSearch += other.timeSearch;
	timeWait += other.timeWait;
	timeData+= other.timeData;
}

void Stat::average(const int n)
{
	if(n <= 1)
		return;
	nEdgeChecked /= n;
	nMotifExplored /= n;
	nMotifGenerated /= n;

	nGraphChecked /= n;
	nSubjectChecked /= n;

	nFreqPos /= n;
	nFreqNeg /= n;

	// network & schedule related
	nMotifSend /= n;
	nMotifRecv /= n;
	netByteSend /= n;
	netByteRecv /= n;

	boundSend /= n;
	topkSend /= n;

	timeTotal /= n;
	timeSearch /= n;
	timeWait /= n;
	timeData /= n;
}
