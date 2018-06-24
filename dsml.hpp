// TODO smallest data type size_t -> min type
// TODO composite

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
struct transition_table;

namespace detail {

template <class T, T...>
struct cstring;
template <char... Chrs>
struct cstring<char, Chrs...> {
  using type = cstring;
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
struct minimal_unsigned
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
struct has_type;
template <typename T, typename... Us>
struct has_type<T, std::tuple<Us...>> : disjunction<std::is_same<T, Us>...> {};

//--------------------------------------------------------------------------

template<typename T>
struct remove_first_type { };
template<typename T, typename... Ts>
struct remove_first_type<std::tuple<T, Ts...>>
{
  using type = std::tuple<Ts...>;
};
template<typename... T>
using remove_first_type_t = typename remove_first_type<T...>::type;

template<typename... T>
struct prepend_type;
template<typename T0, typename... T>
struct prepend_type<T0, std::tuple<T...>>
{
  using type = std::tuple<T0, T...>;
};
template<typename... T>
using prepend_type_t = typename prepend_type<T...>::type;

template<typename... T>
struct unique_types_tuple;
template<typename... T>
using unique_types_tuple_t = typename unique_types_tuple<T...>::type;
template<>
struct unique_types_tuple<std::tuple<>>
{
  using type = std::tuple<>;
};
template<typename T>
struct unique_types_tuple<std::tuple<T>>
{
  using type = std::tuple<T>;
};
template<typename T0, typename... T>
struct unique_types_tuple<std::tuple<T0, T...>>
{
  using type = typename std::conditional_t<has_type<T0, std::tuple<T...>>::value,
                  unique_types_tuple_t<std::tuple<T...>>,
                  prepend_type_t<T0, unique_types_tuple_t<std::tuple<T...>>>
                >;
};

template<typename Tuple, size_t... Is>
auto tuple_ref_selection(const Tuple& tuple, std::index_sequence<Is...>)
{
  return std::make_tuple(std::ref(std::get<Is>(tuple))...);
}

//--------------------------------------------------------------------------

template<size_t I, typename... Ts>
struct type_index_impl;
template<size_t I, typename T>
struct type_index_impl<I, T>
{
};
template<size_t I, typename T, typename T0, typename... Ts>
struct type_index_impl<I, T, T0, Ts...>
{
  static constexpr auto value =
    std::conditional_t<std::is_same<T, T0>::value,
                      std::integral_constant<size_t, I>,
                      type_index_impl<I + 1, T, Ts...>>::value;
};

template<typename... Ts>
struct type_index;
template<typename T, typename... Ts>
struct type_index<T, std::tuple<Ts...>>
{
  static_assert(has_type<T, std::tuple<Ts...>>::value,
                "type not present");
  static constexpr auto value = type_index_impl<0, T, Ts...>::value;
};

//--------------------------------------------------------------------------

template<typename F>
struct callable_args_tuple_impl;
template<typename... Args>
struct callable_args_tuple_impl<void(Args...)>
{
  using type = std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...>;
};
template<typename T, typename... Args>
struct callable_args_tuple_impl<void(T::*)(Args...) const>
{
  using type = std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...>;
};
template<typename T>
struct callable_args_tuple_impl
  : callable_args_tuple_impl<decltype(&T::operator())>
{ };

/// Get arguments type list without CV qualifiers and references.
template<typename F>
using callable_args_tuple_t = typename callable_args_tuple_impl<F>::type;

//--------------------------------------------------------------------------

const auto always_true_guard = [](){ return true; };
const auto no_action = [](){};

struct anonymous {};
struct initial {};

//--------------------------------------------------------------------------

/// Get state type by number from states tuple.
template<typename States, size_t I>
using state_by_number_t = typename std::tuple_element<I, States>::type;
/// Get state number by type from states tuple.
template<typename States, typename S>
constexpr size_t state_number_v = detail::type_index<S, States>::value;

//--------------------------------------------------------------------------

template<typename Event, typename Rows, size_t... Is>
struct rows_with_event_indices_impl;
template<typename Event, typename Rows>
struct rows_with_event_indices_impl<Event, Rows>
{
  using indices_t = std::index_sequence<>;
};
template<typename Event, typename Rows, size_t I>
struct rows_with_event_indices_impl<Event, Rows, I>
{
  using indices_t = std::conditional_t<std::is_same<typename std::tuple_element_t<I, Rows>::event_bundle_t::event_t, Event>::value,
        std::index_sequence<I>,
        std::index_sequence<>
      >;
};
template<typename Event, typename Rows, size_t I, size_t... Is>
struct rows_with_event_indices_impl<Event, Rows, I, Is...>
{
  using indices_t = std::conditional_t<std::is_same<typename std::tuple_element_t<I, Rows>::event_bundle_t::event_t, Event>::value,
        decltype(concat_index_seq(std::index_sequence<I>{}, typename rows_with_event_indices_impl<Event, Rows, Is...>::indices_t{})),
        typename rows_with_event_indices_impl<Event, Rows, Is...>::indices_t
      >;
};

template<typename Event, typename Rows, size_t... Is>
auto rows_with_event_indices(Event, Rows, std::index_sequence<Is...>)
{
  return typename rows_with_event_indices_impl<Event, Rows, Is...>::indices_t{};
}

/// @return tuple of references to rows where the event is present
template<typename Rows, typename Event>
auto rows_with_event(const Rows& rows, const Event& evt)
{
  using all_indices_t = std::make_index_sequence<std::tuple_size<Rows>::value>;
  using indices_t = decltype(rows_with_event_indices(evt, rows, all_indices_t{}));
  return tuple_ref_selection(rows, indices_t{});
}

//==========================================================================

template<typename AllStates, typename... Rows>
struct process_single_event_impl;
template<typename AllStates>
struct process_single_event_impl<AllStates, std::tuple<>>
{
  bool operator()(const size_t, size_t&) const
  {
    return false;
  }
};
template<typename AllStates, typename Row, typename... Rows>
struct process_single_event_impl<AllStates, std::tuple<Row, Rows...>>
{
  bool operator()(const size_t current_state, size_t& new_state) const
  {
    using row_t = std::remove_cv_t<std::remove_reference_t<Row>>;
    bool processed = false;
    if (type_index<typename row_t::src_state_t, AllStates>::value == current_state)
    {
      new_state = type_index<typename row_t::dst_state_t, AllStates>::value;
      processed = true;
    }
    return processed or process_single_event_impl<AllStates, std::tuple<Rows...>>{}(current_state, new_state);
  }
};

template<typename AllStates, typename FilteredRows>
bool process_single_event(const AllStates&, const FilteredRows&,
                          const size_t current_state, size_t& new_state)
{
  return process_single_event_impl<AllStates, FilteredRows>{}(current_state, new_state);
}

//==========================================================================
} // namespace
//==========================================================================

template<typename T> struct state;
template<typename T> struct event;

template<typename T>
struct is_state : std::false_type {};
template<typename T>
struct is_state<state<T>> : std::true_type {};
template<typename T>
constexpr auto is_state_v = is_state<T>::value;

template<typename T>
struct is_event : std::false_type {};
template<typename T>
struct is_event<event<T>> : std::true_type {};
template<typename T>
constexpr auto is_event_v = is_event<T>::value;

//==========================================================================

/// Combination of event, guard and action.
template<typename Event, typename GuardF, typename ActionF>
struct event_bundle
{
  using event_t = std::remove_cv_t<Event>;
  static_assert(is_event_v<event_t>, "must be event type");

