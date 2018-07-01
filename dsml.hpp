#pragma once
#ifndef DSML_HPP__O06IR34S
#define DSML_HPP__O06IR34S

#include <functional>
#include <limits>
#include <tuple>
#include <type_traits>

//==========================================================================
namespace dsml {
//==========================================================================

template<typename>
struct TransitionTable;
template<typename, typename, typename>
struct TableRow;
template<typename>
struct State;
template<typename>
struct Event;

//--------------------------------------------------------------------------

template<typename _T>
struct IsState : std::false_type {};
template<typename _T>
struct IsState<State<_T>> : std::true_type {};
template<typename _T>
static constexpr auto is_state_v = IsState<_T>::value;

template<typename _T>
struct IsEvent : std::false_type {};
template<typename _T>
struct IsEvent<Event<_T>> : std::true_type {};
template<typename _T>
static constexpr auto is_event_v = IsEvent<_T>::value;

//==========================================================================
namespace detail {
//==========================================================================

template <typename _T, _T...>
struct CString;
template <char... _Chrs>
struct CString<char, _Chrs...> {
  using type = CString;
  static auto c_str() noexcept {
    static char str[] = {_Chrs..., 0};
    return str;
  }
};

//--------------------------------------------------------------------------
//
template <class, size_t N, size_t... Ns>
auto get_type_name_impl(const char *ptr, std::index_sequence<Ns...>) {
  static const char str[] = {ptr[N + Ns]..., 0};
  return str;
}
template <typename T>
auto get_type_name() {
#if defined(__GNUC__)
  using seq_t = std::make_index_sequence<sizeof(__PRETTY_FUNCTION__) - 45 - 2>;
  return detail::get_type_name_impl<T, 45>(__PRETTY_FUNCTION__, seq_t{});
#elif defined(__clang__)
  using seq_t = std::make_index_sequence<sizeof(__PRETTY_FUNCTION__) - 40 - 2>;
  return detail::get_type_name_impl<T, 40>(__PRETTY_FUNCTION__, seq_t{});
#endif
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
struct IsTuple : std::false_type {};
template<typename... _T>
struct IsTuple<std::tuple<_T...>> : std::true_type {};

template<typename _T>
struct IsEmptyTuple
{
  static constexpr auto value = std::tuple_size<_T>::value == 0;
};

template <typename _T, typename _Tuple>
struct HasType;
template <typename _T, typename... _Us>
struct HasType<_T, std::tuple<_Us...>> : disjunction<std::is_same<_T, _Us>...> {};

//--------------------------------------------------------------------------

template<typename _T>
struct RemoveFirstType { };
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

template<typename... _Tuples>
using ConcatTuples_t = decltype(std::tuple_cat(std::declval<_Tuples>()...));
/// concatenate subtuples
template<typename>
struct FlattenTuple;
template<typename... _Ts>
struct FlattenTuple<std::tuple<_Ts...>>
{
  using type = ConcatTuples_t<_Ts...>;
};
template<typename _Tuple>
using FlattenTuple_t = typename FlattenTuple<_Tuple>::type;

template<typename... _T>
struct UniqueTypesTuple;
template<typename... _T>
using UniqueTypesTuple_t = typename UniqueTypesTuple<_T...>::type;
template<>
struct UniqueTypesTuple<std::tuple<>>
{
  using type = std::tuple<>;
};
template<typename _T>
struct UniqueTypesTuple<std::tuple<_T>>
{
  using type = std::tuple<_T>;
};
template<typename _T0, typename... _T>
struct UniqueTypesTuple<std::tuple<_T0, _T...>>
{
private:
  using rest_t = UniqueTypesTuple_t<std::tuple<_T...>>;
public:
  using type = typename std::conditional_t<HasType<_T0, std::tuple<_T...>>::value,
                  rest_t,
                  PrependType_t<_T0, rest_t>
                >;
};

/// @return tuple of references to a subset of the original tuple.
template<typename _Tuple, size_t... _Is>
auto tuple_ref_selection(const _Tuple& tuple, std::index_sequence<_Is...>)
{
  return std::make_tuple(std::ref(std::get<_Is>(tuple))...);
}

template <template <class...> class, class>
struct Apply;
template <template <class...> class T, class... Ts>
struct Apply<T, std::tuple<Ts...>> {
  using type = std::tuple<T<Ts>...>;
};
template <template <class...> class T, class D>
using Apply_t = typename Apply<T, D>::type;

template<template <typename...> typename, typename...>
struct Filter;
template<template <typename...> typename _Filter>
struct Filter<_Filter, std::tuple<>>
{
  using type = std::tuple<>;
};
template<template <typename...> typename _Filter, typename _T0, typename... _Ts>
struct Filter<_Filter, std::tuple<_T0, _Ts...>>
{
private:
  using rest_t = typename Filter<_Filter, std::tuple<_Ts...>>::type;
public:
  using type = std::conditional_t<_Filter<_T0>::value,
                        PrependType_t<_T0, rest_t>,
                        rest_t>;
};
template<template <typename...> typename _Filter, typename _Tuple>
using Filter_t = typename Filter<_Filter, _Tuple>::type;

template<template <typename...> typename, typename, typename>
struct TupleIndexFilter;
template<template <typename...> typename _Filter, typename _Tuple>
struct TupleIndexFilter<_Filter, _Tuple, std::index_sequence<>>
{
  using type = std::index_sequence<>;
};
template<template <typename...> typename _Filter, typename _Tuple,
          size_t _I0, size_t... _Is>
struct TupleIndexFilter<_Filter, _Tuple, std::index_sequence<_I0, _Is...>>
{
private:
  using rest_t = typename TupleIndexFilter<_Filter, _Tuple,
                                            std::index_sequence<_Is...>
                                          >::type;
public:
  using type = std::conditional_t<_Filter<
                                  typename std::tuple_element<_I0, _Tuple>::type
                                >::value,
                        PrependIndex_t<_I0, rest_t>,
                        rest_t>;
};
template<template <typename...> typename _Filter, typename _Tuple>
using TupleIndexFilter_t = typename TupleIndexFilter<_Filter, _Tuple,
                std::make_index_sequence<std::tuple_size<_Tuple>::value>>::type;

//--------------------------------------------------------------------------

template<size_t _I, typename... _Ts>
struct TypeIndex_impl;
template<size_t _I, typename _T>
struct TypeIndex_impl<_I, _T>
{
};
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
  static_assert(HasType<_T, std::tuple<_Ts...>>::value,
                "type not present");
  static constexpr auto value = TypeIndex_impl<0, _T, _Ts...>::value;
};

//--------------------------------------------------------------------------

template<typename _F>
struct CallableImpl;
template<typename _Ret, typename... _Args>
struct CallableImpl<_Ret(_Args...)>
{
  using ret_t = _Ret;
  using args_t = std::tuple<_Args...>;
};
template<typename _Ret, typename _T, typename... _Args>
struct CallableImpl<_Ret(_T::*)(_Args...) const>
{
  using ret_t = _Ret;
  using args_t = std::tuple<_Args...>;
};
template<typename _T>
struct CallableImpl : CallableImpl<decltype(&_T::operator())>
{ };

/// Get callable arguments type list without CV qualifiers and references.
template<typename _F>
struct Callable : CallableImpl<_F>
{
  Callable(_F f) : m_f{std::move(f)} {}
private:
  _F m_f;
};

//--------------------------------------------------------------------------

template<typename _F, typename _Deps, size_t... _Is>
auto _call(_F func, _Deps& deps, std::index_sequence<_Is...>)
{
  return func(std::get<_Is>(deps)...);
}

template<typename _F, typename _Deps>
auto call(_F func, _Deps& deps)
{
  using args_t = typename Callable<_F>::args_t;
  using args_indices_t = std::make_index_sequence<std::tuple_size<args_t>::value>;
  return _call(func, deps, args_indices_t{});
}

//--------------------------------------------------------------------------

static const auto always_true_guard = [](){ return true; };
static const auto no_action = [](){};

struct anonymous_t {};
struct on_entry_t {};
struct on_exit_t {};
struct initial_t {};
struct final_t {};

//--------------------------------------------------------------------------

template<typename _T>
struct IsStateActionEvent
{
  static constexpr auto value = std::is_same<_T, Event<on_entry_t>>::value or
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
using CollectStates_t = UniqueTypesTuple_t<ConcatTuples_t<
                      Apply_t<RowSrcState_t, _Rows>,
                      Apply_t<RowDstState_t, _Rows>
                    >>;

//--------------------------------------------------------------------------

template<typename _Row>
using GuardArgumentList_t =
            typename Callable<typename _Row::event_bundle_t::guard_t>::args_t;
template<typename _Row>
using ActionArgumentList_t =
            typename Callable<typename _Row::event_bundle_t::action_t>::args_t;
/// Collect required parameter types (dependencies) from actions and guards
/// signatures.
template<typename _Rows>
using CollectRequiredDepTypes_t = UniqueTypesTuple_t<ConcatTuples_t<
      Apply_t<GuardArgumentList_t, _Rows>,
      Apply_t<ActionArgumentList_t, _Rows>
    >>;

//--------------------------------------------------------------------------

template<typename _Rows, typename _Event>
struct RowsWithEventIndices
{
  template<typename _Row>
  struct filter_t : std::is_same<typename std::decay_t<_Row>::event_bundle_t::event_t, _Event> {};
  using indices_t = TupleIndexFilter_t<filter_t, _Rows>;
};

/// @return tuple of references to rows where the event is present
template<typename _Rows, typename _Event>
auto rows_with_event(const _Rows& rows, const _Event&)
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
auto rows_with_dst_state(const _Rows& rows, const _State&)
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
  auto operator()(size_t)
  {
    return "N/A";
  }
};
template<typename _AllStates, typename _S0, typename... _Ss>
struct GetCurrentStateNameImpl<_AllStates, std::tuple<_S0, _Ss...>>
{
  auto operator()(size_t n)
  {
    if (n == TypeIndex<_S0, _AllStates>::value)
    {
      return get_type_name<typename _S0::base_t>();
    } else {
      return GetCurrentStateNameImpl<_AllStates, std::tuple<_Ss...>>{}(n);
    }
  }
};

template<typename _AllStates>
struct GetCurrentStateName
{
  auto operator()(size_t n)
  {
    return GetCurrentStateNameImpl<_AllStates, _AllStates>{}(n);
  }
};

//==========================================================================

template<typename _Rows, typename _Deps>
static void call_row_action(const _Rows& rows, _Deps& deps, std::true_type)
{
  call(std::get<0>(rows).m_event_bundle.m_action, deps);
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
template<typename _AllStates, typename _AllRows, typename _Deps, typename _StateNum>
struct ProcessSingleEventImpl<_AllStates, _AllRows, _Deps, _StateNum, std::tuple<>>
{
  bool operator()(const _AllRows&, const std::tuple<>&, _StateNum&, _Deps&) const
  {
    return false;
  }
};
template<typename _AllStates, typename _AllRows, typename _Deps, typename _StateNum,
          typename _Row, typename... _Rows>
struct ProcessSingleEventImpl<_AllStates, _AllRows, _Deps, _StateNum,
                              std::tuple<_Row, _Rows...>>
{
  bool operator()(const _AllRows& all_rows, const std::tuple<_Row, _Rows...>& rows,
                  _StateNum& state, _Deps& deps) const
  {
    using row_t = std::remove_const_t<std::remove_reference_t<_Row>>;
    const auto& row = std::get<_Row>(rows);
    const auto& guard = row.m_event_bundle.m_guard;
    bool processed{false};
    constexpr auto source_state =
                        state_number_v<typename row_t::src_state_t, _AllStates>;
    if ((source_state == state) and call(guard, deps))
    {
      constexpr auto destination_state =
                        state_number_v<typename row_t::dst_state_t, _AllStates>;
      if (source_state != destination_state)
      {
        const auto exit_rows = detail::rows_with_dst_state(
                              detail::rows_with_event(all_rows, Event<on_exit_t>{}),
                              typename row_t::src_state_t{});
        call_row_action(exit_rows, deps,
                          std::conditional_t<IsEmptyTuple<decltype(exit_rows)>::value,
                          std::false_type, std::true_type>{});
      }
      call(row.m_event_bundle.m_action, deps);
      if (source_state != destination_state)
      {
        state = destination_state;
        const auto entry_rows = detail::rows_with_dst_state(
                              detail::rows_with_event(all_rows, Event<on_entry_t>{}),
                              typename row_t::dst_state_t{});
        call_row_action(entry_rows, deps,
                          std::conditional_t<IsEmptyTuple<decltype(entry_rows)>::value,
                          std::false_type, std::true_type>{});
      }
      processed = true;
    }
    return processed or
          ProcessSingleEventImpl<_AllStates, _AllRows, _Deps, _StateNum,
                                std::tuple<_Rows...>>{}(
                        all_rows, std::tuple<_Rows...>{std::get<_Rows>(rows)...},
                        state, deps);
  }
};

template<typename _AllStates, typename _AllRows, typename _FilteredRows,
        typename _StateNum, typename _Deps>
bool process_single_event(const _AllStates&, const _AllRows& all_rows,
                          const _FilteredRows& filtered_rows,
                          _StateNum& state, _Deps& deps)
{
  return ProcessSingleEventImpl<_AllStates, _AllRows, _Deps, _StateNum, _FilteredRows>{}(
                                  all_rows, filtered_rows, state, deps);
}

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
auto transition_table_from_machine_declaration()
{
  static_assert(HasTableOperator<_MachineDecl>::value, "must be a SM declaration");
  return _MachineDecl{}();
}

//--------------------------------------------------------------------------

template<typename _State, typename _Wrap>
struct WrapState
{
  static_assert(is_state_v<_State>, "");
  using type = State<std::pair<_Wrap, typename _State::base_t>>;
};

template<typename...>
struct WrapStateInLayers;
template<typename _State, typename _W0>
struct WrapStateInLayers<_State, _W0>
{
  using type = typename WrapState<_State, _W0>::type;
};
template<typename _State, typename _W0, typename... _Ws>
struct WrapStateInLayers<_State, _W0, _Ws...>
{
  using type = typename WrapState<typename WrapStateInLayers<_State, _Ws...>::type, _W0>::type;
};

//--------------------------------------------------------------------------

template<typename _Tag, typename _Row>
auto wrap_row(const _Row& row)
{
  return TableRow<
              typename WrapState<typename _Row::src_state_t, _Tag>::type,
              typename _Row::event_bundle_t,
              typename WrapState<typename _Row::dst_state_t, _Tag>::type
            >(row.m_event_bundle);
}

template<typename _Tag, typename _Rows, size_t... _Is>
auto wrap_states_impl(const _Rows& rows, TypeHolder<_Tag>, std::index_sequence<_Is...>)
{
  return std::make_tuple(wrap_row<_Tag>(std::get<_Is>(rows))...);
}

/// @return tuple of copied rows that contain wrapped states
template<typename _Tag, typename _Rows>
auto wrap_states(const _Rows& rows)
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
                typename WrapState<State<_PointType>, typename _State::base_t>::type,
                _State
              >;
};

template<typename _Row>
auto wrap_entry_exit_row(const _Row& row)
{
  return TableRow<
              // map substate as a source state to its final state
              typename WrapSubPoint<typename _Row::src_state_t, detail::final_t>::type,
              typename _Row::event_bundle_t,
              // map substate as a destination state to its initial state
              typename WrapSubPoint<typename _Row::dst_state_t, detail::initial_t>::type
            >(row.m_event_bundle);
}

template<typename _Rows, size_t... _Is>
auto wrap_entry_exit_states_impl(const _Rows& rows, std::index_sequence<_Is...>)
{
  return std::make_tuple(wrap_entry_exit_row(std::get<_Is>(rows))...);
}

/// @return tuple of copied rows where source states representing a substate
/// will be converted to final states of that submachine. Destination states
/// will become initial states of that submachine.
template<typename _Rows>
auto wrap_entry_exit_states(const _Rows& rows)
{
  using indices_t = std::make_index_sequence<std::tuple_size<_Rows>::value>;
  return wrap_entry_exit_states_impl(rows, indices_t{});
}

//==========================================================================

template<typename _State>
using SubmachineType_t = typename _State::base_t;

/// Collect machine types from a transition table rows.
template<typename _States>
using CollectSubmachineTypes_t = UniqueTypesTuple_t<Filter_t<HasTableOperator,
                                          Apply_t<SubmachineType_t, _States>>>;

//--------------------------------------------------------------------------

template<typename _Rows>
auto make_transition_table_from_tuple(_Rows rows)
{
  return TransitionTable<_Rows>{std::move(rows)};
}

template<typename _MachineDecl>
auto expand_table();

/// Expand and concatenate sub-machines.
template<typename...>
struct AddSubmachines;
template<typename... _STs>
struct AddSubmachines<std::tuple<_STs...>>
{
  auto operator()()
  {
      return std::tuple_cat(wrap_states<_STs>(expand_table<_STs>().m_rows)...);
  }
};

/// Generate recursively a big table for a composite machine with expanded
/// sub-machines and well connected entry and exit points.
template<typename _MachineDecl>
auto expand_table()
{
  const auto table_base = transition_table_from_machine_declaration<_MachineDecl>();
  return make_transition_table_from_tuple(std::tuple_cat(
            wrap_entry_exit_states(std::move(table_base.m_rows)),
            AddSubmachines<typename decltype(table_base)::submachine_types_t>{}()
          ));
}

//==========================================================================
} // namespace
//==========================================================================

/// Groups together event, guard and action as a unification for the table row.
template<typename _Event, typename _GuardF, typename _ActionF>
struct EventBundle
{
  using event_t = std::remove_cv_t<_Event>;
  static_assert(is_event_v<event_t>, "must be event type");
  using guard_t = _GuardF;
  using action_t = _ActionF;

