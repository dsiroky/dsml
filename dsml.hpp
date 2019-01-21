/// State machine library.
///
/// @copyright https://github.com/dsiroky/dsml/blob/master/LICENSE.txt
///
/// @file

#pragma once
#ifndef DSML_HPP__O06IR34S
#define DSML_HPP__O06IR34S

#include <functional>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

#ifdef _MSC_VER
  #pragma warning(push)
#else
  #pragma GCC diagnostic push
  #if defined(__clang__)
    #pragma clang diagnostic ignored "-Wgnu-string-literal-operator-template"
  #endif
#endif

//==========================================================================

#define _DSML_REQUIRES(...) typename std::enable_if<__VA_ARGS__, int>::type = 0

//==========================================================================
namespace dsml {
//==========================================================================

template<typename>
struct TransitionTable;
template<typename, typename, typename, typename, typename>
struct TableRow;
template<typename, typename>
struct State;
template<typename>
struct Event;

template <typename T>
auto get_type_name();

//--------------------------------------------------------------------------

struct Policy {};
struct Observer : Policy {};

//--------------------------------------------------------------------------

template<typename _T>
struct IsState : std::false_type {};
template<typename _T, typename _Tag>
struct IsState<State<_T, _Tag>> : std::true_type {};

template<typename _T>
struct IsEvent : std::false_type {};
template<typename _T>
struct IsEvent<Event<_T>> : std::true_type {};

//==========================================================================
namespace detail { namespace {
//==========================================================================

namespace {
  const auto always_true_guard = [](){ return true; };
  const auto no_action = [](){};
}

//==========================================================================

template <typename _T, _T...>
struct CString;
template <char... _Chrs>
struct CString<char, _Chrs...> {
  using type = CString;
  static auto c_str() noexcept {
    static const char str[] = {_Chrs..., 0};
    return str;
  }
};

//--------------------------------------------------------------------------

template <class, size_t N, size_t... Ns>
auto get_type_name_impl(const char *ptr, std::index_sequence<Ns...>) {
  static const char str[] = {ptr[N + Ns]..., 0};
  return str;
}

//--------------------------------------------------------------------------

template <typename _T>
struct HasStaticCStr
{
  using yes = char[1];
  using no = char[2];

  template <class U>
  struct MethodTrait;

  template <typename U>
  static yes& _has(MethodTrait<decltype(&U::c_str)>*);

  template <typename>
  static no& _has(...);

