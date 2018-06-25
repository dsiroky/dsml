// TODO rename
// TODO action
// TODO guard
// TODO warnings
// TODO composite
// TODO comments

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

template<typename... Rows>
struct TransitionTable;

namespace detail {

template <class T, T...>
struct CString;
template <char... Chrs>
struct CString<char, Chrs...> {
  using type = CString;
  static auto c_str() noexcept {
    static char str[] = {Chrs..., 0};
    return str;
  }
};

//--------------------------------------------------------------------------

// missing in gcc 5 STL
template<class...> struct disjunction : std::false_type { };
template<class B1> struct disjunction<B1> : B1 { };
template<class B1, class... Bn>
struct disjunction<B1, Bn...>
    : std::conditional_t<bool(B1::value), B1, disjunction<Bn...>>  { };

//--------------------------------------------------------------------------

/// provides the minimal unsigned type that can fit the number
template<size_t X>
struct MinimalUnsigned
{
  using type =
    std::conditional_t<X <= std::numeric_limits<uint8_t>::max(),
                  uint8_t,
    std::conditional_t<X <= std::numeric_limits<uint16_t>::max(),
                  uint16_t,
                  uint32_t>>;
};

//--------------------------------------------------------------------------

template<size_t... Is1, size_t... Is2>
auto concat_index_seq(std::index_sequence<Is1...>, std::index_sequence<Is2...>)
{
  return std::index_sequence<Is1..., Is2...>{};
}

//--------------------------------------------------------------------------

template <typename T, typename Tuple>
struct HasType;
template <typename T, typename... Us>
struct HasType<T, std::tuple<Us...>> : disjunction<std::is_same<T, Us>...> {};

//--------------------------------------------------------------------------

template<typename T>
struct RemoveFirstType { };
template<typename T, typename... Ts>
struct RemoveFirstType<std::tuple<T, Ts...>>
{
  using type = std::tuple<Ts...>;
};
template<typename... T>
using RemoveFirstType_t = typename RemoveFirstType<T...>::type;

template<typename... T>
struct PrependType;
template<typename T0, typename... T>
struct PrependType<T0, std::tuple<T...>>
{
  using type = std::tuple<T0, T...>;
};
template<typename... T>
using PrependType_t = typename PrependType<T...>::type;

template<typename... T>
struct UniqueTypesTuple;
template<typename... T>
using UniqueTypesTuple_t = typename UniqueTypesTuple<T...>::type;
template<>
struct UniqueTypesTuple<std::tuple<>>
{
  using type = std::tuple<>;
};
template<typename T>
struct UniqueTypesTuple<std::tuple<T>>
{
  using type = std::tuple<T>;
};
template<typename T0, typename... T>
struct UniqueTypesTuple<std::tuple<T0, T...>>
{
  using type = typename std::conditional_t<HasType<T0, std::tuple<T...>>::value,
                  UniqueTypesTuple_t<std::tuple<T...>>,
                  PrependType_t<T0, UniqueTypesTuple_t<std::tuple<T...>>>
                >;
};

template<typename Tuple, size_t... Is>
auto tuple_ref_selection(const Tuple& tuple, std::index_sequence<Is...>)
{
  return std::make_tuple(std::ref(std::get<Is>(tuple))...);
}

//--------------------------------------------------------------------------

template<size_t I, typename... Ts>
struct TypeIndex_impl;
template<size_t I, typename T>
struct TypeIndex_impl<I, T>
{
};
template<size_t I, typename T, typename T0, typename... Ts>
struct TypeIndex_impl<I, T, T0, Ts...>
{
  static constexpr auto value =
    std::conditional_t<std::is_same<T, T0>::value,
                      std::integral_constant<size_t, I>,
                      TypeIndex_impl<I + 1, T, Ts...>>::value;
};

template<typename... Ts>
struct TypeIndex;
template<typename T, typename... Ts>
struct TypeIndex<T, std::tuple<Ts...>>
{
  static_assert(HasType<T, std::tuple<Ts...>>::value,
                "type not present");
  static constexpr auto value = TypeIndex_impl<0, T, Ts...>::value;
};

//--------------------------------------------------------------------------

template<typename F>
struct CallableArgsTupleImpl;
template<typename... Args>
struct CallableArgsTupleImpl<void(Args...)>
{
  using type = std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...>;
};
template<typename T, typename... Args>
struct CallableArgsTupleImpl<void(T::*)(Args...) const>
{
  using type = std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...>;
};
template<typename T>
struct CallableArgsTupleImpl
  : CallableArgsTupleImpl<decltype(&T::operator())>
{ };

/// Get arguments type list without CV qualifiers and references.
template<typename F>
using CallableArgsTuple_t = typename CallableArgsTupleImpl<F>::type;

//--------------------------------------------------------------------------

const auto always_true_guard = [](){ return true; };
const auto no_action = [](){};

struct anonymous {};
struct initial {};

//--------------------------------------------------------------------------

/// Get state number by type from states tuple.
template<typename S, typename States>
constexpr size_t state_number_v = detail::TypeIndex<S, States>::value;

//--------------------------------------------------------------------------

template<typename _Event, typename Rows, size_t... Is>
struct RowsWithEventIndicesImpl;
template<typename _Event, typename Rows>
struct RowsWithEventIndicesImpl<_Event, Rows>
{
  using indices_t = std::index_sequence<>;
};
template<typename _Event, typename Rows, size_t I>
struct RowsWithEventIndicesImpl<_Event, Rows, I>
{
  using indices_t = std::conditional_t<
        std::is_same<
              typename std::tuple_element_t<I, Rows>::event_bundle_t::event_t,
              _Event
            >::value,
        std::index_sequence<I>,
        std::index_sequence<>
      >;
};
template<typename _Event, typename Rows, size_t I, size_t... Is>
struct RowsWithEventIndicesImpl<_Event, Rows, I, Is...>
{
  using indices_t = std::conditional_t<
        std::is_same<
              typename std::tuple_element_t<I, Rows>::event_bundle_t::event_t,
              _Event
            >::value,
        decltype(concat_index_seq(std::index_sequence<I>{},
                                  typename RowsWithEventIndicesImpl<
                                              _Event, Rows, Is...
                                            >::indices_t{})),
        typename RowsWithEventIndicesImpl<_Event, Rows, Is...>::indices_t
      >;
};

template<typename _Event, typename Rows, size_t... Is>
auto RowsWithEventIndices(_Event, Rows, std::index_sequence<Is...>)
{
  return typename RowsWithEventIndicesImpl<_Event, Rows, Is...>::indices_t{};
}

/// @return tuple of references to rows where the event is present
template<typename Rows, typename _Event>
auto rows_with_event(const Rows& rows, const _Event& evt)
{
  using all_indices_t = std::make_index_sequence<std::tuple_size<Rows>::value>;
  using indices_t = decltype(RowsWithEventIndices(evt, rows, all_indices_t{}));
  return tuple_ref_selection(rows, indices_t{});
}

//==========================================================================

template<typename AllStates, typename... Rows>
struct ProcessSingleEventImpl;
template<typename AllStates>
struct ProcessSingleEventImpl<AllStates, std::tuple<>>
{
  bool operator()(const size_t, size_t&) const
  {
    return false;
  }
};
template<typename AllStates, typename Row, typename... Rows>
struct ProcessSingleEventImpl<AllStates, std::tuple<Row, Rows...>>
{
  bool operator()(const size_t current_state, size_t& new_state) const
  {
    using row_t = std::remove_cv_t<std::remove_reference_t<Row>>;
    bool processed = false;
    if (state_number_v<typename row_t::src_state_t, AllStates> == current_state)
    {
      new_state = state_number_v<typename row_t::dst_state_t, AllStates>;
      processed = true;
    }
    return processed or
          ProcessSingleEventImpl<AllStates, std::tuple<Rows...>>{}(
                                                      current_state, new_state);
  }
};

template<typename AllStates, typename FilteredRows>
bool process_single_event(const AllStates&, const FilteredRows&,
                          const size_t current_state, size_t& new_state)
{
  return ProcessSingleEventImpl<AllStates, FilteredRows>{}(current_state,
                                                              new_state);
}

//==========================================================================
} // namespace
//==========================================================================

