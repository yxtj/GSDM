#include "serialization.h"

using namespace std;

// old (de)seralizes of motif containers

pair<char*, unordered_map<Motif, pair<int, double>>::const_iterator> serializeMP(
	char* res, int bufSize, unordered_map<Motif, pair<int, double>>::const_iterator it,
	unordered_map<Motif, pair<int, double>>::const_iterator itend)
{
	size_t* numObj = reinterpret_cast<size_t*>(res);
	res += sizeof(size_t);
	bufSize -= sizeof(size_t);
	size_t count = 0;
	for(; it != itend; ++it) {
		int n = it->first.getnEdge();
		int size = sizeof(int) + n * 2 * sizeof(int) + sizeof(int) + sizeof(double);
		if(size > bufSize)
			break;
		char *p = serialize(res, it->first);
		*reinterpret_cast<int*>(p) = it->second.first;
		*reinterpret_cast<double*>(p + sizeof(int)) = it->second.second;
		res += size;
		bufSize -= size;
		++count;
	}
	*numObj = count;
	return make_pair(res, it);
}

pair<unordered_map<Motif, pair<int, double>>, char*> deserializeMP(char* p) {
	size_t n = *reinterpret_cast<size_t*>(p);
	p += sizeof(size_t);
	unordered_map<Motif, pair<int, double>> res;
	while(n--) {
		auto mp = deserialize<Motif>(p);
		p = mp.second;
		int count = *reinterpret_cast<int*>(p);
		p += sizeof(int);
		double prob = *reinterpret_cast<double*>(p);
		p += sizeof(double);
		res[mp.first] = make_pair(count, prob);
	}
	return make_pair(move(res), p);
}

pair<char*, vector<Motif>::const_iterator> serializeVM(char* res, int bufSize,
	vector<Motif>::const_iterator it, vector<Motif>::const_iterator itend)
{
	size_t* numObj = reinterpret_cast<size_t*>(res);
	res += sizeof(size_t);
	size_t count = 0;
	for(; it != itend; ++it) {
		int n = it->getnEdge();
		int size = sizeof(int) + n * 2 * sizeof(int);
		if(size > bufSize)
			break;
		res = serialize(res, *it);
		bufSize -= size;
		++count;
	}
	*numObj = count;
	return make_pair(res, it);
}

pair<vector<Motif>, char*> deserializeVM(char *p) {
	size_t n = *reinterpret_cast<size_t*>(p);
	p += sizeof(size_t);
	vector<Motif> res;
	while(n--) {
		auto mp = deserialize<Motif>(p);
		res.push_back(move(mp.first));
		p = mp.second;
	}
	return make_pair(move(res), p);
}