  static constexpr bool value{sizeof(_has<_T>(0)) == sizeof(yes)};
};

template<typename _T>
auto c_str_impl(std::true_type)
{
  return _T::c_str();
}

template<typename _T>
auto c_str_impl(std::false_type)
{
  return get_type_name<_T>();
}

template<typename _T>
auto c_str()
{
  return c_str_impl<_T>(std::conditional_t<HasStaticCStr<_T>::value,
                                          std::true_type, std::false_type>{});
}

//--------------------------------------------------------------------------

// type holder is only for autodeduction (avoid instantiation of the type itself)
template<typename _Type>
struct TypeHolder { using type = _Type; };

//--------------------------------------------------------------------------

// missing in gcc 5 STL
template<typename...> struct disjunction : std::false_type { };
template<typename _B1> struct disjunction<_B1> : _B1 { };
template<typename _B1, typename... _Bn>
struct disjunction<_B1, _Bn...>
    : std::conditional_t<bool(_B1::value), _B1, disjunction<_Bn...>>  { };

//--------------------------------------------------------------------------

/// provides the minimal unsigned type that can fit the number
template<size_t _Num>
struct MinimalUnsigned
{
  using type =
    std::conditional_t<_Num <= std::numeric_limits<uint8_t>::max(),
                  uint8_t,
    std::conditional_t<_Num <= std::numeric_limits<uint16_t>::max(),
                  uint16_t,
                  uint32_t>>;
};

//--------------------------------------------------------------------------

template<size_t, typename>
struct PrependIndex;
template<size_t _I0, size_t... _Is>
struct PrependIndex<_I0, std::index_sequence<_Is...>>
{
  using type = std::index_sequence<_I0, _Is...>;
};
template<size_t _I, typename _Seq>
using PrependIndex_t = typename PrependIndex<_I, _Seq>::type;

template<typename, typename>
struct ConcatIndexSeq;
template<size_t... _Is1, size_t... _Is2>
struct ConcatIndexSeq<std::index_sequence<_Is1...>, std::index_sequence<_Is2...>>
{
  using type = std::index_sequence<_Is1..., _Is2...>;
};
template<typename _S1, typename _S2>
using ConcatIndexSeq_t = typename ConcatIndexSeq<_S1, _S2>::type;

//--------------------------------------------------------------------------

template<typename _T>
struct IsEmptyTuple
{
  static constexpr auto value = std::tuple_size<_T>::value == 0;
};

//--------------------------------------------------------------------------

template <typename _T, typename...>
struct HasType;
template <typename _T, typename... _Us>
struct HasType<_T, std::tuple<_Us...>> : disjunction<std::is_same<_T, _Us>...> {};

template<typename...>
struct RemoveFirstType;
template<typename _T, typename... _Ts>
struct RemoveFirstType<std::tuple<_T, _Ts...>>
{
  using type = std::tuple<_Ts...>;
};
template<typename... _T>
using RemoveFirstType_t = typename RemoveFirstType<_T...>::type;

template<typename... _T>
struct PrependType;
template<typename _T0, typename... _T>
struct PrependType<_T0, std::tuple<_T...>>
{
  using type = std::tuple<_T0, _T...>;
};
template<typename... _T>
using PrependType_t = typename PrependType<_T...>::type;

template<typename...>
struct ConcatTypesImpl;
template<typename _T>
struct ConcatTypesImpl<_T>
{
  using type = _T;
};
template<typename... _T0s, typename... _T1s, typename... _Rest>
struct ConcatTypesImpl<std::tuple<_T0s...>, std::tuple<_T1s...>, _Rest...>
{
  using type = typename ConcatTypesImpl<std::tuple<_T0s..., _T1s...>, _Rest...>::type;
};

template<typename... _Tuples>
using ConcatTypes_t = typename ConcatTypesImpl<_Tuples...>::type;

template<typename... _T>
struct UniqueTypes;
template<typename... _T>
using UniqueTypes_t = typename UniqueTypes<_T...>::type;
template<>
struct UniqueTypes<std::tuple<>>
{
  using type = std::tuple<>;
};
template<typename _T0, typename... _T>
struct UniqueTypes<std::tuple<_T0, _T...>>
{
private:
  using rest_t = UniqueTypes_t<std::tuple<_T...>>;
public:
  using type = typename std::conditional_t<HasType<_T0, std::tuple<_T...>>::value,
                  rest_t,
                  PrependType_t<_T0, rest_t>
                >;
};

template <template <class...> class, class>
struct Apply;
template <template <class...> class T, class... Ts>
struct Apply<T, std::tuple<Ts...>> {
  using type = std::tuple<T<Ts>...>;
};
template <template <class...> class T, class D>
using Apply_t = typename Apply<T, D>::type;

template<template <typename...> class, typename...>
struct Filter;
template<template <typename...> class _Filter>
struct Filter<_Filter, std::tuple<>>
{
  using type = std::tuple<>;
};
template<template <typename...> class _Filter, typename _T0, typename... _Ts>
struct Filter<_Filter, std::tuple<_T0, _Ts...>>
{
private:
  using rest_t = typename Filter<_Filter, std::tuple<_Ts...>>::type;
public:
  using type = std::conditional_t<_Filter<_T0>::value,
                        PrependType_t<_T0, rest_t>,
                        rest_t>;
};
template<template <typename...> class _Filter, typename _Tuple>
using Filter_t = typename Filter<_Filter, _Tuple>::type;

//--------------------------------------------------------------------------

template<size_t _I, typename... _Ts>
struct TypeIndex_impl;
template<size_t _I, typename _T, typename _T0, typename... _Ts>
struct TypeIndex_impl<_I, _T, _T0, _Ts...>
{
  static constexpr auto value =
    std::conditional_t<std::is_same<_T, _T0>::value,
                      std::integral_constant<size_t, _I>,
                      TypeIndex_impl<_I + 1, _T, _Ts...>>::value;
};

template<typename... _Ts>
struct TypeIndex;
template<typename _T, typename... _Ts>
struct TypeIndex<_T, std::tuple<_Ts...>>
{
  static constexpr auto value = TypeIndex_impl<0, _T, _Ts...>::value;
};

//--------------------------------------------------------------------------

template<template <typename...> class, typename, typename>
struct TupleIndexFilter;
template<template <typename...> class _Filter, typename _Tuple>
struct TupleIndexFilter<_Filter, _Tuple, std::index_sequence<>>
{
  using type = std::index_sequence<>;
};
template<template <typename...> class _Filter, typename _Tuple,
          size_t _I0, size_t... _Is>
struct TupleIndexFilter<_Filter, _Tuple, std::index_sequence<_I0, _Is...>>
{
private:
  using rest_t = typename TupleIndexFilter<_Filter, _Tuple,
                                            std::index_sequence<_Is...>
                                          >::type;
public:
  using type = std::conditional_t<_Filter<
                                  typename std::tuple_element_t<_I0, _Tuple>
                                >::value,
                        PrependIndex_t<_I0, rest_t>,
                        rest_t>;
};
template<template <typename...> class _Filter, typename _Tuple>
using TupleIndexFilter_t = typename TupleIndexFilter<_Filter, _Tuple,
                std::make_index_sequence<std::tuple_size<_Tuple>::value>>::type;

//--------------------------------------------------------------------------

/// @return tuple of references to a subset of the original tuple.
template<typename _Tuple, size_t... _Is>
constexpr auto tuple_ref_selection(const _Tuple& tuple, std::index_sequence<_Is...>)
{
  return std::make_tuple(std::ref(std::get<_Is>(tuple))...);
}

//--------------------------------------------------------------------------

template<typename _T>
struct IsPolicy
{
  static constexpr bool value{std::is_base_of<Policy, std::decay_t<_T>>::value};
};

template<typename _T>
struct IsNotPolicy
{
  static constexpr bool value{!std::is_base_of<Policy, std::decay_t<_T>>::value};
};

template<typename _T>
struct IsObserver
{
  static constexpr bool value{std::is_base_of<Observer, std::decay_t<_T>>::value};
};

//--------------------------------------------------------------------------

template<bool>
struct NotifyObserverImpl;

template<>
struct NotifyObserverImpl<true>
{
  template<typename _Deps, typename _Event>
  static void event(const _Deps&)
  {
  }

  template<typename _Deps, typename TGuard>
  static void guard(const _Deps&, const TGuard&, const bool)
  {
  }

  template<typename _Deps, typename TAction>
  static void action(const _Deps&, const TAction&)
  {
  }

  template<typename _Deps, typename TSrcState, typename TDstState>
  static void state_change(const _Deps&)
  {
  }
};

template<>
struct NotifyObserverImpl<false>
{
  template<typename _Deps, typename _Event>
  static void event(_Deps& deps)
  {
    get_observer(deps).template event<std::decay_t<_Event>>();
  }

  template<typename _Deps, typename _Guard>
  static void guard(_Deps& deps, const _Guard& grd, const bool result)
  {
    if (!std::is_same<std::decay_t<_Guard>,
                      std::decay_t<decltype(always_true_guard)>>::value)
    {
      get_observer(deps).template guard<std::decay_t<_Guard>>(grd, result);
    }
  }

  template<typename _Deps, typename _Action>
  static void action(_Deps& deps, const _Action& actn)
  {
    if (!std::is_same<std::decay_t<_Action>,
                      std::decay_t<decltype(no_action)>>::value)
    {
      get_observer(deps).template action<std::decay_t<_Action>>(actn);
    }
  }

