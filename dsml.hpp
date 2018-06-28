// TODO on_entry
// TODO logging
// TODO move private stuff to detail
// TODO optimize
// TODO comments
// TODO automatic dependencies like boost::sml

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
constexpr auto is_state_v = IsState<_T>::value;

template<typename _T>
struct IsEvent : std::false_type {};
template<typename _T>
struct IsEvent<Event<_T>> : std::true_type {};
template<typename _T>
constexpr auto is_event_v = IsEvent<_T>::value;

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

// missing in gcc 5 STL
template<typename...> struct disjunction : std::false_type { };
template<typename _B1> struct disjunction<_B1> : _B1 { };
template<typename _B1, typename... _Bn>
struct disjunction<_B1, _Bn...>
    : std::conditional_t<bool(_B1::value), _B1, disjunction<_Bn...>>  { };

//--------------------------------------------------------------------------

/// provides the minimal unsigned type that can fit the number
template<size_t _X>
struct MinimalUnsigned
{
  using type =
    std::conditional_t<_X <= std::numeric_limits<uint8_t>::max(),
                  uint8_t,
    std::conditional_t<_X <= std::numeric_limits<uint16_t>::max(),
                  uint16_t,
                  uint32_t>>;
};

//--------------------------------------------------------------------------

template<size_t... _Is1, size_t... _Is2>
auto concat_index_seq(std::index_sequence<_Is1...>, std::index_sequence<_Is2...>)
{
  return std::index_sequence<_Is1..., _Is2...>{};
}

//--------------------------------------------------------------------------

template<typename _T>
struct IsTuple : std::false_type {};
template<typename... _T>
struct IsTuple<std::tuple<_T...>> : std::true_type {};

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
  using type = typename std::conditional_t<HasType<_T0, std::tuple<_T...>>::value,
                  UniqueTypesTuple_t<std::tuple<_T...>>,
                  PrependType_t<_T0, UniqueTypesTuple_t<std::tuple<_T...>>>
                >;
};

/// @return tuple of references to a subset of the original tuple.
template<typename _Tuple, size_t... _Is>
auto tuple_ref_selection(const _Tuple& tuple, std::index_sequence<_Is...>)
{
  return std::make_tuple(std::ref(std::get<_Is>(tuple))...);
}

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

template<typename... _Args>
using RawArgs_t = std::tuple<_Args...>;

template<typename _F>
struct CallableArgsTupleImpl;
template<typename _Ret, typename... _Args>
struct CallableArgsTupleImpl<_Ret(_Args...)>
{ using type = RawArgs_t<_Args...>; };
template<typename _Ret, typename _T, typename... _Args>
struct CallableArgsTupleImpl<_Ret(_T::*)(_Args...) const>
{ using type = RawArgs_t<_Args...>; };
template<typename _T>
struct CallableArgsTupleImpl : CallableArgsTupleImpl<decltype(&_T::operator())>
{ };

/// Get callable arguments type list without CV qualifiers and references.
template<typename _F>
using CallableArgsTuple_t = typename CallableArgsTupleImpl<_F>::type;

//--------------------------------------------------------------------------

template<typename _F, typename _Deps, size_t... _Is>
auto _call(_F func, _Deps& deps, std::index_sequence<_Is...>)
{
  return func(std::get<_Is>(deps)...);
}

template<typename _F, typename _Deps>
auto call(_F func, _Deps& deps)
{
  using args_t = CallableArgsTuple_t<_F>;
  using args_indices_t = std::make_index_sequence<std::tuple_size<args_t>::value>;
  return _call(func, deps, args_indices_t{});
}

//--------------------------------------------------------------------------

const auto always_true_guard = [](){ return true; };
const auto no_action = [](){};

struct anonymous_t {};
struct initial_t {};
struct final_t {};

//--------------------------------------------------------------------------

/// Get state number by type from states tuple.
template<typename _S, typename _States>
constexpr size_t state_number_v = detail::TypeIndex<_S, _States>::value;

//--------------------------------------------------------------------------

/// Gather src types from rows.
template<typename...>
struct SrcStates;
template<typename... _Rows>
struct SrcStates<std::tuple<_Rows...>>
{
  using type = std::tuple<typename _Rows::src_state_t...>;
};

/// Gather dst types from rows.
template<typename...>
struct DstStates;
template<typename... _Rows>
struct DstStates<std::tuple<_Rows...>>
{
  using type = std::tuple<typename _Rows::dst_state_t...>;
};

//--------------------------------------------------------------------------

