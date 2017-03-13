#pragma once
#include "../common/Graph.h"
#include "../common/Motif.h"
#include <vector>
#include "../serialization/serializer_basic.hpp"
struct SDSignature {
	std::vector<std::vector<int>> sd;
	SDSignature(int n) : sd(n, std::vector<int>(n, 2 * n)) {
		//for(int i = 0; i < n; ++i)
		//	sd[i][i] = 0;
	}
	SDSignature(const Graph& g);
	SDSignature(const MotifBuilder& m, const int nNode);

	void update(const int s, const int d);

	std::vector<std::vector<int>>::reference operator[](const int idx) { return sd[idx]; }
	std::vector<std::vector<int>>::const_reference operator[](const int idx) const { return sd[idx]; }

	static std::vector<std::vector<int>> calA2AShortestDistance(const Graph& g);
};

template <>
struct _Serializer<SDSignature> {
	int estimateSize(const SDSignature& item) {
		return sizeof(int)*(1 + item.sd.size()*item.sd.size());
	}
	char* serial(char* res, int bufSize, const SDSignature& item) {
		if(bufSize < estimateSize(item))
			return nullptr;
		return serial(res, item);
	}
	char* serial(char* res, const SDSignature& item) {
		int32_t* p = reinterpret_cast<int32_t*>(res);
		*p++ = item.sd.size();
		for(auto& vec : item.sd) {
			for(auto& v : vec)
				*p++ = v;
		}
		return reinterpret_cast<char*>(p);
	}
	std::pair<SDSignature, const char*> deserial(const char* p) {
		const int32_t* pi= reinterpret_cast<const int32_t*>(p);
		int n = *p++;
		SDSignature res(n);
		for(int i = 0; i < n; ++i) {
			for(int j = 0; j < n; ++j)
				res.sd[i][j] = *p++;
		}
		return std::make_pair(std::move(res), reinterpret_cast<const char*>(pi));
	}
};
