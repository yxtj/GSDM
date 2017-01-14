#pragma once
#include "serialization.h"
#include "../util/is_container.h"
#include <vector>

template <class Cont, class Enable = void>
struct _SerializerCont {
	typedef typename Cont::const_iterator const_iterator;
	std::pair<char*, const_iterator> serializeVec(char* res, int bufSize, 
		const_iterator first, const_iterator last)
	{
		static_assert(false, "serialization of this container is not provided.");
		return std::make_pair(nullptr, res);

	}
	T deserializeVec(char* p) {
		static_assert(false, "deserialization of this container is not provided.");
		return Cont();
	}
};

template <class Cont>
struct _SerializerCont<Cont, typename std::enable_if<is_container<Cont>::value>::type>
{
	typedef typename Cont::const_iterator const_iterator;
	std::pair<char*, const_iterator> serial(char* res, int bufSize,
		const_iterator first, const_iterator last)
	{
		char* pend = res + bufSize;
		res += sizeof(uint32_t);
		uint32_t count = 0;
		for(; first != last; ++first) {
			char* p = serialize(res, bufSize, *first);
			if(p == nullptr)
				break;
			//bufSize -= p - res;
			bufSize = pend - p;
			res = p;
			++count;
		}
		uint32_t* numObj = reinterpret_cast<uint32_t*>(res);
		*numObj = count;
		return make_pair(res, first);
	}
	Cont deserial(char* p) {
		uint32_t n = *reinterpret_cast<uint32_t*>(p);
		p += sizeof(uint32_t);
		Cont res;
		while(n--) {
			auto mp = deserialize(p);
			res.push_back(move(mp.first));
			p = mp.second;
		}
		return res;
	}
};