  template<typename _Deps, typename _SrcState, typename _DstState>
  static void state_change(_Deps& deps)
  {
    get_observer(deps).template state_change<std::decay_t<_SrcState>,
                                              std::decay_t<_DstState>>();
  }

private:
  template<typename _Deps>
  static auto& get_observer(_Deps& deps)
  {
    using indices_t = TupleIndexFilter_t<IsObserver, _Deps>;
    return std::get<0>(tuple_ref_selection(deps, indices_t{}));
  }
};

template<typename _Deps>
struct NotifyObserver : NotifyObserverImpl<
                          std::tuple_size<
                                    typename Filter<IsObserver, _Deps>::type
                                >::value == 0
                        > {};

//--------------------------------------------------------------------------

template<typename _F>
struct CallableImpl;
template<typename _Ret, typename... _Args>
struct CallableImpl<_Ret(_Args...)>
{
  using ret_t = _Ret;
  using args_t = std::tuple<_Args...>;
};
template<typename _Ret, typename... _Args>
struct CallableImpl<_Ret(*)(_Args...)> : CallableImpl<_Ret(_Args...)> {};
template<typename _Ret, typename... _Args>
struct CallableImpl<_Ret(* const)(_Args...)> : CallableImpl<_Ret(_Args...)> {};
template<typename _Ret, typename _T, typename... _Args>
struct CallableImpl<_Ret(_T::*)(_Args...)>
  : CallableImpl<_Ret(_Args...)> {};
template<typename _Ret, typename _T, typename... _Args>
struct CallableImpl<_Ret(_T::*)(_Args...) const>
  : CallableImpl<_Ret(_Args...)> {};
template<typename _Ret, typename _T, typename... _Args>
struct CallableImpl<_Ret(_T::*)(_Args...) volatile>
  : CallableImpl<_Ret(_Args...)> {};
template<typename _Ret, typename _T, typename... _Args>
struct CallableImpl<_Ret(_T::*)(_Args...) const volatile>
  : CallableImpl<_Ret(_Args...)> {};
#if (__cplusplus > 201402L && __cpp_noexcept_function_type >= 201510) && !defined(_MSC_VER)
template<typename _Ret, typename... _Args>
struct CallableImpl<_Ret(*)(_Args...) noexcept> : CallableImpl<_Ret(_Args...)> {};
template<typename _Ret, typename _T, typename... _Args>
struct CallableImpl<_Ret(_T::*)(_Args...) noexcept>
  : CallableImpl<_Ret(_Args...)> {};
template<typename _Ret, typename _T, typename... _Args>
struct CallableImpl<_Ret(_T::*)(_Args...) const noexcept>
  : CallableImpl<_Ret(_Args...)> {};
template<typename _Ret, typename _T, typename... _Args>
struct CallableImpl<_Ret(_T::*)(_Args...) volatile noexcept>
  : CallableImpl<_Ret(_Args...)> {};
template<typename _Ret, typename _T, typename... _Args>
struct CallableImpl<_Ret(_T::*)(_Args...) const volatile noexcept>
  : CallableImpl<_Ret(_Args...)> {};
#endif
template<typename _T>
struct CallableImpl : CallableImpl<decltype(&_T::operator())> {};

template<typename _F>
struct Callable : CallableImpl<_F> {};

//--------------------------------------------------------------------------

template<typename _T, typename = void>
struct IsCallable
{
  static constexpr bool value{
      (std::is_pointer<_T>::value &&
          std::is_function<std::remove_pointer_t<_T>>::value)
      ||
      std::is_member_function_pointer<_T>::value
    };
};

template<typename _T>
struct IsCallable<_T, typename std::enable_if<
    std::is_same<decltype(void(&_T::operator())), void>::value
    >::type> : std::true_type { };

//--------------------------------------------------------------------------

template<typename, bool>
struct IsGuardImpl;
template<typename _F>
struct IsGuardImpl<_F, true>
{
  static constexpr bool value{std::is_same<typename Callable<_F>::ret_t, bool>::value};
};
template<typename _F>
struct IsGuardImpl<_F, false>
{
  static constexpr bool value{false};
};
template<typename _F>
struct IsGuard : IsGuardImpl<_F, IsCallable<_F>::value> {};

//--------------------------------------------------------------------------

template<typename, bool>
struct IsActionImpl;
template<typename _F>
struct IsActionImpl<_F, true>
{
  static constexpr bool value{std::is_same<typename Callable<_F>::ret_t, void>::value};
};
template<typename _F>
struct IsActionImpl<_F, false>
{
  static constexpr bool value{false};
};
template<typename _F>
struct IsAction : IsActionImpl<_F, IsCallable<_F>::value> {};

//--------------------------------------------------------------------------

/// makes a lambda wrapper `[](Klass& k){...}` around a member function pointer
template<typename _Ret, typename _Klass, typename... _Args>
auto method_callee(_Ret(_Klass::*ptr)(_Args...))
{
  return [ptr](_Klass& k)
          noexcept(noexcept(((k).*(ptr))()))
          { return ((k).*(ptr))(); };
}
template<typename _Ret, typename _Klass, typename... _Args>
auto method_callee(_Ret(_Klass::*ptr)(_Args...) const)
{
  return [ptr](const _Klass& k)
          noexcept(noexcept(((k).*(ptr))()))
          { return ((k).*(ptr))(); };
}
template<typename _Ret, typename _Klass, typename... _Args>
auto method_callee(_Ret(_Klass::*ptr)(_Args...) volatile)
{
  return [ptr](_Klass& k)
          noexcept(noexcept(((k).*(ptr))()))
          { return ((k).*(ptr))(); };
}
template<typename _Ret, typename _Klass, typename... _Args>
auto method_callee(_Ret(_Klass::*ptr)(_Args...) const volatile)
{
  return [ptr](const _Klass& k)
          noexcept(noexcept(((k).*(ptr))()))
          { return ((k).*(ptr))(); };
}

//--------------------------------------------------------------------------

template<typename, bool>
struct UnifyCalleeImpl;
template<typename _F>
struct UnifyCalleeImpl<_F, true>
{ static auto unify(_F f) { return method_callee(f); } };
template<typename _F>
struct UnifyCalleeImpl<_F, false>
{ static auto unify(_F f) { return std::move(f); } };

template<typename _F>
struct UnifyCallee
  : UnifyCalleeImpl<_F, std::is_member_function_pointer<_F>::value>
{};

//--------------------------------------------------------------------------

template<typename _F, typename _Deps, size_t... _Is>
auto call_impl(_F func, _Deps& deps, std::index_sequence<_Is...>)
{
  return func(std::get<_Is>(deps)...);
}

template<typename _F, typename _Deps>
auto call(_F func, _Deps& deps)
{
  using nonpolicy_indices_t = TupleIndexFilter_t<IsNotPolicy, _Deps>;
  auto filtered_deps = tuple_ref_selection(deps, nonpolicy_indices_t{});
  using args_t = typename Callable<_F>::args_t;
  using args_indices_t = std::make_index_sequence<std::tuple_size<args_t>::value>;
  return call_impl(func, filtered_deps, args_indices_t{});
}

//--------------------------------------------------------------------------

template<typename...>
struct OpNotImpl;
template<typename _F, typename... Args>
struct OpNotImpl<_F, std::tuple<Args...>>
{
  explicit OpNotImpl(_F f) : m_f{std::move(f)} {}

