#pragma once
#include <type_traits>
#include <tuple>

template <class T, class = void>
struct is_tuple :public std::false_type {};

template<class T>
struct is_tuple<T, typename std::enable_if<std::tuple_size<T>::value != 0>::type> :
	public std::true_type {};

template <class T, int N, class = void>
struct is_tuple_n :public std::false_type {};

template<class T, int N>
struct is_tuple_n<T, N, typename std::enable_if<std::tuple_size<T>::value == N>::type> :
	public std::true_type {};
