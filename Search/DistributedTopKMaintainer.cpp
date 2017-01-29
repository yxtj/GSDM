#include "stdafx.h"
#include "DistributedTopKMaintainer.h"

using namespace std;

void DistributedTopKMaintainer::init(const int k)
{
	this->k = k;
	data.clear();
	data.reserve(k);
}

size_t DistributedTopKMaintainer::update(const std::vector<double>& recv, const int source)
{
	vector<pair<double, int>> temp;
	temp.reserve(k);
	auto it = back_inserter(temp);
	size_t cnt = 0;
	auto first1 = data.begin(), last1 = data.end();
	auto first2 = recv.begin(), last2 = recv.end();
	// replace the entries with the same source & sort up
	//   Implemented by ignoring the entries in gTopKScoures with the same source
	while(first1 != last1 && first2 != last2 && cnt < k) {
		if(first1->second == source) {
			++first1;
		} else if(first1->first >= *first2) {
			*it++ = *first1++;
			++cnt;
		} else {
			*it++ = make_pair(*first2++, source);
			++cnt;
		}
	}
	while(cnt < k && first1 != last1) {
		*it++ = *first1++;
		++cnt;
	}
	while(cnt < k && first2 != last2) {
		*it++ = make_pair(*first2++, source);
		++cnt;
	}
	data = move(temp);
	return data.size();
}
