#include "c_motif.h"

using namespace std;

// (de)seralizes single motif

size_t estimateSizeMotif(const Motif & m)
{
	//n, (src, dst)*n
	return sizeof(int) + 2 * m.getnEdge() * sizeof(int);
}

char* serializeMotif(char* res, int bufSize, const Motif& m) {
	if(static_cast<size_t>(bufSize) < estimateSizeMotif(m))
		return nullptr;
	return serializeMotif(res, m);
}

char* serializeMotif(char* res, const Motif& m) {
	int* pint = reinterpret_cast<int*>(res);
	*pint++ = m.getnEdge();
	for(const Edge& e : m.edges) {
		*pint++ = e.s;
		*pint++ = e.d;
	}
	return reinterpret_cast<char*>(pint);
}

pair<Motif, const char*> deserializeMotif(const char* p) {
	const int* pint = reinterpret_cast<const int*>(p);
	int ne = *pint++;
	Motif m;
	while(ne--) {
		int s = *pint++;
		int d = *pint++;
		m.addEdge(s, d);
	}
	return make_pair(move(m), reinterpret_cast<const char*>(pint));
}

size_t estimateSizeMotif(const MotifBuilder & m)
{
	return sizeof(int) + 2 * m.getnEdge() * sizeof(int);
}

char * serializeMotif(char * res, int bufSize, const MotifBuilder & m)
{
	if(static_cast<size_t>(bufSize) < estimateSizeMotif(m))
		return nullptr;
	return serializeMotif(res, m);
}

char * serializeMotif(char * res, const MotifBuilder & m)
{
	int* pint = reinterpret_cast<int*>(res);
	*pint++ = m.getnEdge();
	for(const Edge& e : m.edges) {
		*pint++ = e.s;
		*pint++ = e.d;
	}
	return reinterpret_cast<char*>(pint);
}

std::pair<MotifBuilder, const char*> deserializeMotifBuilder(const char * p)
{
	const int* pint = reinterpret_cast<const int*>(p);
	int ne = *pint++;
	MotifBuilder m;
	while(ne--) {
		int s = *pint++;
		int d = *pint++;
		m.addEdge(s, d);
	}
	return make_pair(move(m), reinterpret_cast<const char*>(pint));
}

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
		char *p = serializeMotif(res, it->first);
		*reinterpret_cast<int*>(p) = it->second.first;
		*reinterpret_cast<double*>(p + sizeof(int)) = it->second.second;
		res += size;
		bufSize -= size;
		++count;
	}
	*numObj = count;
	return make_pair(res, it);
}

pair<unordered_map<Motif, pair<int, double>>, const char*> deserializeMP(const char* p) {
	size_t n = *reinterpret_cast<const size_t*>(p);
	p += sizeof(size_t);
	unordered_map<Motif, pair<int, double>> res;
	while(n--) {
		auto mp = deserializeMotif(p);
		p = mp.second;
		int count = *reinterpret_cast<const int*>(p);
		p += sizeof(int);
		double prob = *reinterpret_cast<const double*>(p);
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
		res = serializeMotif(res, *it);
		bufSize -= size;
		++count;
	}
	*numObj = count;
	return make_pair(res, it);
}

pair<vector<Motif>, const char*> deserializeVM(const char *p) {
	size_t n = *reinterpret_cast<const size_t*>(p);
	p += sizeof(size_t);
	vector<Motif> res;
	while(n--) {
		auto mp = deserializeMotif(p);
		res.push_back(move(mp.first));
		p = mp.second;
	}
	return make_pair(move(res), p);
}
