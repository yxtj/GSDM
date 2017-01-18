#pragma once
#include "serializer_basic.hpp"
#include "serializer_iter.hpp"

/***********************************
Part I, single item
************************************/

/*
Give an upper bound for about number of bytes needed to serialize the item
*/
template <class T>
inline int estimateSize(const T& item) {
	_Serializer<T> s;
	return s.estimateSize(item);
}

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
Serialize an item to a string
*/
template <class T>
inline std::string serialize(const T& item)
{
	_Serializer<T> s;
	std::string str(s.estimateSize(item), 0);
	s.serial(const_cast<char*>(str.data()), item);
	return str;
}

/*
Deserialize an item of type T from a buffer.
Input: pointer to the buffer $p$
Output: the item and a point one byte over last byte of the item in the buffer
*/
template <class T>
inline std::pair<T, const char*> deserialize(const char* p) {
	_Serializer<T> s;
	return s.deserial(p);
}

/*
Deserialize an item of type T from a string
*/
template <class T>
inline T deserialize(const std::string& str) {
	_Serializer<T> s;
	return s.deserial(str.data()).first;
}


/***********************************
Part II, serialize container using iterator
************************************/

/*
Similar to previous single item versions but only try to serialize items with in given range.
Output:
	1, the pointer one over the end of the used buffer.
	2, the iterator one over the last serialized one.
*/
template <class Cont>
inline std::pair<char*, typename Cont::const_iterator> serializeCont(char* res, int bufSize,
	typename Cont::const_iterator first, typename Cont::const_iterator last)
{
	_SerializerCont<Cont> s;
	return s.serial(res, bufSize, first, last);
}

/*
Similiar to the sized version but assume the buffer size is enough and always return the last iterator
*/
template <class Cont>
inline std::pair<char*, typename Cont::const_iterator> serializeCont(char* res, 
	typename Cont::const_iterator first, typename Cont::const_iterator last)
{
	_SerializerCont<Cont> s;
	return s.serial(res, first, last);
}

template <class Cont>
inline std::string serializeCont(typename Cont::const_iterator first, typename Cont::const_iterator last)
{
	_SerializerCont<Cont> s;
	std::string res;
	do {
		std::string str(128, 0);
		char* p, *p0 = const_cast<char*>(str.data());
		tie(p, first) = s.serial(p0, first, last);
		str.resize(p - p0);
		res += str;
	} while(first != last);
	return res;
}

