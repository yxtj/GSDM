#include <type_traits>
#include <utility>

template<typename T, typename Enable = void>
struct is_container : public std::false_type {};


namespace impl{
	namespace type_traits {
		template<typename... Ts>
		struct is_container_helper {};
	}
}

template <typename T>
struct is_container<
	T,
	typename std::conditional<
		false,
		impl::type_traits::is_container_helper<
			typename T::value_type,
			typename T::size_type,
			typename T::allocator_type,
			typename T::iterator,
			typename T::const_iterator,
			decltype(std::declval<T>().size()),
			decltype(std::declval<T>().begin()),
			decltype(std::declval<T>().end()),
			decltype(std::declval<T>().cbegin()),
			decltype(std::declval<T>().cend())
		>,
		void
	>::type
> : public std::true_type {};
