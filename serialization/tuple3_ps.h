#pragma once
#include "serializer_basic.hpp"
#include "../util/is_tuple.h"

// tuple-3:
template <class T>
struct _Serializer<T, typename std::enable_if<is_tuple_n<T, 3>::value>::type> {
	typedef typename std::tuple_element<0, T>::type T0;
	typedef typename std::tuple_element<1, T>::type T1;
	typedef typename std::tuple_element<2, T>::type T2;
	_Serializer<typename std::remove_cv<T0>::type> s0;
	_Serializer<typename std::remove_cv<T1>::type> s1;
	_Serializer<typename std::remove_cv<T2>::type> s2;
	int estimateSize(const T& item) {
		return s0.estimateSize(std::get<0>(item))
			+ s1.estimateSize(std::get<1>(item))
			+ s2.estimateSize(std::get<2>(item));
	}
	char* serial(char* res, int bufSize, const T& item) {
		if(estimateSize(item) > bufSize)
			return nullptr;
		return serial(res, item);
	}
	char* serial(char* res, const T& item) {
		res = s0.serial(res, std::get<0>(item));
		res = s1.serial(res, std::get<1>(item));
		res = s2.serial(res, std::get<2>(item));
		return res;
	}
	std::pair<T, const char*> deserial(const char* p) {
		auto t0 = s0.deserial(p);
		auto t1 = s1.deserial(t0.second);
		auto t2 = s2.deserial(t1.second);
		return std::make_pair(std::make_tuple(
			std::move(t0.first), std::move(t1.first), std::move(t2.first))
			, t2.second);
	}
};