  event_bundle(GuardF gf, ActionF af) noexcept : m_guard{gf}, m_action{af} {}

  template<typename F>
  auto operator/(F action) const noexcept
  {
    return event_bundle<Event, GuardF, F>{m_guard, action};
  }

  GuardF m_guard{};
  ActionF m_action{};
};

template<typename T>
struct event
{
  template<typename ActionF>
  auto operator/(ActionF action) const noexcept
  {
    return event_bundle<event<T>, decltype(detail::always_true_guard), ActionF>{
                detail::always_true_guard, action
              };
  }

  template<typename GuardF>
  auto operator[](GuardF guard) const noexcept
  {
    return event_bundle<event<T>, GuardF, decltype(detail::no_action)>{
                guard, detail::no_action
              };
  }
};

//==========================================================================

template<typename SrcS, typename EventBundle>
struct state_transition;

template<typename S>
struct state
{
  template<typename E>
  auto operator+(const event<E>&) const noexcept
  {
    using eb_t = event_bundle<event<E>,
                                decltype(detail::always_true_guard),
                                decltype(detail::no_action)>;
    return state_transition<state<S>, eb_t>{
                  eb_t{detail::always_true_guard, detail::no_action}
                };
  }

  template<typename E, typename GuardF, typename ActionF>
  auto operator+(const event_bundle<E, GuardF, ActionF>& eb) const noexcept
  {
    return state_transition<state<S>, event_bundle<E, GuardF, ActionF>>{eb};
  }

