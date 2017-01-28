#pragma once

namespace impl {
	namespace type_traits {
		template<typename... Ts>
		struct dummy_t {};

		// used for static_assert(false,...) on g++ 
		// g++ asserts a template function even it is not specalized.
		template<typename T>
		struct template_false_type :public std::false_type {};
	}
}