  bool operator()(Args&&... args) const
  {
    return ! m_f(std::forward<Args>(args)...);
  }

private:
  const _F m_f;
};

template<typename _F>
struct OpNot : OpNotImpl<_F, typename Callable<_F>::args_t>
{
  using super = OpNotImpl<_F, typename Callable<_F>::args_t>;
  using super::super;
};

//--------------------------------------------------------------------------

struct ExprAnd
{
  static constexpr bool eval(const bool v1, const bool v2) noexcept
  {
    return v1 && v2;
  }
};

struct ExprOr
{
  static constexpr bool eval(const bool v1, const bool v2) noexcept
  {
    return v1 || v2;
  }
};

//--------------------------------------------------------------------------

template<typename...>
struct OpBinaryImpl;
template<typename _Expr, typename _F1, typename _F2, typename... Args1, typename... Args2>
struct OpBinaryImpl<_Expr, _F1, std::tuple<Args1...>, _F2, std::tuple<Args2...>>
{
  explicit OpBinaryImpl(_F1 f1, _F2 f2)
    : m_f1{std::move(f1)}, m_f2{std::move(f2)}
  {}

  constexpr bool operator()(Args1&&... args) const
  {
    auto tup = std::forward_as_tuple(args...);
    return _Expr::eval(call(m_f1, tup), call(m_f2, tup));
  }

private:
  const _F1 m_f1;
  const _F2 m_f2;
};

//--------------------------------------------------------------------------

template<typename _F1, typename _F2>
struct OpAnd : OpBinaryImpl<ExprAnd,
                          _F1, typename Callable<_F1>::args_t,
                          _F2, typename Callable<_F2>::args_t>
{
public:
  using super = OpBinaryImpl<ExprAnd,
                              _F1, typename Callable<_F1>::args_t,
                              _F2, typename Callable<_F2>::args_t>;
  using super::super;
};

//--------------------------------------------------------------------------

template<typename _F1, typename _F2>
struct OpOr : OpBinaryImpl<ExprOr,
                          _F1, typename Callable<_F1>::args_t,
                          _F2, typename Callable<_F2>::args_t>
{
public:
  using super = OpBinaryImpl<ExprOr,
                              _F1, typename Callable<_F1>::args_t,
                              _F2, typename Callable<_F2>::args_t>;
  using super::super;
};

//--------------------------------------------------------------------------

template<typename...>
struct ActionBatchImpl;
template<typename _Expr, typename _F1, typename _F2, typename... Args1, typename... Args2>
struct ActionBatchImpl<_Expr, _F1, std::tuple<Args1...>, _F2, std::tuple<Args2...>>
{
  explicit ActionBatchImpl(_F1 f1, _F2 f2)
    : m_f1{std::move(f1)}, m_f2{std::move(f2)}
  {}

  constexpr void operator()(Args1&&... args) const
  {
    auto tup = std::forward_as_tuple(args...);
    call(m_f1, tup);
    call(m_f2, tup);
  }

private:
  const _F1 m_f1;
  const _F2 m_f2;
};

template<typename _F1, typename _F2>
struct ActionBatch : ActionBatchImpl<ExprOr,
                          _F1, typename Callable<_F1>::args_t,
                          _F2, typename Callable<_F2>::args_t>
{
public:
  using super = ActionBatchImpl<ExprOr,
                              _F1, typename Callable<_F1>::args_t,
                              _F2, typename Callable<_F2>::args_t>;
  using super::super;
};

//--------------------------------------------------------------------------

struct anonymous_t { static auto c_str() noexcept { return "anonymous"; } };
struct on_entry_t { static auto c_str() noexcept { return "on_entry"; } };
struct on_exit_t { static auto c_str() noexcept { return "on_exit"; } };
struct unexpected_t { static auto c_str() noexcept { return "unexpected"; } };
struct initial_t { static auto c_str() noexcept { return "initial"; } };
struct final_t { static auto c_str() noexcept { return "final"; } };

//--------------------------------------------------------------------------

template<typename _T>
struct IsStateActionEvent
{
  static constexpr auto value = std::is_same<_T, Event<on_entry_t>>::value ||
                                std::is_same<_T, Event<on_exit_t>>::value;
};

//--------------------------------------------------------------------------

/// Get state number by type from states tuple.
template<typename _State, typename _StateList>
static constexpr size_t state_number_v = detail::TypeIndex<_State, _StateList>::value;

//--------------------------------------------------------------------------

template<typename _Row>
using RowSrcState_t = typename _Row::src_state_t;
template<typename _Row>
using RowDstState_t = typename _Row::dst_state_t;
template<typename _Rows>
using CollectStates_t = UniqueTypes_t<ConcatTypes_t<
                      Apply_t<RowSrcState_t, _Rows>,
                      Apply_t<RowDstState_t, _Rows>
                    >>;

//--------------------------------------------------------------------------

template<typename _Rows, typename _Event>
struct RowsWithEventIndices
{
  template<typename _Row>
  struct filter_t : std::is_same<typename std::decay_t<_Row>::event_t, _Event> {};
  using indices_t = TupleIndexFilter_t<filter_t, _Rows>;
};

/// @return tuple of references to rows where the event is present
template<typename _Rows, typename _Event>
constexpr auto rows_with_event(const _Rows& rows, const _Event&)
{
  using indices_t = typename RowsWithEventIndices<std::decay_t<_Rows>, _Event>::indices_t;
  return tuple_ref_selection(rows, indices_t{});
}

//--------------------------------------------------------------------------

template<typename _Rows, typename _State>
struct RowsWithDstStateIndices
{
  template<typename _Row>
  struct filter_t : std::is_same<typename std::decay_t<_Row>::dst_state_t, _State> {};
  using indices_t = TupleIndexFilter_t<filter_t, _Rows>;
};

/// @return tuple of references to rows where the event is present
template<typename _Rows, typename _State>
constexpr auto rows_with_dst_state(const _Rows& rows, const _State&)
{
  using indices_t = typename RowsWithDstStateIndices<std::decay_t<_Rows>, _State>::indices_t;
  return tuple_ref_selection(rows, indices_t{});
}

//==========================================================================

template<typename...>
struct GetCurrentStateNameImpl;
template<typename _AllStates>
struct GetCurrentStateNameImpl<_AllStates, std::tuple<>>
{
  auto operator()(size_t) const noexcept
  {
    return "N/A";
  }
};
template<typename _AllStates, typename _S0, typename... _Ss>
struct GetCurrentStateNameImpl<_AllStates, std::tuple<_S0, _Ss...>>
{
  auto operator()(size_t n) const noexcept
  {
    if (n == TypeIndex<_S0, _AllStates>::value)
    {
      return detail::c_str<typename _S0::base_t>();
    } else {
      return GetCurrentStateNameImpl<_AllStates, std::tuple<_Ss...>>{}(n);
    }
  }
};

template<typename _AllStates>
struct GetCurrentStateName
{
  auto operator()(size_t n) const noexcept
  {
    return GetCurrentStateNameImpl<_AllStates, _AllStates>{}(n);
  }
};

//==========================================================================
}} // namespace
//==========================================================================

/// Groups together event, guard and action as a unification for the table row.
template<typename _Event, typename _GuardF, typename _ActionF>
struct EventBundle
{
  using event_t = std::remove_cv_t<_Event>;
  using guard_t = _GuardF;
  using action_t = _ActionF;