template<typename T> struct state;
template<typename T> struct Event;

template<typename T>
struct IsState : std::false_type {};
template<typename T>
struct IsState<state<T>> : std::true_type {};
template<typename T>
constexpr auto is_state_v = IsState<T>::value;

template<typename T>
struct IsEvent : std::false_type {};
template<typename T>
struct IsEvent<Event<T>> : std::true_type {};
template<typename T>
constexpr auto is_event_v = IsEvent<T>::value;

//==========================================================================

/// Groups together event, guard and action as a unification for the table row.
template<typename _Event, typename GuardF, typename ActionF>
struct EventBundle
{
  using event_t = std::remove_cv_t<_Event>;
  static_assert(is_event_v<event_t>, "must be event type");

  EventBundle(GuardF gf, ActionF af) noexcept : m_guard{gf}, m_action{af} {}

  template<typename F>
  auto operator/(F action) const noexcept
  {
    return EventBundle<_Event, GuardF, F>{m_guard, action};
  }

  GuardF m_guard{};
  ActionF m_action{};
};

template<typename T>
struct Event
{
  template<typename ActionF>
  auto operator/(ActionF action) const noexcept
  {
    return EventBundle<Event<T>, decltype(detail::always_true_guard), ActionF>{
                detail::always_true_guard, action
              };
  }