  EventBundle(_GuardF gf, _ActionF af) noexcept : m_guard{gf}, m_action{af} {}

  template<typename _F>
  auto operator/(_F action) const noexcept
  {
    return EventBundle<_Event, _GuardF, _F>{m_guard, action};
  }

  const _GuardF m_guard{};
  const _ActionF m_action{};
};

template<typename _T>
struct Event
{
  template<typename _ActionF>
  auto operator/(_ActionF action) const noexcept
  {
    return EventBundle<Event<_T>, decltype(detail::always_true_guard), _ActionF>{
                detail::always_true_guard, action
              };
  }

  template<typename _GuardF>
  auto operator[](_GuardF guard) const noexcept
  {
    return EventBundle<Event<_T>, _GuardF, decltype(detail::no_action)>{
                guard, detail::no_action
              };
  }
};

//==========================================================================

template<typename _SrcS, typename _EventBundle>
struct StateTransition;

template<typename _Base>
struct State
{
  using base_t = _Base;

  template<typename _E>
  auto operator+(const Event<_E>&) const noexcept
  {
    using eb_t = EventBundle<Event<_E>,
                                decltype(detail::always_true_guard),
                                decltype(detail::no_action)>;
    return StateTransition<State<base_t>, eb_t>{
                  eb_t{detail::always_true_guard, detail::no_action}
                };
  }

