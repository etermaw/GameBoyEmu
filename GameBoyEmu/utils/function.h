#pragma once

#include <utility>
#include <type_traits>

template<class T>
class function;

template<typename R,typename... Args>
class function<R(Args...)> final
{
	using fun_type = R(*)(void*, Args&&...);

	private:
		void* obj_ptr;
		fun_type fun_ptr;

	public:
		function(void* object = nullptr, fun_type function = nullptr) : obj_ptr(object), fun_ptr(function) {}

		template<class F>
		function(F&& lambda) : obj_ptr(std::addressof(lambda))
		{
			fun_ptr = [](void* ptr, Args&&... args) -> R
			{
				return (*static_cast<std::add_pointer_t<F>>(ptr))(std::forward<Args>(args)...);
			};
		}

		R operator() (Args... args) const
		{
			return (*fun_ptr)(obj_ptr, std::forward<Args>(args)...);
		}
};

template<class R,typename T,typename... Args>
struct method_call final
{
	template<R(T::*fun)(Args...)>
	static R call(void* obj, Args&&... args)
	{
		return (static_cast<T*>(obj)->*fun)(std::forward<Args>(args)...);
	}

	template<R(T::*fun)(Args...)>
	inline static function<R(Args...)> bind(T* object)
	{
		return function<R(Args...)>(object, &call<fun>);
	}
};

template<class R, typename T, typename... Args>
struct const_method_call final
{
	template<R(T::*fun)(Args...) const>
	static R call(void* obj, Args&&... args)
	{
		return (static_cast<T*>(obj)->*fun)(std::forward<Args>(args)...);
	}

	template<R(T::*fun)(Args...) const>
	inline static function<R(Args...)> bind(T* object)
	{
		return function<R(Args...)>(object, &call<fun>);
	}
};

template<class R, typename... Args>
struct function_call final
{
	template<R(*fun)(Args...)>
	static R call(void* obj, Args&&... args)
	{
		return (*fun)(std::forward<Args>(args)...);
	}

	template<R(*fun)(Args...)>
	inline static function<R(Args...)> bind()
	{
		return function<R(Args...)>(nullptr, &call<fun>);
	}
};

template<typename R,class T,typename... Args>
method_call<R, T, Args...> make_fun(R(T::*)(Args...))
{
	return method_call<R, T, Args...>();
}

template<typename R, class T, typename... Args>
const_method_call<R, T, Args...> make_fun(R(T::*)(Args...) const)
{
	return const_method_call<R, T, Args...>();
}

template<typename R, typename... Args>
function_call<R, Args...> make_fun(R(*)(Args...))
{
	return function_call<R, Args...>();
}

#define make_function1(function_ptr) make_fun(function_ptr).bind<function_ptr>()
#define make_function2(method_ptr,object_ptr) make_fun(method_ptr).bind<method_ptr>(object_ptr)

#define EXPAND(x) x
#define GET_MACRO(_1,_2,name,...) name
#define make_function(...) EXPAND(GET_MACRO(__VA_ARGS__,make_function2,make_function1)(__VA_ARGS__))