  template<typename GuardF>
  auto operator[](GuardF guard) const noexcept
  {
    return EventBundle<Event<T>, GuardF, decltype(detail::no_action)>{
                guard, detail::no_action
              };
  }
};

//==========================================================================

template<typename SrcS, typename EventBundle>
struct StateTransition;

template<typename S>
struct state
{
  template<typename E>
  auto operator+(const Event<E>&) const noexcept
  {
    using eb_t = EventBundle<Event<E>,
                                decltype(detail::always_true_guard),
                                decltype(detail::no_action)>;
    return StateTransition<state<S>, eb_t>{
                  eb_t{detail::always_true_guard, detail::no_action}
                };
  }

  template<typename E, typename GuardF, typename ActionF>
  auto operator+(const EventBundle<E, GuardF, ActionF>& eb) const noexcept
  {
    return StateTransition<state<S>, EventBundle<E, GuardF, ActionF>>{eb};
  }

  template<typename DstS>
  auto operator=(const state<DstS>& dst) const noexcept
  {
    return *this + Event<detail::anonymous>{} = dst;
  }
};

constexpr auto initial_state = state<detail::initial>{};

//==========================================================================

template<typename SrcS, typename EventBundle, typename DstS>
struct TableRow
{
  static_assert(is_state_v<SrcS>, "");
  static_assert(is_state_v<DstS>, "");

  using src_state_t = SrcS;
  using dst_state_t = DstS;
  using event_bundle_t = EventBundle;

  TableRow(EventBundle event_bundle) : m_event_bundle{event_bundle} {}

  EventBundle m_event_bundle{};
};

template<typename SrcS, typename EventBundle>
struct StateTransition
{
  static_assert(is_state_v<SrcS>, "");

  StateTransition(EventBundle event_bundle) : m_event_bundle{event_bundle} {}

  template<typename DstS>
  auto operator=(const state<DstS>&) const noexcept
  {
    return TableRow<SrcS, EventBundle, state<DstS>>{m_event_bundle};
  }

  EventBundle m_event_bundle{};
};

template<typename... Rows>
struct TransitionTable
{
  /// just helpers because Rows::src_state_t... is a bad syntax
  template<typename T>
  struct src_state { using type = typename T::src_state_t; };
  template<typename T>
  struct dst_state { using type = typename T::dst_state_t; };

  using states_t = detail::UniqueTypesTuple_t<std::tuple<
          typename src_state<Rows>::type...,
          typename dst_state<Rows>::type...
        >>;

  static_assert(std::tuple_size<states_t>::value > 0,
                "table must have at least 1 state");
  static_assert(detail::HasType<state<detail::initial>, states_t>::value,
                "table must have initial state");

  TransitionTable(Rows... rows) noexcept : m_rows{rows...} {}

  std::tuple<Rows...> m_rows;
};

template<typename... T>
auto make_transition_table(T... transitions) noexcept
{
  return TransitionTable<T...>{transitions...};
}

//==========================================================================

template<typename T, typename... Args>
class Sm
{
public:
  explicit Sm(Args&&...)
  {
    process_anonymous_events();
  }

  template<typename State>
  bool is() const noexcept
  {
    constexpr auto number = detail::TypeIndex<
                                      std::remove_cv_t<State>,
                                      typename transition_table_t::states_t
                                    >::value;
    return m_state_number == number;
  }

  template<typename ET>
  void process_event(const Event<ET>& event)
  {
    process_single_event(event);
    process_anonymous_events();
  }

  //--------------------------------

private:
  using transition_table_t = decltype(T{}());
  static constexpr auto states_count =
                  std::tuple_size<typename transition_table_t::states_t>::value;
  using state_number_t = typename detail::MinimalUnsigned<states_count - 1>::type;

  //--------------------------------

  /// @return true if transition was executed
  template<typename ET>
  bool process_single_event(const Event<ET>& evt)
  {
    const auto rows = detail::rows_with_event(m_table.m_rows, evt);
    // only for re-casting
    size_t tmp_state{m_state_number};
    const auto processed = detail::process_single_event(
                              typename transition_table_t::states_t{}, rows,
                              tmp_state, tmp_state);
    m_state_number = static_cast<state_number_t>(tmp_state);
    return processed;
  }

  void process_anonymous_events()
  {
    while (process_single_event(Event<detail::anonymous>{}))
    { }
  }

  //--------------------------------

  const transition_table_t m_table = T{}();
  /// actual state machine state
  state_number_t m_state_number{detail::TypeIndex<
                                    state<detail::initial>,
                                    typename transition_table_t::states_t
                                  >::value};
};



//==========================================================================

namespace literals {

template<typename T, T... Chrs>
auto operator""_s() {
  return state<detail::CString<T, Chrs...>>{};
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