template<typename... _Rows>
struct GatherRequiredDepTypesImpl;
template<>
struct GatherRequiredDepTypesImpl<std::tuple<>>
{
  using type = std::tuple<>;
};
template<typename _Row0, typename... _Rows>
struct GatherRequiredDepTypesImpl<std::tuple<_Row0, _Rows...>>
{
//private:
  using action_types_t = CallableArgsTuple_t<typename _Row0::event_bundle_t::action_t>;
  using guard_types_t = CallableArgsTuple_t<typename _Row0::event_bundle_t::guard_t>;
public:
  using type = ConcatTuples_t<action_types_t, guard_types_t,
                            typename GatherRequiredDepTypesImpl<std::tuple<_Rows...>>::type>;
};

/// Gather required parameter types (dependencies) from actions and guards
/// signatures.
template<typename _Rows>
using GatherRequiredDepTypes_t = UniqueTypesTuple_t<
                              typename GatherRequiredDepTypesImpl<_Rows>::type>;

//--------------------------------------------------------------------------

template<typename _Event, typename _Rows, size_t... _Is>
struct RowsWithEventIndicesImpl;
template<typename _Event, typename _Rows>
struct RowsWithEventIndicesImpl<_Event, _Rows>
{
  using indices_t = std::index_sequence<>;
};
template<typename _Event, typename _Rows, size_t _I>
struct RowsWithEventIndicesImpl<_Event, _Rows, _I>
{
  using indices_t = std::conditional_t<
        std::is_same<
              typename std::tuple_element_t<_I, _Rows>::event_bundle_t::event_t,
              _Event
            >::value,
        std::index_sequence<_I>,
        std::index_sequence<>
      >;
};
template<typename _Event, typename _Rows, size_t _I, size_t... _Is>
struct RowsWithEventIndicesImpl<_Event, _Rows, _I, _Is...>
{
  using indices_t = std::conditional_t<
        std::is_same<
              typename std::tuple_element_t<_I, _Rows>::event_bundle_t::event_t,
              _Event
            >::value,
        decltype(concat_index_seq(std::index_sequence<_I>{},
                                  typename RowsWithEventIndicesImpl<
                                              _Event, _Rows, _Is...
                                            >::indices_t{})),
        typename RowsWithEventIndicesImpl<_Event, _Rows, _Is...>::indices_t
      >;
};

template<typename _Event, typename _Rows, size_t... _Is>
auto RowsWithEventIndices(_Event, _Rows, std::index_sequence<_Is...>)
{
  return typename RowsWithEventIndicesImpl<_Event, _Rows, _Is...>::indices_t{};
}

/// @return tuple of references to rows where the event is present
template<typename _Rows, typename _Event>
auto rows_with_event(const _Rows& rows, const _Event& evt)
{
  using all_indices_t = std::make_index_sequence<std::tuple_size<_Rows>::value>;
  using indices_t = decltype(RowsWithEventIndices(evt, rows, all_indices_t{}));
  return tuple_ref_selection(rows, indices_t{});
}

//==========================================================================

/// Go through rows and try to match it against current state and filter by
/// guards.
template<typename _AllStates, typename _Deps, typename... _Rows>
struct ProcessSingleEventImpl;
template<typename _AllStates, typename _Deps>
struct ProcessSingleEventImpl<_AllStates, _Deps, std::tuple<>>
{
  bool operator()(const std::tuple<>&, const size_t, size_t&, _Deps&) const
  {
    return false;
  }
};
template<typename _AllStates, typename _Deps, typename _Row, typename... _Rows>
struct ProcessSingleEventImpl<_AllStates, _Deps, std::tuple<_Row, _Rows...>>
{
  bool operator()(const std::tuple<_Row, _Rows...>& rows,
                  const size_t current_state, size_t& new_state,
                  _Deps& deps) const
  {
    using row_t = std::remove_const_t<std::remove_reference_t<_Row>>;
    const auto& row = std::get<_Row>(rows);
    const auto& guard = row.m_event_bundle.m_guard;
    bool processed{false};
    if ((state_number_v<typename row_t::src_state_t, _AllStates> == current_state)
        and
        call(guard, deps))
    {
      const auto& action = row.m_event_bundle.m_action;
      call(action, deps);
      new_state = state_number_v<typename row_t::dst_state_t, _AllStates>;
      processed = true;
    }
    return processed or
          ProcessSingleEventImpl<_AllStates, _Deps, std::tuple<_Rows...>>{}(
                            std::tuple<_Rows...>{std::get<_Rows>(rows)...},
                            current_state, new_state, deps);
  }
};

