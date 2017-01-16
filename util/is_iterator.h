#pragma once
#include <type_traits>
#include "type_trais_dummy.h"

template<class T, class Enable = void>
struct is_iterator : public std::false_type {};


template<class T>
struct is_iterator<T,
	typename std::conditional<
	false,
	typename std::iterator_traits<T>::iterator_category,
	void
	>::type
>
//impl::type_traits::dummy_t<
//	typename std::iterator_traits<T>::iterator_category> >
	: public std::true_type {};


template<class T, class Cont, class Enable = void>
struct is_iterator_of : public std::false_type {};

template<class T, class Cont>
struct is_iterator_of<T, Cont, typename std::enable_if<
	std::is_same<T, typename Cont::iterator>::value >::type>
	:public std::true_type {};

template<class T, class Cont>
struct is_iterator_of<T, Cont, typename std::enable_if<
	std::is_same<T, typename Cont::const_iterator>::value >::type>
	:public std::true_type {};