  template<typename _E, typename _GuardF, typename _ActionF>
  auto operator+(const EventBundle<_E, _GuardF, _ActionF>& eb) const noexcept
  {
    using tag_t = std::conditional_t<
                detail::IsStateActionEvent<
                                  typename std::decay_t<decltype(eb)>::event_t
                                >::value,
                detail::TypeHolder<state_action_t>,
                detail::TypeHolder<transition_action_t>>;
    return combine_event_bundle<_E, _GuardF, _ActionF>(eb, tag_t{});
  }

  template<typename _F>
  auto operator[](_F guard) const noexcept
  {
    return *this + Event<detail::anonymous_t>{} [ guard ];
  }

  template<typename _F>
  auto operator/(_F action) const noexcept
  {
    return *this + Event<detail::anonymous_t>{} / action;
  }

  template<typename _DstS>
  auto operator=(const State<_DstS>& dst) const noexcept
  {
    return *this + Event<detail::anonymous_t>{} = dst;
  }

private:
  struct transition_action_t {};
  struct state_action_t {};

  template<typename _E, typename _GuardF, typename _ActionF>
  auto combine_event_bundle(const EventBundle<_E, _GuardF, _ActionF>& eb,
                            detail::TypeHolder<transition_action_t>) const noexcept
  {
    return StateTransition<State<base_t>, EventBundle<_E, _GuardF, _ActionF>>{eb};
  }

