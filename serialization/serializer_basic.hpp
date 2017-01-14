#pragma once
#include <utility>
#include <type_traits>
#include <cstdint>

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

template <class T>
struct _Serializer<T, typename std::enable_if<std::is_pod<T>::value>::type> {
	static constexpr uint32_t size = sizeof(T);
	char* serial(char* res, int bufSize, const T& item) {
		if(size > bufSize)
			return nullptr;
		return serialNoCheck(res, bufSize, item);
	}
	char* serial(char* res, const T& item) {
		char* p = reinterpret_cast<char*>(res);
		std::memcpy(p, reinterpret_cast<const char*>(&item), size);
		return p + size;
	}
	std::pair<T, char*> deserialize(char* p) {
		T item = *reinterpret_cast<T*>(p);
		return make_pair(move(item), p + size);
	}
};

