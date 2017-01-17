#pragma once
#include "serialization.h"
#include "../util/is_container.h"
#include "../util/is_stl_container.h"
#include <vector>

template <class Cont, class Enable = void>
struct _SerializerCont {
	//typedef typename Cont::const_iterator const_iterator;
	template <typename const_iterator>
	std::pair<char*, const_iterator> serialize(char* res, int bufSize, 
		const_iterator first, const_iterator last)
	{
		static_assert(false, "serialization of this container is not provided.");
		return std::make_pair(nullptr, res);
	}
	std::pair<Cont, char*> deserialize(char* p) {
		static_assert(false, "deserialization of this container is not provided.");
		return make_pair(Cont(), nullptr);
	}
};

/*
	Partial specialization version for raw array.
*/
template <class T>
struct _SerializerCont<T, typename std::enable_if<std::is_pointer<T>::value>::type>
{
	typedef typename std::add_const<T>::type const_iterator;
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
	std::pair<std::vector<T>, char*> deserial(char* p) {
		uint32_t n = *reinterpret_cast<uint32_t*>(p);
		p += sizeof(uint32_t);
		std::vector<T> res;
		while(n--) {
			auto mp = deserialize<T>(p);
			res.push_back(move(mp.first));
			p = mp.second;
		}
		return make_pair(move(res), p);
	}
};

/*
	Partial specialization version for contianer and its iterator.
*/
template <class Cont>
struct _SerializerCont<Cont, typename std::enable_if<is_stl_container<Cont>::value>::type>
	//typename std::enable_if<is_container<Cont>::value>::type>
{
	typedef typename Cont::const_iterator const_iterator;
	typedef typename Cont::value_type value_type;
	std::pair<char*, const_iterator> serial(char* res, int bufSize,
		const_iterator first, const_iterator last)
	{
		uint32_t* numObj = reinterpret_cast<uint32_t*>(res);
		res += sizeof(uint32_t);
		uint32_t count = 0;
		char* pend = res + bufSize;
		for(; first != last; ++first) {
			char* p = serialize(res, bufSize, *first);
			if(p == nullptr)
				break;
			//bufSize -= p - res;
			bufSize = pend - p;
			res = p;
			++count;
		}
		*numObj = count;
		return make_pair(res, first);
	}
	std::pair<Cont, char*> deserial(char* p) {
		uint32_t n = *reinterpret_cast<uint32_t*>(p);
		p += sizeof(uint32_t);
		Cont res;
		while(n--) {
			auto mp = deserialize<value_type>(p);
			res.insert(res.end(), move(mp.first));
			p = mp.second;
		}
		return make_pair(move(res), p);
	}
};