  template<typename _E, typename _GuardF, typename _ActionF>
  auto combine_event_bundle(const EventBundle<_E, _GuardF, _ActionF>& eb,
                            detail::TypeHolder<state_action_t>) const noexcept
  {
    // make a loop to itself to have unified table rows
    return StateTransition<State<base_t>, EventBundle<_E, _GuardF, _ActionF>>{eb} = State{};
  }
};

//==========================================================================

template<typename _SrcS, typename _EventBundle, typename _DstS>
struct TableRow
{
  static_assert(is_state_v<_SrcS>, "");
  static_assert(is_state_v<_DstS>, "");

  using src_state_t = _SrcS;
  using dst_state_t = _DstS;
  using event_bundle_t = _EventBundle;

  TableRow(_EventBundle event_bundle) : m_event_bundle{event_bundle} {}

  const _EventBundle m_event_bundle{};
};

template<typename _SrcS, typename _EventBundle>
struct StateTransition
{
  static_assert(is_state_v<_SrcS>, "");

  StateTransition(_EventBundle event_bundle) : m_event_bundle{event_bundle} {}

  template<typename _DstS>
  auto operator=(const State<_DstS>&) const noexcept
  {
    return TableRow<_SrcS, _EventBundle, State<_DstS>>{m_event_bundle};
  }