  explicit constexpr EventBundle(_GuardF gf, _ActionF af) noexcept
    : m_guard{std::move(gf)}, m_action{std::move(af)}
  {}

  template<typename _F,
          _DSML_REQUIRES(detail::IsAction<_F>::value)>
  constexpr auto operator/(_F action) const noexcept
  {
    return EventBundle<_Event, _GuardF, _F>{m_guard, std::move(action)};
  }

  const _GuardF m_guard{};
  const _ActionF m_action{};
};

template<typename _T>
struct Event
{
  using base_t = _T;

  explicit constexpr Event() = default;

  template<typename _ActionF,
          _DSML_REQUIRES(detail::IsAction<_ActionF>::value)>
  constexpr auto operator/(_ActionF action) const noexcept
  {
    return EventBundle<Event<_T>, decltype(detail::always_true_guard), _ActionF>{
                                detail::always_true_guard, std::move(action)
                              };
  }

  template<typename _GuardF,
          _DSML_REQUIRES(detail::IsGuard<_GuardF>::value)>
  constexpr auto operator[](_GuardF guard) const noexcept
  {
    return EventBundle<Event<_T>, _GuardF, decltype(detail::no_action)>{
                std::move(guard), detail::no_action
              };
  }

  static auto c_str() noexcept
  {
    return detail::c_str<base_t>();
  };
};

//==========================================================================
namespace detail { namespace {
//==========================================================================

template<typename _Rows, typename _Deps>
static void call_row_action(const _Rows& rows, _Deps& deps, std::true_type)
{
  call(std::get<0>(rows).m_action, deps);
}

template<typename _Rows, typename _Deps>
static void call_row_action(const _Rows&, _Deps&, std::false_type)
{
  // do nothing
}

/// Go through rows and try to match it against current state and filter by
/// guards.
template<typename...>
struct ProcessSingleEventImpl;
template<typename _AllStates, typename _AllRows, typename _Deps, typename _StateNum,
          typename _FilteredRows>
struct ProcessSingleEventImpl<_AllStates, _AllRows, _Deps, _StateNum, _FilteredRows,
                                std::index_sequence<>>
{
  static constexpr bool process(const _AllRows&, const _FilteredRows&,
                                _StateNum&, _Deps&) noexcept
  {
    return false;
  }
};
template<typename _AllStates, typename _AllRows, typename _Deps, typename _StateNum,
          typename _FilteredRows, size_t _I0, size_t... _Is>
struct ProcessSingleEventImpl<_AllStates, _AllRows, _Deps, _StateNum,
                              _FilteredRows, std::index_sequence<_I0, _Is...>>
{
  static constexpr bool process(const _AllRows& all_rows,
                                const _FilteredRows& filtered_rows,
                                _StateNum& state,
                                _Deps& deps)
  {
    using row_t = std::remove_const_t<std::remove_reference_t<
                          std::tuple_element_t<_I0, _FilteredRows>
                        >>;
    const auto& row = std::get<_I0>(filtered_rows);
    bool processed{false};
    constexpr auto source_state =
                        state_number_v<typename row_t::src_state_t, _AllStates>;
    if (source_state == state)
    {
      const auto& guard = row.m_guard;
      const auto allowed = call(guard, deps);

      detail::NotifyObserver<_Deps>::template guard<_Deps, decltype(guard)>
                                                        (deps, guard, allowed);

      if (allowed)
      {
        constexpr auto destination_state
            = state_number_v<typename row_t::dst_state_t, _AllStates>;
        if (state != destination_state)
        {
          state = destination_state;
        }

        // on exit action
        const auto exit_rows = detail::rows_with_dst_state(
                              detail::rows_with_event(all_rows, Event<on_exit_t>{}),
                              typename row_t::src_state_t{});
        call_row_action(exit_rows, deps,
                        std::conditional_t<IsEmptyTuple<decltype(exit_rows)>::value,
                                           std::false_type, std::true_type>{});

        // event action
        detail::NotifyObserver<_Deps>
                  ::template action<_Deps, decltype(row.m_action)>
                                              (deps, row.m_action);
        call(row.m_action, deps);

        detail::NotifyObserver<_Deps>::template state_change<
                                          _Deps,
                                          typename row_t::src_state_t,
                                          typename row_t::dst_state_t>(deps);

        // on entry action
        const auto entry_rows = detail::rows_with_dst_state(
                              detail::rows_with_event(all_rows, Event<on_entry_t>{}),
                              typename row_t::dst_state_t{});
        call_row_action(entry_rows, deps,
                          std::conditional_t<IsEmptyTuple<decltype(entry_rows)>::value,
                                            std::false_type, std::true_type>{});
        processed = true;
      }
    }

    return processed ||
          ProcessSingleEventImpl<_AllStates, _AllRows, _Deps, _StateNum,
                                _FilteredRows, std::index_sequence<_Is...>>
              ::process(all_rows, filtered_rows, state, deps);
  }
};

//==========================================================================

template<typename...>
struct HasTableOperatorHelper;
template<typename _T>
struct HasTableOperatorHelper<TransitionTable<_T>> {};

/// type trait to detect a SM declaration
/// (a struct with operator() returning a table)
template <typename T, typename = int>
struct HasTableOperator : std::false_type { };
template <typename T>
struct HasTableOperator<T, decltype(
                      HasTableOperatorHelper<decltype(std::declval<T>()())>{},
                      0
                  )> : std::true_type { };

//--------------------------------------------------------------------------

template<typename _MachineDecl>
constexpr auto transition_table_from_machine_declaration()
{
  static_assert(HasTableOperator<_MachineDecl>::value, "must be a SM declaration");
  return _MachineDecl{}();
}

//--------------------------------------------------------------------------

template<typename _State, typename _Wrap>
struct WrapState
{
  static_assert(IsState<_State>::value, "");
  using type = State<typename _State::base_t,
                      PrependType_t<_Wrap, typename _State::tags_t>>;
};

template<typename _State, typename... _Ws>
struct WrapStateInLayers
{
  static_assert(IsState<_State>::value, "");
  using type = State<typename _State::base_t,
                      ConcatTypes_t<std::tuple<_Ws...>, typename _State::tags_t>>;
};

//--------------------------------------------------------------------------

template<typename _Tag, typename _Row>
constexpr auto wrap_row(const _Row& row)
{
  return TableRow<
              typename WrapState<typename _Row::src_state_t, _Tag>::type,
              typename _Row::event_t,
              typename _Row::guard_t,
              typename _Row::action_t,
              typename WrapState<typename _Row::dst_state_t, _Tag>::type
            >(row.m_guard, row.m_action);
}

template<typename _Tag, typename _Rows, size_t... _Is>
constexpr auto wrap_states_impl(const _Rows& rows, TypeHolder<_Tag>,
                                std::index_sequence<_Is...>)
{
  return std::make_tuple(wrap_row<_Tag>(std::get<_Is>(rows))...);
}

/// @return tuple of copied rows that contain wrapped states
template<typename _Tag, typename _Rows>
constexpr auto wrap_states(const _Rows& rows)
{
  using indices_t = std::make_index_sequence<std::tuple_size<_Rows>::value>;
  return wrap_states_impl(rows, TypeHolder<_Tag>{}, indices_t{});
}

//--------------------------------------------------------------------------

template<typename _State, typename _PointType>
struct WrapSubPoint
{
  using type = std::conditional_t<
                HasTableOperator<typename _State::base_t>::value,
                State<_PointType, std::tuple<typename _State::base_t>>,
                _State
              >;
};

//==========================================================================

template<typename _Row>
using RowSrcStateRaw_t = typename _Row::src_state_raw_t::base_t;
template<typename _Row>
using RowDstStateRaw_t = typename _Row::dst_state_raw_t::base_t;
/// Collect machine types from a transition table rows.
template<typename _Rows>
using CollectSubmachineTypes_t = UniqueTypes_t<Filter_t<HasTableOperator,
                                    ConcatTypes_t<
                                      Apply_t<RowSrcStateRaw_t, _Rows>,
                                      Apply_t<RowDstStateRaw_t, _Rows>
                                    >>
                                  >;

//--------------------------------------------------------------------------

template<typename _Rows>
constexpr auto make_transition_table_from_tuple(_Rows rows)
{
  return TransitionTable<_Rows>{std::move(rows)};
}

template<typename _MachineDecl>
constexpr auto expand_table();

/// Expand and concatenate sub-machines.
template<typename...>
struct AddSubmachines;
template<typename... _STs>
struct AddSubmachines<std::tuple<_STs...>>
{
  static constexpr auto add()
  {
    return std::tuple_cat(wrap_states<_STs>(expand_table<_STs>().m_rows)...);
  }
};

/// Generate recursively a big table for a composite machine with expanded
/// sub-machines and well connected entry and exit points.
template<typename _MachineDecl>
constexpr auto expand_table()
{
  const auto table_base = transition_table_from_machine_declaration<_MachineDecl>();
  return make_transition_table_from_tuple(std::tuple_cat(
            table_base.m_rows,
            AddSubmachines<typename decltype(table_base)::submachine_types_t>::add()
          ));
}

//==========================================================================
}} // namespace
//==========================================================================

template<typename _SrcS, typename _EventBundle>
struct StateTransition;

template<typename _Base, typename _Tags=std::tuple<>>
struct State
{
  using base_t = _Base;
  using tags_t = _Tags;

