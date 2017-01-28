#pragma once
#include <type_traits>
#include <utility>
#include "type_trais_dummy.h"

template<typename T, typename Enable = void>
struct is_pair: public std::false_type {};

template <typename T>
struct is_pair<
	T,
	typename std::conditional<
		false,
		impl::type_traits::dummy_t<
			typename T::first_type,
			typename T::second_type
		>,
		void
	>::type
> : public std::true_type{};
