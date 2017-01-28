#pragma once
#include "../util/is_container.h"
#include "../util/is_stl_container.h"
#include "serializer_basic.hpp"
#include <vector>
#include <string>

template <class Cont, class Enable = void>
struct _SerializerCont {
	//typedef typename Cont::const_iterator const_iterator;
	template <typename const_iterator>
	std::pair<char*, const_iterator> serial(char* res, int bufSize,
		const_iterator first, const_iterator last)
	{
		static_assert(impl::type_traits::template_false_type<Cont>::value,
			"serialization of this container is not provided.");
		return std::make_pair(nullptr, res);
	}
	template <typename const_iterator>
	std::pair<char*, const_iterator> serial(char* res,
		const_iterator first, const_iterator last)
	{
		static_assert(impl::type_traits::template_false_type<Cont>::value,
			"serialization of this container is not provided.");
		return std::make_pair(nullptr, res);
	}
};

/*
	Partial specialization version for raw array.
*/
template <class T>
struct _SerializerCont<T, typename std::enable_if<std::is_pointer<T>::value>::type>
{
	typedef typename std::add_const<T>::type const_iterator;
	typedef typename std::remove_pointer<T>::type value_type;
	_Serializer<value_type> sv;

	std::pair<char*, const_iterator> serial(char* res, int bufSize,
		const_iterator first, const_iterator last)
	{
		uint32_t* numObj = reinterpret_cast<uint32_t*>(res);
		res += sizeof(uint32_t);
		uint32_t count = 0;
		char* pend = res + bufSize;
		for(; first != last; ++first) {
			char* p = sv.serial(res, bufSize, *first);
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
	std::pair<char*, const_iterator> serial(char* res,
		const_iterator first, const_iterator last)
	{
		uint32_t* numObj = reinterpret_cast<uint32_t*>(res);
		res += sizeof(uint32_t);
		uint32_t count = 0;
		for(; first != last; ++first) {
			res = sv.serial(res, *first);
			++count;
		}
		*numObj = count;
		return make_pair(res, first);
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
	_Serializer<value_type> sv;

	std::pair<char*, const_iterator> serial(char* res, int bufSize,
		const_iterator first, const_iterator last)
	{
		uint32_t* numObj = reinterpret_cast<uint32_t*>(res);
		res += sizeof(uint32_t);
		uint32_t count = 0;
		char* pend = res + bufSize;
		for(; first != last; ++first) {
			char* p = sv.serial(res, bufSize, *first);
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
	std::pair<char*, const_iterator> serial(char* res,
		const_iterator first, const_iterator last)
	{
		uint32_t* numObj = reinterpret_cast<uint32_t*>(res);
		res += sizeof(uint32_t);
		uint32_t count = 0;
		for(; first != last; ++first) {
			res = sv.serial(res, *first);
			++count;
		}
		*numObj = count;
		return make_pair(res, first);
	}
};

