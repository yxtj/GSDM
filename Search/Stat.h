#pragma once
#include <vector>
#include <utility>
#include "../serialization/serializer_basic.hpp"

struct Stat
{
	// search realted
	unsigned long long nMotifExplored;
	unsigned long long nMotifGenerated;

	unsigned long long nGraphChecked;
	unsigned long long nSubjectChecked;

	unsigned long long nFreqPos;
	unsigned long long nFreqNeg;

	// network & schedule related
	unsigned long long nMotifSend;
	unsigned long long nMotifRecv;
	unsigned long long byteSend;
	unsigned long long byteRecv;

	unsigned long long boundSend;
	unsigned long long topkSend;

	unsigned long long timeNetwork; //in MS
	unsigned long long timeWait; //in MS

	// progress
	std::vector<std::pair<unsigned long long, double>> progBound;
	std::vector<std::pair<unsigned long long, double>> progCESize;
// functions:
	void merge(const Stat& other);
};

template <>
struct _Serializer<Stat> {
	static constexpr int fixed = sizeof(unsigned long long) * (6 + 6 + 1 + 1);
	_Serializer<std::vector<std::pair<unsigned long long, double>>> sp;
	int estimateSize(const Stat& item) {
		return fixed + sizeof(std::pair<unsigned long long, double>)*(
			item.progBound.size() + item.progCESize.size());
	}
	char* serial(char* res, int bufSize, const Stat& item) {
		if(bufSize < estimateSize(item))
			return nullptr;
		return serial(res, item);
	}
	char* serial(char* res, const Stat& item) {
		unsigned long long* p = reinterpret_cast<unsigned long long*>(res);
		*p++ = item.nMotifExplored;
		*p++ = item.nMotifGenerated;
		*p++ = item.nGraphChecked;
		*p++ = item.nSubjectChecked;
		*p++ = item.nFreqPos;
		*p++ = item.nFreqNeg;
		// network & schedule related
		*p++ = item.nMotifSend;
		*p++ = item.nMotifRecv;
		*p++ = item.byteSend;
		*p++ = item.byteRecv;
		*p++ = item.boundSend;
		*p++ = item.topkSend;
		// time
		*p++ = item.timeNetwork;
		*p++ = item.timeWait;
		res= reinterpret_cast<char*>(p);
		res = sp.serial(res, item.progBound);
		res = sp.serial(res, item.progCESize);
		return res;
	}
	std::pair<Stat, const char*> deserial(const char* p) {
		Stat item;
		const unsigned long long* up = reinterpret_cast<const unsigned long long*>(p);
		item.nMotifExplored = *up++;
		item.nMotifGenerated = *up++;
		item.nGraphChecked = *up++;
		item.nSubjectChecked = *up++;
		item.nFreqPos = *up++;
		item.nFreqNeg = *up++;
		// network & schedule related
		item.nMotifSend = *up++;
		item.nMotifRecv = *up++;
		item.byteSend = *up++;
		item.byteRecv = *up++;
		item.boundSend = *up++;
		item.topkSend = *up++;
		// time
		item.timeNetwork = *up++;
		item.timeWait = *up++;
		p = reinterpret_cast<const char*>(up);
		std::tie(item.progBound, p) = std::move(sp.deserial(p));
		std::tie(item.progCESize, p) = std::move(sp.deserial(p));
		return std::make_pair(std::move(item), p);
	}
};
