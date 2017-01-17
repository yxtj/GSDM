#pragma once
#include <utility>
#include <type_traits>
#include <cstdint>
#include "../util/is_container.h"
#include "../util/is_pair.h"

template <class T, class Enable = void>
struct _Serializer {
	char* serial(char* res, int bufSize, const T& item) {
		static_assert(false, "serialization of this type is not provided.");
		return nullptr;
	}
	char* serial(char* res, const T& item) {
		static_assert(false, "serialization of this type is not provided.");
		return nullptr;
	}
	std::pair<T, char*> deserial(char* p) {
		static_assert(false, "deserialization of this type is not provided.");
		return std::make_pair(T(), nullptr);
	}
};

// POD
template <class T>
struct _Serializer<T, typename std::enable_if<std::is_pod<T>::value>::type> {
	static constexpr uint32_t size = sizeof(T);
	char* serial(char* res, int bufSize, const T& item) {
		if(size > bufSize)
			return nullptr;
		return serial(res, item);
	}
	char* serial(char* res, const T& item) {
		char* p = reinterpret_cast<char*>(res);
		std::memcpy(p, reinterpret_cast<const char*>(&item), size);
		return p + size;
	}
	std::pair<T, char*> deserial(char* p) {
		T item = *reinterpret_cast<T*>(p);
		return make_pair(move(item), p + size);
	}
};

// pair:
template <class T>
struct _Serializer<T, typename std::enable_if<is_pair<T>::value>::type> {
	typedef typename T::first_type T1;
	typedef typename T::second_type T2;
	static constexpr uint32_t size = sizeof(T1) + sizeof(T2);
	char* serial(char* res, int bufSize, const T& item) {
		if(size > bufSize)
			return nullptr;
		return serial(res, item);
	}
	char* serial(char* res, const T& item) {
		_Serializer<T1> s1;
		res = s1.serial(res, item.first);
		_Serializer<T2> s2;
		res = s2.serial(res, item.second);
		return res;
	}
	std::pair<T, char*> deserial(char* p) {
		_Serializer<T1> s1;
		auto t1 = s1.deserial(p);
		_Serializer<T2> s2;
		auto t2 = s2.deserial(t1.second);
		return std::make_pair(std::make_pair(std::move(t1.first), std::move(t2.first)), t2.second);
	}
};

// containter:
template <class T>
struct _Serializer<T, typename std::enable_if<is_container<T>::value>::type> {
	char* serial(char* res, int bufSize, const T& item) {
		static_assert(false, "Please use iterator version for container");
		return nullptr;
	}
	char* serial(char* res, const T& item) {
		static_assert(false, "Please use iterator version for container");
		return nullptr;
	}
	std::pair<T, char*> deserial(char* p) {
		static_assert(false, "Please use iterator version for container");
		return std::make_pair(T(), nullptr);
	}
};
