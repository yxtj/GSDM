#pragma once
#include <type_traits>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <queue>

//specialize a type for all of the associated STL containers.
namespace impl {
	namespace type_traits {
		template <typename T>       struct is_associated_container :std::false_type {};
		template <typename... Args> struct is_associated_container<std::set               <Args...>> :std::true_type {};
		template <typename... Args> struct is_associated_container<std::multiset          <Args...>> :std::true_type {};
		template <typename... Args> struct is_associated_container<std::map               <Args...>> :std::true_type {};
		template <typename... Args> struct is_associated_container<std::multimap          <Args...>> :std::true_type {};
		template <typename... Args> struct is_associated_container<std::unordered_set     <Args...>> :std::true_type {};
		template <typename... Args> struct is_associated_container<std::unordered_multiset<Args...>> :std::true_type {};
		template <typename... Args> struct is_associated_container<std::unordered_map     <Args...>> :std::true_type {};
		template <typename... Args> struct is_associated_container<std::unordered_multimap<Args...>> :std::true_type {};
	}
}

//type trait to utilize the implementation type traits as well as decay the type
template <typename T>
struct is_associated_container {
	static constexpr bool value = impl::type_traits::is_associated_container<typename std::decay<T>::type>::value;
};