  template<typename _F>
  auto operator/(_F action) const noexcept
  {
    auto new_bundle = EventBundle<typename _EventBundle::event_t,
                                  typename _EventBundle::guard_t,
                                  _F>
                                  {m_event_bundle.m_guard, action};
    return StateTransition<_SrcS, decltype(new_bundle)>{std::move(new_bundle)};
  }

  const _EventBundle m_event_bundle{};
};

template<typename _Rows>
struct TransitionTable
{
  using states_t = detail::CollectStates_t<_Rows>;
  using rows_t = _Rows;
  /// just plain types, no references
  using deps_t = detail::CollectRequiredDepTypes_t<rows_t>;
  using submachine_types_t = detail::CollectSubmachineTypes_t<states_t>;

  static_assert(std::tuple_size<states_t>::value > 0,
                "table must have at least 1 state");
  static_assert(detail::HasType<State<detail::initial_t>, states_t>::value,
                "table must have initial state");

  TransitionTable(const _Rows& rows) noexcept : m_rows{rows} {}

  const rows_t m_rows;
};

template<typename... _Ts>
auto make_transition_table(_Ts... rows)
{
  return TransitionTable<std::tuple<_Ts...>>{std::tuple<_Ts...>{rows...}};
}

//==========================================================================

template<typename _MachineDecl, typename... _Deps>
class Sm
{
public:
  /// Dependencies are always passed as a reference. If the dependency is an
  /// rvalue then it will be moved to an internal value.
  explicit Sm(_Deps&... deps)
    : m_deps{deps...}
  {
    process_anonymous_events();
  }

