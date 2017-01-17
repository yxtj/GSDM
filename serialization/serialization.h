#pragma once
#include "../common/Motif.h"
#include <cstdint>
#include <vector>
#include <unordered_map>
#include "serializer_basic.hpp"
#include "serializer_cont.hpp"

// TODO: global header + unit (basic) + cont
// TODO: core (struct _Serializer) + (customize type with overload non-template function) + function header (delegate function + partial spec. for customized types)

/***********************************
	Part I, single item
************************************/


/*
Serialize an item of type T into a buffer with given size, if the size is large enough.
Input: pointer to the buffer $res$, buffer size: $bufSize$, item to serialize: $item$
Output:
If the buffer is large enough for $item$, return the pointer one over the end of the used buffer.
Otherwise, return nullptr.
*/
template <class T>
inline char* serialize(char* res, int bufSize, const T& item)
{
	_Serializer<T> s;
	return s.serial(res, bufSize, item);
}

/*
Serialize an item of type T into a buffer without checking size.
Input: pointer to the buffer $res$, buffer size: $bufSize$, item to serialize: $item$
Output: the pointer one over the end of the used buffer.
*/
template <class T>
inline char* serialize(char* res, const T& item)
{
	_Serializer<T> s;
	return s.serial(res, item);
}

/*
Deserialize an item of type T from a buffer.
Input: pointer to the buffer $p$
Output: the item and a point one byte over last byte of the item in the buffer
*/
template <class T>
inline std::pair<T, char*> deserialize(char* p) {
	_Serializer<T> s;
	return s.deserial(p);
}

/*
	Existing overload versions:
*/
size_t estimateBufferSize(const Motif& m);

char* serialize(char* res, int bufSize, const Motif& item);
char* serialize(char* res, const Motif& item);
std::pair<Motif, char*> deserializeMotif(char* p);

/*
	Partial specialization versions (utlize existing functions / support customized type)
*/

// for Motif
template <>
inline std::pair<Motif, char*> deserialize<Motif>(char* p) {
	return deserializeMotif(p);
}


/***********************************
	Part II, container (using iterator)
************************************/


template <class Cont>
std::pair<char*,typename Cont::const_iterator> serializeCont(char* res, int bufSize,
	typename Cont::const_iterator first, typename Cont::const_iterator last)
{
	_SerializerCont<Cont> s;
	return s.serial(res, bufSize, first, last);
}

template <class Cont>
std::pair<Cont, char*> deserializeCont(char* p) {
	_SerializerCont<Cont> s;
	return s.deserial(p);
}

/*
	Expsting overload versions
*/
std::pair<char*, std::unordered_map<Motif, std::pair<int, double>>::const_iterator> serializeMP(
	char* res, int bufSize, std::unordered_map<Motif, std::pair<int, double>>::const_iterator first,
	std::unordered_map<Motif, std::pair<int, double>>::const_iterator last);
std::pair<std::unordered_map<Motif, std::pair<int, double>>, char*> deserializeMP(char* p);

std::pair<char*, std::vector<Motif>::const_iterator> serializeVM(char* res, int bufSize,
	std::vector<Motif>::const_iterator first, std::vector<Motif>::const_iterator last);
std::pair<std::vector<Motif>, char*> deserializeVM(char *p);


/*
	Partial specialization versions (utlizing existing functions)
*/

// for unordered_map<Motif, pair<int,double>>
template <>
inline std::pair<char*, std::unordered_map<Motif, std::pair<int, double>>::const_iterator> 
serializeCont<std::unordered_map<Motif, std::pair<int, double>>>(
	char* res, int bufSize,
	std::unordered_map<Motif, std::pair<int, double>>::const_iterator first,
	std::unordered_map<Motif, std::pair<int, double>>::const_iterator last)
{
	return serializeMP(res, bufSize, first, last);
}

template <>
inline std::pair<std::unordered_map<Motif, std::pair<int, double>>, char*>
deserializeCont<std::unordered_map<Motif, std::pair<int, double>>>(char* p)
{
	return deserializeMP(p);
}

// for vector<Motif>
template <>
inline std::pair<char*, std::vector<Motif>::const_iterator>
serializeCont<std::vector<Motif>>(char* res, int bufSize,
	std::vector<Motif>::const_iterator first, std::vector<Motif>::const_iterator last)
{
	return serializeVM(res, bufSize, first, last);
}

template <>
inline std::pair<std::vector<Motif>, char*> deserializeCont<std::vector<Motif>>(char* p)
{
	return deserializeVM(p);
}


/*
Serialize a vector of type T given by iteratoers.
	Given a fixed buffer size, try to handle the items in range [it, itend].
	If there is no enough space, serialize as much as the buffer can take.
Input: pointer to the buffer: $res$, buffer size: $bufSize$,
	first iterator to be  $first$, the iterator over the last one: $last$.
Output: pointer to the end of the used buffer (one over the last),
	iterator to the next iterator to serialize.
If all the items in range [first, last) are serialized, the returned iterator equals to last
Format:
	<num: unit32> <item: T>*num
*/
/*
template <class T>
std::pair<char*, typename std::vector<T>::const_iterator> serializeVec(char* res, int bufSize,
	typename std::vector<T>::const_iterator first, typename std::vector<T>::const_iterator last)
{
	res += sizeof(uint32_t);
	uint32_t count = 0;
	for(; first != last; ++first) {
		char* p = serialize(res, *first);
		if(p == nullptr)
			break;
		auto size = p - res;
		bufSize -= size;
		res = p;
		++count;
	}
	uint32_t* numObj = reinterpret_cast<uint32_t*>(res);
	*numObj = count;
	return make_pair(res, first);

}


template <class T>
std::vector<T> deserializeVec(char* p)
{
	uint32_t n = *reinterpret_cast<uint32_t*>(p);
	p += sizeof(uint32_t);
	vector<T> res;
	while(n--) {
		auto mp = deserialize(p);
		res.push_back(move(mp.first));
		p = mp.second;
	}
	return res;

}
*/
