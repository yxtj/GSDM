#pragma once
#include <vector>
#include <utility>
#include "../serialization/serializer_basic.hpp"

struct Stat
{
	// search realted
	unsigned long long nEdgeChecked;
	unsigned long long nMotifExplored;
	unsigned long long nMotifGenerated;

	unsigned long long nGraphChecked;
	unsigned long long nSubjectChecked;

	unsigned long long nFreqPos;
	unsigned long long nFreqNeg;

	// network & schedule related
	unsigned long long netByteSend;
	unsigned long long netByteRecv;

	unsigned long long nMotifSend;
	unsigned long long nMotifRecv;
	unsigned long long boundSend;
	unsigned long long topkSend;

	// time
	unsigned long long timeTotal; //in MS
	unsigned long long timeWait; //in MS
	unsigned long long timeData; //in MS

	// progress
	std::vector<std::pair<unsigned long long, double>> progBound;
	std::vector<std::pair<unsigned long long, int>> progCESize;
// functions:
	void merge(const Stat& other);
	void average(const int n);
};

template <>
struct _Serializer<Stat> {
	static constexpr int fixed = sizeof(unsigned long long) * (7 + 6 + 3);
	_Serializer<std::vector<std::pair<unsigned long long, double>>> sd;
	_Serializer<std::vector<std::pair<unsigned long long, int>>> si;
	int estimateSize(const Stat& item) {
		return fixed + sd.estimateSize(item.progBound) + si.estimateSize(item.progCESize);
	}
	char* serial(char* res, int bufSize, const Stat& item) {
		if(bufSize < estimateSize(item))
			return nullptr;
		return serial(res, item);
	}
	char* serial(char* res, const Stat& item) {
		unsigned long long* p = reinterpret_cast<unsigned long long*>(res);
		*p++ = item.nEdgeChecked;
		*p++ = item.nMotifExplored;
		*p++ = item.nMotifGenerated;
		*p++ = item.nGraphChecked;
		*p++ = item.nSubjectChecked;
		*p++ = item.nFreqPos;
		*p++ = item.nFreqNeg;
		// network & schedule related
		*p++ = item.netByteSend;
		*p++ = item.netByteRecv;
		*p++ = item.nMotifSend;
		*p++ = item.nMotifRecv;
		*p++ = item.boundSend;
		*p++ = item.topkSend;
		// time
		*p++ = item.timeTotal;
		*p++ = item.timeWait;
		*p++ = item.timeData;
		res= reinterpret_cast<char*>(p);
		res = sd.serial(res, item.progBound);
		res = si.serial(res, item.progCESize);
		return res;
	}
	std::pair<Stat, const char*> deserial(const char* p) {
		Stat item;
		const unsigned long long* up = reinterpret_cast<const unsigned long long*>(p);
		item.nEdgeChecked = *up++;
		item.nMotifExplored = *up++;
		item.nMotifGenerated = *up++;
		item.nGraphChecked = *up++;
		item.nSubjectChecked = *up++;
		item.nFreqPos = *up++;
		item.nFreqNeg = *up++;
		// network & schedule related
		item.netByteSend = *up++;
		item.netByteRecv = *up++;
		item.nMotifSend = *up++;
		item.nMotifRecv = *up++;
		item.boundSend = *up++;
		item.topkSend = *up++;
		// time
		item.timeTotal = *up++;
		item.timeWait = *up++;
		item.timeData = *up++;
		p = reinterpret_cast<const char*>(up);
		std::tie(item.progBound, p) = std::move(sd.deserial(p));
		std::tie(item.progCESize, p) = std::move(si.deserial(p));
		return std::make_pair(std::move(item), p);
	}
};
