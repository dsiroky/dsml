#include <iostream>
#include <tuple>
#include <type_traits>

#include "gtest/gtest.h"

namespace detail {

template <class T, T...>
struct string;
template <char... Chrs>
struct string<char, Chrs...> {
  using type = string;
  static auto c_str() noexcept {
    static char str[] = {Chrs..., 0};
    return str;
  }
};

const auto always_true_guard = [](){ return true; };
const auto no_action = [](){};

struct anonymous {};

// missing in gcc 5 STL
template<class...> struct disjunction : std::false_type { };
template<class B1> struct disjunction<B1> : B1 { };
template<class B1, class... Bn>
struct disjunction<B1, Bn...>
    : std::conditional_t<bool(B1::value), B1, disjunction<Bn...>>  { };

template <typename T, typename Tuple>
struct has_type;
template <typename T, typename... Us>
struct has_type<T, std::tuple<Us...>> : disjunction<std::is_same<T, Us>...> {};

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

template<typename T>
struct src_state { using type = typename T::src_state_t; };
template<typename T>
struct dst_state { using type = typename T::dst_state_t; };

} // namespace

//==========================================================================

/// Combination of event, guard and action.
template<typename Event, typename GuardF, typename ActionF>
struct event_bundle
{
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
    return event_bundle<T, decltype(detail::always_true_guard), ActionF>{
                detail::always_true_guard, action
              };
  }

  template<typename GuardF>
  auto operator[](GuardF guard) const noexcept
  {
    return event_bundle<T, GuardF, decltype(detail::no_action)>{
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
    return state_transition<S, eb_t>{eb_t{detail::always_true_guard, detail::no_action}};
  }

  template<typename E, typename GuardF, typename ActionF>
  auto operator+(const event_bundle<E, GuardF, ActionF>& eb) const noexcept
  {
    return state_transition<S, event_bundle<E, GuardF, ActionF>>{eb};
  }

  template<typename DstS>
  auto operator=(const state<DstS>& dst) const noexcept
  {
    return *this + event<detail::anonymous>{} = dst;
  }
};

//==========================================================================

template<typename SrcS, typename EventBundle, typename DstS>
struct table_row
{
  using src_state_t = SrcS;
  using dst_state_t = DstS;

  table_row(EventBundle event_bundle) : m_event_bundle{event_bundle} {}

  EventBundle m_event_bundle{};
};

template<typename SrcS, typename EventBundle>
struct state_transition
{
  state_transition(EventBundle event_bundle) : m_event_bundle{event_bundle} {}

  template<typename DstS>
  auto operator=(const state<DstS>&) const noexcept
  {
    return table_row<SrcS, EventBundle, DstS>{m_event_bundle};
  }

  EventBundle m_event_bundle{};
};

template<typename... Rows>
struct transition_table
{
  using states_t = detail::unique_types_tuple_t<std::tuple<
          typename detail::src_state<Rows>::type...,
          typename detail::dst_state<Rows>::type...
        >>;

  transition_table(Rows... rows) noexcept : m_rows{rows...} {}

  std::tuple<Rows...> m_rows;
};

template<typename... T>
auto make_transition_table(T... transitions) noexcept
{
  return transition_table<T...>{transitions...};
}

//==========================================================================

template<typename T, T... Chrs>
auto operator""_s() {
  return state<detail::string<T, Chrs...>>{};
}

template<typename T, T... Chrs>
auto operator""_e() {
  return event<detail::string<T, Chrs...>>{};
}

//==========================================================================

TEST(UniqueTuple, RemovesDuplicitTypes)
{
  EXPECT_TRUE((std::is_same<
        detail::unique_types_tuple_t<std::tuple<>>,
        std::tuple<>
      >::value));

  EXPECT_TRUE((std::is_same<
        detail::unique_types_tuple_t<std::tuple<int>>,
        std::tuple<int>
      >::value));

  EXPECT_TRUE((std::is_same<
        detail::unique_types_tuple_t<std::tuple<int, bool>>,
        std::tuple<int, bool>
      >::value));

  EXPECT_TRUE((std::is_same<
        detail::unique_types_tuple_t<std::tuple<int, bool, double>>,
        std::tuple<int, bool, double>
      >::value));

  EXPECT_TRUE((std::is_same<
        detail::unique_types_tuple_t<std::tuple<int, int>>,
        std::tuple<int>
      >::value));

  EXPECT_TRUE((std::is_same<
        detail::unique_types_tuple_t<std::tuple<int, int, bool>>,
        std::tuple<int, bool>
      >::value));

  EXPECT_TRUE((std::is_same<
        detail::unique_types_tuple_t<std::tuple<int, bool, int>>,
        std::tuple<bool, int>
      >::value));

  EXPECT_TRUE((std::is_same<
        detail::unique_types_tuple_t<std::tuple<bool, int, int>>,
        std::tuple<bool, int>
      >::value));

  EXPECT_TRUE((std::is_same<
        detail::unique_types_tuple_t<std::tuple<
                      bool, int, int, bool, double, int, bool, double, double
                    >>,
        std::tuple<int, bool, double>
      >::value));
}

//==========================================================================

int main(int argc, char *argv[])
{
  auto action1 = []() { std::cout << "action1" << '\n'; };
  auto guard = []() { return true; };

  const auto table = make_transition_table(
        "start"_s + "e1"_e / action1 = "A"_s
        , "A"_s + "e1"_e [ guard ] = "B"_s
        , "A"_s + "e2"_e [ guard ] = "C"_s
        , "B"_s + "e1"_e [ guard ] / action1 = "C"_s
        , "C"_s = "D"_s
      );

  std::cout << "states count:" << count_states(table) << '\n';

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
