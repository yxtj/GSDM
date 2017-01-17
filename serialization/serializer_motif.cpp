#include "serialization.h"

using namespace std;

// (de)seralizes single motif

size_t estimateBufferSize(const Motif & m)
{
	//n, (src, dst)*n
	return sizeof(int) + 2 * m.getnEdge() * sizeof(int);
}

char* serialize(char* res, int bufSize, const Motif& m) {
	if(static_cast<size_t>(bufSize) < estimateBufferSize(m))
		return nullptr;
	return serialize(res, m);
}

char* serialize(char* res, const Motif& m) {
	int* pint = reinterpret_cast<int*>(res);
	*pint++ = m.getnEdge();
	for(const Edge& e : m.edges) {
		*pint++ = e.s;
		*pint++ = e.d;
	}
	return reinterpret_cast<char*>(pint);
}

pair<Motif, char*> deserializeMotif(char* p) {
	int* pint = reinterpret_cast<int*>(p);
	int ne = *pint++;
	Motif m;
	while(ne--) {
		int s = *pint++;
		int d = *pint++;
		m.addEdge(s, d);
	}
	return make_pair(move(m), reinterpret_cast<char*>(pint));
}