  explicit constexpr State() = default;

  template<typename _E>
  constexpr auto operator+(const Event<_E>&) const noexcept
  {
    must_be_non_final<base_t>();

    using eb_t = EventBundle<Event<_E>,
                                decltype(detail::always_true_guard),
                                decltype(detail::no_action)>;
    return StateTransition<State<base_t>, eb_t>{
                  eb_t{detail::always_true_guard, detail::no_action}
                };
  }

  template<typename _E, typename _GuardF, typename _ActionF>
  constexpr auto operator+(const EventBundle<_E, _GuardF, _ActionF>& eb) const noexcept
  {
    must_be_non_final<base_t>();

    using tag_t = std::conditional_t<
                detail::IsStateActionEvent<
                                  typename std::decay_t<decltype(eb)>::event_t
                                >::value,
                detail::TypeHolder<state_action_t>,
                detail::TypeHolder<transition_action_t>>;
    return combine_event_bundle<_E, _GuardF, _ActionF>(eb, tag_t{});
  }

  template<typename _F,
          _DSML_REQUIRES(detail::IsGuard<_F>::value)>
  constexpr auto operator[](_F guard) const noexcept
  {
    must_be_non_final<base_t>();
    return *this + Event<detail::anonymous_t>{} [ guard ];
  }

