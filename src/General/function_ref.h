/// An efficient, type-erasing, non-owning reference to a callable. This is intended for use as
/// the type of a function parameter that is not used after the function in question returns.
///
/// This class does not own the callable, so it is not in general safe to store a function_ref.

// This is a cut-down version of the implementation from LLVM

#ifndef GENERAL_FUNCTION_REF_H
#define GENERAL_FUNCTION_REF_H

#include <type_traits>
#include <utility>

#ifdef __ECV__

// eCv doesn't yet handle variadic templates or template specialisations, so use dummy definitions for now
template<typename Fn> class function_ref
{
};

template<typename Fn> class function_ref_noexcept
{
};

#else

// Version that may throw exceptions
template<typename Fn> class function_ref;

template<typename RetType, typename ...Params>
class function_ref<RetType(Params...)>
{
	RetType (*callback)(void *callable, Params ...params);
	void *callable;

	template<typename Callable> static RetType callback_fn(void *callable2, Params ...params)
	{
		return (*reinterpret_cast<Callable*>(callable2))(std::forward<Params>(params)...);
	}

public:
	template <typename Callable> function_ref(Callable &&callable2,
				typename std::enable_if<!std::is_same<typename std::remove_reference<Callable>::type, function_ref>::value>::type * = nullptr) noexcept
	   : callback(callback_fn<typename std::remove_reference<Callable>::type>),
		 callable(reinterpret_cast<void *>(&callable2)) {}

	/// Creates a function_ref which refers to the same callable as rhs
    constexpr function_ref(const function_ref<RetType(Params...)> &rhs) noexcept = default;

    /// Call the stored callable with the given arguments
    RetType operator()(Params ...params) const
	{
		return callback(callable, std::forward<Params>(params)...);
	}
};

// Version that never throws exceptions
template<typename Fn> class function_ref_noexcept;

template<typename RetType, typename ...Params>
class function_ref_noexcept<RetType(Params...) noexcept>
{
	RetType (*callback)(void *callable, Params ...params) noexcept;
	void *callable;

	template<typename Callable> static RetType callback_fn(void *callable2, Params ...params) noexcept
	{
		return (*reinterpret_cast<Callable*>(callable2))(std::forward<Params>(params)...);
	}

public:
	template <typename Callable> function_ref_noexcept(Callable &&callable2,
				typename std::enable_if<!std::is_same<typename std::remove_reference<Callable>::type, function_ref_noexcept>::value>::type * = nullptr) noexcept
	   : callback(callback_fn<typename std::remove_reference<Callable>::type>),
		 callable(reinterpret_cast<void *>(&callable2)) {}

	/// Creates a function_ref which refers to the same callable as rhs
    constexpr function_ref_noexcept(const function_ref_noexcept<RetType(Params...) noexcept> &rhs) noexcept = default;

    /// Call the stored callable with the given arguments
    RetType operator()(Params ...params) const noexcept
	{
		return callback(callable, std::forward<Params>(params)...);
	}
};

#endif

#endif /* GENERAL_FUNCTION_REF_H */