template<typename _AllStates, typename _FilteredRows, typename _Deps>
bool process_single_event(const _AllStates&, const _FilteredRows& filtered_rows,
                          const size_t current_state, size_t& new_state,
                          _Deps& deps)
{
  return ProcessSingleEventImpl<_AllStates, _Deps, _FilteredRows>{}(
                                  filtered_rows, current_state, new_state, deps);
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

// tag holder is only for autodeduction (avoid instantiation of the tag itself)
template<typename _Tag>
struct TagHolder {};

template<typename _Tag, typename _Rows, size_t... _Is>
auto wrap_states_impl(const _Rows& rows, TagHolder<_Tag>, std::index_sequence<_Is...>)
{
  return std::make_tuple(wrap_row<_Tag>(std::get<_Is>(rows))...);
}

/// @return tuple of copied rows that contain wrapped states
template<typename _Tag, typename _Rows>
auto wrap_states(const _Rows& rows)
{
  using indices_t = std::make_index_sequence<std::tuple_size<_Rows>::value>;
  return wrap_states_impl(rows, TagHolder<_Tag>{}, indices_t{});
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

/// Collect machine types from a transition table rows.
template<typename...>
struct CollectSubmachineTypes;
template<>
struct CollectSubmachineTypes<std::tuple<>>
{
  using type = std::tuple<>;
};
template<typename _Row0, typename... _Rows>
struct CollectSubmachineTypes<std::tuple<_Row0, _Rows...>>
{
  using type = UniqueTypesTuple_t<ConcatTuples_t<
      std::conditional_t<HasTableOperator<typename _Row0::src_state_t::base_t>::value,
                        std::tuple<typename _Row0::src_state_t::base_t>,
                        std::tuple<>>,
      std::conditional_t<HasTableOperator<typename _Row0::dst_state_t::base_t>::value,
                        std::tuple<typename _Row0::dst_state_t::base_t>,
                        std::tuple<>>,
      typename CollectSubmachineTypes<std::tuple<_Rows...>>::type
    >>;
};

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

  _GuardF m_guard{};
  _ActionF m_action{};
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

template<typename _S>
struct State
{
  using base_t = _S;

  template<typename _E>
  auto operator+(const Event<_E>&) const noexcept
  {
    using eb_t = EventBundle<Event<_E>,
                                decltype(detail::always_true_guard),
                                decltype(detail::no_action)>;
    return StateTransition<State<_S>, eb_t>{
                  eb_t{detail::always_true_guard, detail::no_action}
                };
  }

  template<typename _E, typename _GuardF, typename _ActionF>
  auto operator+(const EventBundle<_E, _GuardF, _ActionF>& eb) const noexcept
  {
    return StateTransition<State<_S>, EventBundle<_E, _GuardF, _ActionF>>{eb};
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
};

constexpr auto initial_state = State<detail::initial_t>{};
constexpr auto final_state = State<detail::final_t>{};
constexpr auto anonymous_event = Event<detail::anonymous_t>{};

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

  _EventBundle m_event_bundle{};
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

  _EventBundle m_event_bundle{};
};

template<typename _Rows>
struct TransitionTable
{
  using states_t = detail::UniqueTypesTuple_t<detail::ConcatTuples_t<
          typename detail::SrcStates<_Rows>::type,
          typename detail::DstStates<_Rows>::type
        >>;
  using rows_t = _Rows;
  /// just plain types, no references
  using deps_t = detail::GatherRequiredDepTypes_t<rows_t>;
  using submachine_types_t = typename detail::CollectSubmachineTypes<rows_t>::type;

  static_assert(std::tuple_size<states_t>::value > 0,
                "table must have at least 1 state");
  static_assert(detail::HasType<State<detail::initial_t>, states_t>::value,
                "table must have initial state");

  TransitionTable(_Rows rows) noexcept : m_rows{rows} {}

  rows_t m_rows;
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
                                      typename detail::WrapStateInLayers<std::remove_cv_t<_State>, _Submachines...>::type,
                                      typename transition_table_t::states_t
                                    >::value;
    return m_state_number == number;
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
  using required_types_t = detail::GatherRequiredDepTypes_t<
                                          typename transition_table_t::rows_t>;

  static constexpr auto states_count =
                  std::tuple_size<typename transition_table_t::states_t>::value;
  using state_number_t = typename detail::MinimalUnsigned<states_count - 1>::type;

  //--------------------------------

  /// @return true if transition was executed
  template<typename _ET>
  bool process_single_event(const Event<_ET>& evt)
  {
    const auto rows = detail::rows_with_event(m_table.m_rows, evt);
    // only for re-casting
    size_t tmp_state{m_state_number};
    const auto processed = detail::process_single_event(
                              typename transition_table_t::states_t{}, rows,
                              tmp_state, tmp_state, m_deps);
    m_state_number = static_cast<state_number_t>(tmp_state);
    return processed;
  }

  void process_anonymous_events()
  {
    while (process_single_event(Event<detail::anonymous_t>{}))
    { }
  }

  //--------------------------------

  std::tuple<_Deps&...> m_deps;
  const transition_table_t m_table = detail::expand_table<_MachineDecl>();
  /// actual state machine state
  state_number_t m_state_number{detail::TypeIndex<
                                    State<detail::initial_t>,
                                    typename transition_table_t::states_t
                                  >::value};
};



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