  template<typename _F,
          _DSML_REQUIRES(detail::IsAction<_F>::value)>
  constexpr auto operator/(_F action) const noexcept
  {
    must_be_non_final<base_t>();
    return *this + Event<detail::anonymous_t>{} / action;
  }

  template<typename _DstS>
  constexpr auto operator=(const State<_DstS>& dst) const noexcept
  {
    must_be_non_final<base_t>();
    return *this + Event<detail::anonymous_t>{} = dst;
  }

  static auto c_str() noexcept
  {
    return detail::c_str<base_t>();
  };

private:
  struct transition_action_t {};
  struct state_action_t {};

  template<typename _E, typename _GuardF, typename _ActionF>
  constexpr auto combine_event_bundle(const EventBundle<_E, _GuardF, _ActionF>& eb,
                            detail::TypeHolder<transition_action_t>) const noexcept
  {
    return StateTransition<State<base_t>, EventBundle<_E, _GuardF, _ActionF>>{eb};
  }

  template<typename _E, typename _GuardF, typename _ActionF>
  constexpr auto combine_event_bundle(const EventBundle<_E, _GuardF, _ActionF>& eb,
                            detail::TypeHolder<state_action_t>) const noexcept
  {
    // make a loop to itself to have unified table rows
    return StateTransition<State<base_t>, EventBundle<_E, _GuardF, _ActionF>>{eb} = State{};
  }

  // just for a static check
  template<typename _B>
  static void must_be_non_final()
  {
    static_assert(not std::is_same<_B, detail::final_t>::value,
                  "can't add an event to a final state");
  }
};

//==========================================================================

template<typename _SrcS, typename _Event, typename _Guard, typename _Action,
          typename _DstS>
struct TableRow
{
  static_assert(IsState<_SrcS>::value, "");
  static_assert(IsEvent<_Event>::value, "");
  static_assert(detail::IsAction<_Action>::value, "");
  static_assert(detail::IsGuard<_Guard>::value, "");
  static_assert(IsState<_DstS>::value, "");

  // map substate as a source state to its final state
  using src_state_t = typename detail::WrapSubPoint<_SrcS, detail::final_t>::type;
  using src_state_raw_t = _SrcS;
  using event_t = _Event;
  using guard_t = _Guard;
  using action_t = _Action;
  // map substate as a destination state to its initial state
  using dst_state_t = typename detail::WrapSubPoint<_DstS, detail::initial_t>::type;
  using dst_state_raw_t = _DstS;

  explicit constexpr TableRow(_Guard guard, _Action action)
    : m_guard{std::move(guard)}, m_action{std::move(action)}
  {}

  const _Guard m_guard;
  const _Action m_action;
};

template<typename _SrcS, typename _EventBundle>
struct StateTransition
{
  static_assert(IsState<_SrcS>::value, "");

  explicit constexpr StateTransition(_EventBundle event_bundle)
    : m_event_bundle{event_bundle}
  {}

  template<typename _DstS>
  constexpr auto operator=(const State<_DstS>&) const noexcept
  {
    return TableRow<_SrcS,
                    typename _EventBundle::event_t,
                    typename _EventBundle::guard_t,
                    typename _EventBundle::action_t,
                    State<_DstS>>
                {m_event_bundle.m_guard, m_event_bundle.m_action};
  }

  template<typename _F,
          _DSML_REQUIRES(detail::IsAction<_F>::value)>
  constexpr auto operator/(_F action) const noexcept
  {
    auto new_bundle = EventBundle<typename _EventBundle::event_t,
                                  typename _EventBundle::guard_t,
                                  _F>
                          {m_event_bundle.m_guard, std::move(action)};
    return StateTransition<_SrcS, decltype(new_bundle)>{std::move(new_bundle)};
  }

  const _EventBundle m_event_bundle{};
};

template<typename _Rows>
struct TransitionTable
{
  using states_t = detail::CollectStates_t<_Rows>;
  using rows_t = _Rows;
  using submachine_types_t = detail::CollectSubmachineTypes_t<_Rows>;

  static_assert(std::tuple_size<states_t>::value > 0,
                "table must have at least 1 state");
  static_assert(detail::HasType<State<detail::initial_t>, states_t>::value,
                "table must have initial state");

  explicit constexpr TransitionTable(const _Rows& rows) noexcept
    : m_rows{rows}
  {}

  const rows_t m_rows;
};

template<typename... _Ts>
constexpr auto make_transition_table(_Ts... rows)
{
  return TransitionTable<std::tuple<_Ts...>>{std::tuple<_Ts...>{rows...}};
}

//==========================================================================

static constexpr auto initial_state = State<detail::initial_t>{};
static constexpr auto final_state = State<detail::final_t>{};
static constexpr auto anonymous_event = Event<detail::anonymous_t>{};
static constexpr auto on_entry = Event<detail::on_entry_t>{};
static constexpr auto on_exit = Event<detail::on_exit_t>{};
static constexpr auto unexpected_event = Event<detail::unexpected_t>{};

//==========================================================================

template<typename _MachineDecl, typename... _Deps>
class Sm
{
public:
  /// Dependencies are always passed as a reference. If the dependency is an
  /// rvalue then it will be moved to an internal value.
  explicit Sm(_Deps&... deps)
      : m_deps{deps...}
      , m_state_number{
            detail::TypeIndex<State<detail::initial_t>,
                      typename table_types::transition_table_t::states_t>::value}
  {
    static_assert(table_types::states_count
                  <= std::numeric_limits<state_number_t>::max(),
                  "state_number_t does not cover required states count");

    reset();
  }

