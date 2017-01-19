#pragma once
#include <type_traits>
#include <array>
#include <vector>
#include <deque>
#include <list>
#include <forward_list>

//specialize a type for all of the sequential STL containers.
namespace impl {
	namespace type_traits {
		template <typename T>       struct is_sequential_container :std::false_type {};
		template <typename T, std::size_t N> struct is_sequential_container<std::array    <T, N>> :std::true_type {};
		template <typename... Args> struct is_sequential_container<std::vector            <Args...>> :std::true_type {};
		template <typename... Args> struct is_sequential_container<std::deque             <Args...>> :std::true_type {};
		template <typename... Args> struct is_sequential_container<std::list              <Args...>> :std::true_type {};
		template <typename... Args> struct is_sequential_container<std::forward_list      <Args...>> :std::true_type {};
	}
}

//type trait to utilize the implementation type traits as well as decay the type
template <typename T>
struct is_sequential_container {
	static constexpr bool value = impl::type_traits::is_sequential_container<typename std::decay<T>::type>::value;
};