  template<typename DstS>
  auto operator=(const state<DstS>& dst) const noexcept
  {
    return *this + event<detail::anonymous>{} = dst;
  }
};

constexpr auto initial_state = state<detail::initial>{};

//==========================================================================

template<typename SrcS, typename EventBundle, typename DstS>
struct table_row
{
  static_assert(is_state_v<SrcS>, "");
  static_assert(is_state_v<DstS>, "");

  using src_state_t = SrcS;
  using dst_state_t = DstS;
  using event_bundle_t = EventBundle;

  table_row(EventBundle event_bundle) : m_event_bundle{event_bundle} {}

  EventBundle m_event_bundle{};
};

template<typename SrcS, typename EventBundle>
struct state_transition
{
  static_assert(is_state_v<SrcS>, "");

  state_transition(EventBundle event_bundle) : m_event_bundle{event_bundle} {}

  template<typename DstS>
  auto operator=(const state<DstS>&) const noexcept
  {
    return table_row<SrcS, EventBundle, state<DstS>>{m_event_bundle};
  }

  EventBundle m_event_bundle{};
};

template<typename... Rows>
struct transition_table
{
  /// just helpers because Rows::src_state_t... is a bad syntax
  template<typename T>
  struct src_state { using type = typename T::src_state_t; };
  template<typename T>
  struct dst_state { using type = typename T::dst_state_t; };

  using states_t = detail::unique_types_tuple_t<std::tuple<
          typename src_state<Rows>::type...,
          typename dst_state<Rows>::type...
        >>;

  static_assert(std::tuple_size<states_t>::value > 0,
                "table must have at least 1 state");
  static_assert(detail::has_type<state<detail::initial>, states_t>::value,
                "table must have initial state");

  transition_table(Rows... rows) noexcept : m_rows{rows...} {}

  std::tuple<Rows...> m_rows;
};

template<typename... T>
auto make_transition_table(T... transitions) noexcept
{
  return transition_table<T...>{transitions...};
}

//==========================================================================

template<typename T, typename... Args>
class sm
{
public:
  explicit sm(Args&&...)
  {
    process_anonymous_events();
  }

  template<typename State>
  bool is() const noexcept
  {
    constexpr auto number = detail::type_index<
                                      std::remove_cv_t<State>,
                                      typename transition_table_t::states_t
                                    >::value;
    return m_state_number == number;
  }

  template<typename Event>
  void process_event(const event<Event>& event)
  {
    process_single_event(event);
    process_anonymous_events();
  }

  //--------------------------------

private:
  using transition_table_t = decltype(T{}());
  static constexpr auto states_count =
                  std::tuple_size<typename transition_table_t::states_t>::value;
  using state_number_t = typename detail::minimal_unsigned<states_count - 1>::type;

  //--------------------------------

  /// @return true if transition was executed
  template<typename Event>
  bool process_single_event(const event<Event>& evt)
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
    while (process_single_event(event<detail::anonymous>{}))
    { }
  }

  //--------------------------------

  const transition_table_t m_table = T{}();
  /// actual state machine state
  state_number_t m_state_number{detail::type_index<
                                    state<detail::initial>,
                                    typename transition_table_t::states_t
                                  >::value};
};



//==========================================================================

namespace literals {

template<typename T, T... Chrs>
auto operator""_s() {
  return state<detail::cstring<T, Chrs...>>{};
}

template<typename T, T... Chrs>
auto operator""_e() {
  return event<detail::cstring<T, Chrs...>>{};
}

}

//==========================================================================
} // namespace
//==========================================================================

#endif /* include guard */