  // reset the SM to its initial state
  void reset()
  {
    m_state_number =
            detail::TypeIndex<State<detail::initial_t>,
                      typename table_types::transition_table_t::states_t>::value;

    // on entry for initial_state
    const auto table = detail::expand_table<_MachineDecl>();
    const auto entry_rows = detail::rows_with_dst_state(
                          detail::rows_with_event(table.m_rows, Event<detail::on_entry_t>{}),
                          initial_state);
    detail::call_row_action(entry_rows, m_deps,
                      std::conditional_t<detail::IsEmptyTuple<decltype(entry_rows)>::value,
                                        std::false_type, std::true_type>{});

    process_anonymous_events();
  }

  template<typename _State>
  constexpr bool is(const _State&) const noexcept
  {
    constexpr auto number = detail::TypeIndex<
                              std::remove_cv_t<_State>,
                              typename table_types::transition_table_t::states_t
                            >::value;
    return m_state_number == number;
  }

  /// XXX this will be removed and somehow unified with is()
  template<typename _State, typename... _Submachines>
  constexpr bool is_sub(const _State&, const _Submachines&...) const noexcept
  {
    constexpr auto number = detail::TypeIndex<
                              typename detail::WrapStateInLayers<
                                              std::remove_cv_t<_State>,
                                              _Submachines...
                                            >::type,
                              typename table_types::transition_table_t::states_t
                            >::value;
    return m_state_number == number;
  }

  const char* get_current_state_name()
  {
    return detail::GetCurrentStateName<
        typename table_types::transition_table_t::states_t>{}(m_state_number);
  }

  template<typename _ET>
  void process_event(const Event<_ET>& event)
  {
    process_single_event(event);
    process_anonymous_events();
  }

  //--------------------------------

private:
  using deps_t = std::tuple<_Deps&...>;
  using state_number_t = uint_fast16_t;

  /// This wrapper avoids type deductions during Sm declaration.
  struct table_types
  {
    using transition_table_t = decltype(detail::expand_table<_MachineDecl>());

    static constexpr auto states_count =
        std::tuple_size<typename transition_table_t::states_t>::value;
  };

  //--------------------------------

  /// @return true if transition was executed
  template<typename _ET>
  constexpr bool process_raw_event(const Event<_ET>& evt)
  {
    const auto table = detail::expand_table<_MachineDecl>();
    auto filtered_rows = detail::rows_with_event(table.m_rows, evt);
    using frows_t = decltype(filtered_rows);
    detail::NotifyObserver<deps_t>::template event<deps_t, decltype(evt)>(m_deps);
    return detail::ProcessSingleEventImpl<
                            typename table_types::transition_table_t::states_t,
                            decltype(table.m_rows),
                            deps_t,
                            state_number_t,
                            frows_t,
                            std::make_index_sequence<std::tuple_size<frows_t>::value>
                          >
                    ::process(table.m_rows, filtered_rows, m_state_number, m_deps);
  }

  //--------------------------------

  /// @return true if transition was executed
  template<typename _ET>
  constexpr bool process_single_event(const Event<_ET>& evt)
  {
    return process_raw_event(evt) or process_raw_event(unexpected_event);
  }

  //--------------------------------

  void process_anonymous_events()
  {
    while (process_raw_event(Event<detail::anonymous_t>{}))
    { }
  }

  //--------------------------------

  deps_t m_deps;
  /// actual state machine state
  state_number_t m_state_number{};
};

//==========================================================================

/// Convert a callable to a form usable as a guard or an action. Lambdas and
/// function pointers will be passed as is and member function pointers will be
/// converted to a lambda in a form `[](Klass& k){ return ((k).*(ptr))(); }`.
template<typename _F>
auto callee(_F f)
{
  return detail::UnifyCallee<_F>::unify(std::move(f));
}

//==========================================================================

/// Simple type introspection without RTTI.
template <typename T>
auto get_type_name() {
#if defined(_MSC_VER)
  using seq_t = std::make_index_sequence<sizeof(__FUNCSIG__) - 33 - 8>;
  return detail::get_type_name_impl<T, 33>(__FUNCSIG__, seq_t{});
#elif defined(__clang__)
  using seq_t = std::make_index_sequence<sizeof(__PRETTY_FUNCTION__) - 32 - 2>;
  return detail::get_type_name_impl<T, 32>(__PRETTY_FUNCTION__, seq_t{});
#elif defined(__GNUC__)
  using seq_t = std::make_index_sequence<sizeof(__PRETTY_FUNCTION__) - 37 - 2>;
  return detail::get_type_name_impl<T, 37>(__PRETTY_FUNCTION__, seq_t{});
#endif
}

//==========================================================================

namespace literals {

#ifndef _MSC_VER

template<typename _T, _T... Chrs>
auto operator""_s() {
  return State<detail::CString<char, Chrs...>>{};
}

template<typename _T, _T... Chrs>
auto operator""_e() {
  return Event<detail::CString<char, Chrs...>>{};
}

#endif

}

//==========================================================================

namespace operators {

template<typename _F, _DSML_REQUIRES(detail::IsGuard<_F>::value)>
auto operator!(_F func)
{
  return detail::OpNot<_F>{std::move(func)};
}

template<typename _F1, typename _F2,
        _DSML_REQUIRES(detail::IsGuard<_F1>::value &&
                        detail::IsGuard<_F2>::value)>
auto operator&&(_F1 func1, _F2 func2)
{
  return detail::OpAnd<_F1, _F2>{std::move(func1), std::move(func2)};
}

template<typename _F1, typename _F2,
        _DSML_REQUIRES(detail::IsGuard<_F1>::value &&
                        detail::IsGuard<_F2>::value)>
auto operator||(_F1 func1, _F2 func2)
{
  return detail::OpOr<_F1, _F2>{std::move(func1), std::move(func2)};
}

template<typename _F1, typename _F2,
        _DSML_REQUIRES(detail::IsAction<_F1>::value &&
                        detail::IsAction<_F2>::value)>
auto operator,(_F1 func1, _F2 func2)
{
  return detail::ActionBatch<_F1, _F2>{std::move(func1), std::move(func2)};
}

} // namespace

//==========================================================================
} // namespace
//==========================================================================

#ifdef _MSC_VER
  #pragma warning(pop)
#else
  #pragma GCC diagnostic pop
#endif

#endif /* include guard */
