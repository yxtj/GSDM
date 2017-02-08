#pragma once
#include <utility>
#include <type_traits>
#include <cstdint>
#include <cstring>
#include "../util/is_container.h"
#include "../util/is_pair.h"
#include "../util/type_trais_dummy.h"

template <class T, class Enable = void>
struct _Serializer {
	int estimateSize(const T& item) {
		static_assert(impl::type_traits::template_false_type<T>::value,
			"serialization of this type is not provided.");
		return 0;
	}
	char* serial(char* res, int bufSize, const T& item) {
		static_assert(impl::type_traits::template_false_type<T>::value,
			"serialization of this type is not provided.");
		return nullptr;
	}
	char* serial(char* res, const T& item) {
		static_assert(impl::type_traits::template_false_type<T>::value,
			"serialization of this type is not provided.");
		return nullptr;
	}
	std::pair<T, const char*> deserial(const char* p) {
		static_assert(impl::type_traits::template_false_type<T>::value, 
			"deserialization of this type is not provided.");
		return std::make_pair(T(), nullptr);
	}
};

// POD
template <class T>
struct _Serializer<T, typename std::enable_if<std::is_pod<T>::value>::type> {
	static constexpr uint32_t size = sizeof(T);
	int estimateSize(const T& item) {
		return size;
	}
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
	std::pair<T, const char*> deserial(const char* p) {
		T item = *reinterpret_cast<const T*>(p);
		return std::make_pair(std::move(item), p + size);
	}
};

// string:
template <>
struct _Serializer<std::string> {
	int estimateSize(const std::string& item) {
		return sizeof(uint32_t) + item.size();
	}
	char* serial(char* res, int bufSize, const std::string& item) {
		if(estimateSize(item) > bufSize)
			return nullptr;
		return serial(res, item);
	}
	char* serial(char* res, const std::string& item) {
		uint32_t size = item.size();
		*reinterpret_cast<uint32_t*>(res) = size;
		std::memcpy(res + sizeof(uint32_t), item.data(), item.size());
		return res + sizeof(uint32_t) + item.size();
	}
	std::pair<std::string, const char*> deserial(const char* p) {
		uint32_t size = *reinterpret_cast<const uint32_t*>(p);
		std::string str(size, 0);
		std::memcpy(const_cast<char*>(str.data()), p + sizeof(uint32_t), size);
		return std::make_pair(std::move(str), p + sizeof(uint32_t) + size);
	}
};

// pair:
template <class T>
struct _Serializer<T, typename std::enable_if<is_pair<T>::value>::type> {
	typedef typename T::first_type T1;
	typedef typename T::second_type T2;
	_Serializer<typename std::remove_cv<T1>::type> s1;
	_Serializer<typename std::remove_cv<T2>::type> s2;
	int estimateSize(const T& item) {
		return s1.estimateSize(item.first) + s2.estimateSize(item.second);
	}
	char* serial(char* res, int bufSize, const T& item) {
		if(estimateSize(item) > bufSize)
			return nullptr;
		return serial(res, item);
	}
	char* serial(char* res, const T& item) {
		res = s1.serial(res, item.first);
		res = s2.serial(res, item.second);
		return res;
	}
	std::pair<T, const char*> deserial(const char* p) {
		auto t1 = s1.deserial(p);
		auto t2 = s2.deserial(t1.second);
		return std::make_pair(std::make_pair(std::move(t1.first), std::move(t2.first)), t2.second);
	}
};

// containter:
template <class T>
struct _Serializer<T, typename std::enable_if<is_container<T>::value>::type> {
	typedef typename T::const_iterator const_iterator;
	typedef typename std::remove_cv<typename T::value_type>::type value_type;
	_Serializer<value_type> sv;
	int estimateSize(const T& item) {
		int c = sizeof(uint32_t);
		if(std::is_pod<value_type>::value) {
			c += item.size() * sizeof(value_type);
		} else {
			for(auto& v : item)
				c += sv.estimateSize(v);
		}
		return c;
	}
	char* serial(char* res, int bufSize, const T& item) {
		static_assert(impl::type_traits::template_false_type<T>::value, 
			"Please use iterator version for container");
		return nullptr;
	}
	char* serial(char* res, const T& item) {
		//static_assert(impl::type_traits::template_false_type<T>::value,
		//	"Please use iterator version for container");
		uint32_t* numObj = reinterpret_cast<uint32_t*>(res);
		res += sizeof(uint32_t);
		uint32_t count = 0;
		auto last = item.end();
		for(auto first=item.begin(); first != last; ++first) {
			char* p = sv.serial(res, *first);
			res = p;
			++count;
		}
		*numObj = count;
		return res;
	}
	std::pair<T, const char*> deserial(const char* p) {
		uint32_t n = *reinterpret_cast<const uint32_t*>(p);
		p += sizeof(uint32_t);
		T res;
		while(n--) {
			auto mp = sv.deserial(p);
			//auto mp = deserialize<value_type>(p);
			res.insert(res.end(), std::move(mp.first));
			p = mp.second;
		}
		return std::make_pair(std::move(res), p);
	}
};