  template<typename _State>
  bool is(const _State&) const noexcept
  {
    constexpr auto number = detail::TypeIndex<
                                      std::remove_cv_t<_State>,
                                      typename transition_table_t::states_t
                                    >::value;
    return m_state_number == number;
  }

  /// XXX this will be removed and somehow unified with is()
  template<typename _State, typename... _Submachines>
  bool is_sub(const _State&, const _Submachines&...) const noexcept
  {
    constexpr auto number = detail::TypeIndex<
                                      typename detail::WrapStateInLayers<
                                                      std::remove_cv_t<_State>,
                                                      _Submachines...
                                                    >::type,
                                      typename transition_table_t::states_t
                                    >::value;
    return m_state_number == number;
  }

  const char* get_current_state_name()
  {
    return detail::GetCurrentStateName<typename transition_table_t::states_t>{}(
                                                                m_state_number);
  }

  template<typename _ET>
  void process_event(const Event<_ET>& event)
  {
    process_single_event(event);
    process_anonymous_events();
  }

  //--------------------------------

private:
  using transition_table_t = decltype(detail::expand_table<_MachineDecl>());
  using required_types_t = detail::CollectRequiredDepTypes_t<
                                          typename transition_table_t::rows_t>;

  static constexpr auto states_count =
                  std::tuple_size<typename transition_table_t::states_t>::value;
  using state_number_t = typename detail::MinimalUnsigned<states_count - 1>::type;

  //--------------------------------

  /// @return true if transition was executed
  template<typename _ET>
  bool process_single_event(const Event<_ET>& evt)
  {
    const auto table = detail::expand_table<_MachineDecl>();
    const auto rows = detail::rows_with_event(table.m_rows, evt);
    // only for re-casting
    const auto processed = detail::process_single_event(
                              typename transition_table_t::states_t{}, table.m_rows, rows,
                              m_state_number, m_deps);
    return processed;
  }

  void process_anonymous_events()
  {
    while (process_single_event(Event<detail::anonymous_t>{}))
    { }
  }

  //--------------------------------

  std::tuple<_Deps&...> m_deps;
  /// actual state machine state
  state_number_t m_state_number{detail::TypeIndex<
                                    State<detail::initial_t>,
                                    typename transition_table_t::states_t
                                  >::value};
};

//==========================================================================

static constexpr auto initial_state = State<detail::initial_t>{};
static constexpr auto final_state = State<detail::final_t>{};
static constexpr auto anonymous_event = Event<detail::anonymous_t>{};
static constexpr auto on_entry = Event<detail::on_entry_t>{};
static constexpr auto on_exit = Event<detail::on_exit_t>{};

//==========================================================================

namespace literals {

template<typename T, T... Chrs>
auto operator""_s() {
  return State<detail::CString<T, Chrs...>>{};
}

template<typename T, T... Chrs>
auto operator""_e() {
  return Event<detail::CString<T, Chrs...>>{};
}

}

//==========================================================================
} // namespace
//==========================================================================

#endif /* include guard */
