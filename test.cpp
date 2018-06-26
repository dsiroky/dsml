#include "gtest/gtest.h"

#include "dsml.hpp"

//==========================================================================

const auto true_guard = [](){ return true; };
const auto false_guard = [](){ return false; };

//==========================================================================

TEST(MinimalUnsigned, ProvidesType)
{
  EXPECT_TRUE((std::is_same<
        dsml::detail::MinimalUnsigned<0>::type,
        uint8_t
      >::value));
  EXPECT_TRUE((std::is_same<
        dsml::detail::MinimalUnsigned<255>::type,
        uint8_t
      >::value));

  EXPECT_TRUE((std::is_same<
        dsml::detail::MinimalUnsigned<256>::type,
        uint16_t
      >::value));
  EXPECT_TRUE((std::is_same<
        dsml::detail::MinimalUnsigned<65535>::type,
        uint16_t
      >::value));

  EXPECT_TRUE((std::is_same<
        dsml::detail::MinimalUnsigned<65536>::type,
        uint32_t
      >::value));
}

//==========================================================================

TEST(ConcatTuples, Misc)
{
  EXPECT_TRUE((std::is_same<
        dsml::detail::ConcatTuples_t<std::tuple<>>,
        std::tuple<>
      >::value));
  EXPECT_TRUE((std::is_same<
        dsml::detail::ConcatTuples_t<std::tuple<int>>,
        std::tuple<int>
      >::value));
  EXPECT_TRUE((std::is_same<
        dsml::detail::ConcatTuples_t<std::tuple<int, char>>,
        std::tuple<int, char>
      >::value));
  EXPECT_TRUE((std::is_same<
        dsml::detail::ConcatTuples_t<std::tuple<>, std::tuple<int, char>>,
        std::tuple<int, char>
      >::value));
  EXPECT_TRUE((std::is_same<
        dsml::detail::ConcatTuples_t<std::tuple<int, char>, std::tuple<>>,
        std::tuple<int, char>
      >::value));
  EXPECT_TRUE((std::is_same<
        dsml::detail::ConcatTuples_t<std::tuple<int, char>, std::tuple<bool>>,
        std::tuple<int, char, bool>
      >::value));
  EXPECT_TRUE((std::is_same<
        dsml::detail::ConcatTuples_t<std::tuple<int, char>, std::tuple<bool, int>>,
        std::tuple<int, char, bool, int>
      >::value));
  EXPECT_TRUE((std::is_same<
        dsml::detail::ConcatTuples_t<
                            std::tuple<int, char>,
                            std::tuple<bool, int>,
                            std::tuple<>
                          >,
        std::tuple<int, char, bool, int>
      >::value));
  EXPECT_TRUE((std::is_same<
        dsml::detail::ConcatTuples_t<
                            std::tuple<int, char>,
                            std::tuple<const bool, int, char[3]>,
                            std::tuple<double*>
                          >,
        std::tuple<int, char, const bool, int, char[3], double*>
      >::value));
}

//==========================================================================

TEST(UniqueTuple, RemovesDuplicitTypes)
{
  EXPECT_TRUE((std::is_same<
        dsml::detail::UniqueTypesTuple_t<std::tuple<>>,
        std::tuple<>
      >::value));

  EXPECT_TRUE((std::is_same<
        dsml::detail::UniqueTypesTuple_t<std::tuple<int>>,
        std::tuple<int>
      >::value));

  EXPECT_TRUE((std::is_same<
        dsml::detail::UniqueTypesTuple_t<std::tuple<int, bool>>,
        std::tuple<int, bool>
      >::value));

  EXPECT_TRUE((std::is_same<
        dsml::detail::UniqueTypesTuple_t<std::tuple<int, bool, double>>,
        std::tuple<int, bool, double>
      >::value));

  EXPECT_TRUE((std::is_same<
        dsml::detail::UniqueTypesTuple_t<std::tuple<int, int>>,
        std::tuple<int>
      >::value));

  EXPECT_TRUE((std::is_same<
        dsml::detail::UniqueTypesTuple_t<std::tuple<int, int, bool>>,
        std::tuple<int, bool>
      >::value));

  EXPECT_TRUE((std::is_same<
        dsml::detail::UniqueTypesTuple_t<std::tuple<int, bool, int>>,
        std::tuple<bool, int>
      >::value));

  EXPECT_TRUE((std::is_same<
        dsml::detail::UniqueTypesTuple_t<std::tuple<bool, int, int>>,
        std::tuple<bool, int>
      >::value));

  EXPECT_TRUE((std::is_same<
        dsml::detail::UniqueTypesTuple_t<std::tuple<
                      bool, int, int, bool, double, int, bool, double, double
                    >>,
        std::tuple<int, bool, double>
      >::value));
}

//--------------------------------------------------------------------------

TEST(TypeIndex, TypePresent_ValueIsIndex)
{
  {
    const auto idx = dsml::detail::TypeIndex<int,
                                std::tuple<int>>::value;
    EXPECT_EQ(0u, idx);
  }
  {
    const auto idx = dsml::detail::TypeIndex<int,
                                std::tuple<int, bool>>::value;
    EXPECT_EQ(0u, idx);
  }
  {
    const auto idx = dsml::detail::TypeIndex<int,
                                std::tuple<int, bool, double>>::value;
    EXPECT_EQ(0u, idx);
  }
  {
    const auto idx = dsml::detail::TypeIndex<int,
                                std::tuple<bool, int>>::value;
    EXPECT_EQ(1u, idx);
  }
  {
    const auto idx = dsml::detail::TypeIndex<int,
                                std::tuple<bool, int, double>>::value;
    EXPECT_EQ(1u, idx);
  }
  {
    const auto idx = dsml::detail::TypeIndex<int,
                                std::tuple<bool, double, int>>::value;
    EXPECT_EQ(2u, idx);
  }
}

//==========================================================================

static
void dummy_free_function_with_args(int, double*, const char * const, const bool&)
{}

TEST(CallableArgsTuple, FromFreeFunction)
{
  using args_t =
    dsml::detail::CallableArgsTuple_t<decltype(dummy_free_function_with_args)>;

  EXPECT_TRUE((std::is_same<
                  std::tuple_element_t<0, args_t>,
                  int
                >::value));
  EXPECT_TRUE((std::is_same<
                  std::tuple_element_t<1, args_t>,
                  double*
                >::value));
  EXPECT_TRUE((std::is_same<
                  std::tuple_element_t<2, args_t>,
                  const char*
                >::value));
  EXPECT_TRUE((std::is_same<
                  std::tuple_element_t<3, args_t>,
                  const bool&
                >::value));
}

//--------------------------------------------------------------------------

static
int dummy_free_function_with_args_and_return(
                                int, double*, const char * const, const bool&)
{
  return 42;
}

TEST(CallableArgsTuple, FromFreeFunctionWithReturn)
{
  using args_t =
    dsml::detail::CallableArgsTuple_t<
                          decltype(dummy_free_function_with_args_and_return)>;

  EXPECT_TRUE((std::is_same<
                  std::tuple_element_t<0, args_t>,
                  int
                >::value));
  EXPECT_TRUE((std::is_same<
                  std::tuple_element_t<1, args_t>,
                  double*
                >::value));
  EXPECT_TRUE((std::is_same<
                  std::tuple_element_t<2, args_t>,
                  const char*
                >::value));
  EXPECT_TRUE((std::is_same<
                  std::tuple_element_t<3, args_t>,
                  const bool&
                >::value));
}

//--------------------------------------------------------------------------

static
int dummy_free_function_with_args_and_return_noexcept(
                                int, double*, const char * const, const bool&) noexcept
{
  return 42;
}

TEST(CallableArgsTuple, FromFreeFunctionWithReturnNoExcept)
{
  using args_t =
    dsml::detail::CallableArgsTuple_t<
                          decltype(dummy_free_function_with_args_and_return_noexcept)>;

  EXPECT_TRUE((std::is_same<
                  std::tuple_element_t<0, args_t>,
                  int
                >::value));
  EXPECT_TRUE((std::is_same<
                  std::tuple_element_t<1, args_t>,
                  double*
                >::value));
  EXPECT_TRUE((std::is_same<
                  std::tuple_element_t<2, args_t>,
                  const char*
                >::value));
  EXPECT_TRUE((std::is_same<
                  std::tuple_element_t<3, args_t>,
                  const bool&
                >::value));
}

//--------------------------------------------------------------------------

TEST(CallableArgsTuple, FromLambda)
{
  struct S {};
  auto f = [](int, double*, const char * const, const bool&, S&){};

  using args_t = dsml::detail::CallableArgsTuple_t<decltype(f)>;

  EXPECT_TRUE((std::is_same<
                  std::tuple_element_t<0, args_t>,
                  int
                >::value));
  EXPECT_TRUE((std::is_same<
                  std::tuple_element_t<1, args_t>,
                  double*
                >::value));
  EXPECT_TRUE((std::is_same<
                  std::tuple_element_t<2, args_t>,
                  const char*
                >::value));
  EXPECT_TRUE((std::is_same<
                  std::tuple_element_t<3, args_t>,
                  const bool&
                >::value));
  EXPECT_TRUE((std::is_same<
                  std::tuple_element_t<4, args_t>,
                  S&
                >::value));
}

//--------------------------------------------------------------------------

TEST(CallableArgsTuple, FromLambdaWithReturn)
{
  struct S {};
  auto f = [](int, double*, const char * const, const bool&, S&){ return 42; };

  using args_t = dsml::detail::CallableArgsTuple_t<decltype(f)>;

  EXPECT_TRUE((std::is_same<
                  std::tuple_element_t<0, args_t>,
                  int
                >::value));
  EXPECT_TRUE((std::is_same<
                  std::tuple_element_t<1, args_t>,
                  double*
                >::value));
  EXPECT_TRUE((std::is_same<
                  std::tuple_element_t<2, args_t>,
                  const char*
                >::value));
  EXPECT_TRUE((std::is_same<
                  std::tuple_element_t<3, args_t>,
                  const bool&
                >::value));
  EXPECT_TRUE((std::is_same<
                  std::tuple_element_t<4, args_t>,
                  S&
                >::value));
}

//==========================================================================

TEST(TypeTraits, IsState)
{
  EXPECT_FALSE(dsml::is_state_v<int>);
  EXPECT_TRUE(dsml::is_state_v<dsml::State<int>>);
  EXPECT_TRUE(dsml::is_state_v<dsml::State<struct S>>);
}

TEST(TypeTraits, IsEvent)
{
  EXPECT_FALSE(dsml::is_event_v<int>);
  EXPECT_TRUE(dsml::is_event_v<dsml::Event<int>>);
  EXPECT_TRUE(dsml::is_event_v<dsml::Event<struct S>>);
}

//==========================================================================

TEST(RowWithEvent, NoMatch_EmptyTuple)
{
  using namespace dsml::literals;
  const auto rows = std::make_tuple(

          dsml::initial_state = "A"_s
          , "A"_s + "e1"_e = "B"_s
          , "B"_s + "e2"_e = "C"_s

    );

  const auto transitions = dsml::detail::rows_with_event(rows, "e4"_e);

  EXPECT_EQ(0u, std::tuple_size<decltype(transitions)>::value);
}

//--------------------------------------------------------------------------

TEST(RowWithEvent, SingleTransition)
{
  using namespace dsml::literals;
  const auto rows = std::make_tuple(

          dsml::initial_state = "A"_s
          , "A"_s + "e1"_e = "B"_s
          , "B"_s + "e2"_e = "C"_s

    );

  const auto transitions = dsml::detail::rows_with_event(rows, "e1"_e);

  EXPECT_EQ(1u, std::tuple_size<decltype(transitions)>::value);
  using item0_t = std::tuple_element_t<0, decltype(transitions)>;
  EXPECT_TRUE((std::is_same<
        std::remove_reference_t<item0_t>::src_state_t,
        decltype("A"_s)
      >::value));
  EXPECT_TRUE((std::is_same<
        std::remove_reference_t<item0_t>::dst_state_t,
        decltype("B"_s)
      >::value));
}

//--------------------------------------------------------------------------

TEST(RowWithEvent, MultipleTransitions)
{
  using namespace dsml::literals;
  const auto rows = std::make_tuple(

          dsml::initial_state = "A"_s
          , "A"_s + "e1"_e = "B"_s
          , "B"_s + "e2"_e = "C"_s
          , "B"_s + "e1"_e = "D"_s
          , "B"_s + "e3"_e = "A"_s

    );

  const auto transitions = dsml::detail::rows_with_event(rows, "e1"_e);

  EXPECT_EQ(2u, std::tuple_size<decltype(transitions)>::value);

  using item0_t = std::tuple_element_t<0, decltype(transitions)>;
  EXPECT_TRUE((std::is_same<
        std::remove_reference_t<item0_t>::src_state_t,
        decltype("A"_s)
      >::value));
  EXPECT_TRUE((std::is_same<
        std::remove_reference_t<item0_t>::dst_state_t,
        decltype("B"_s)
      >::value));

  using item1_t = std::tuple_element_t<1, decltype(transitions)>;
  EXPECT_TRUE((std::is_same<
        std::remove_reference_t<item1_t>::src_state_t,
        decltype("B"_s)
      >::value));
  EXPECT_TRUE((std::is_same<
        std::remove_reference_t<item1_t>::dst_state_t,
        decltype("D"_s)
      >::value));
}

//==========================================================================

TEST(Sm_Is, OnlyInitialStateAndAnonymousTransition_IsInTheSecondState)
{
  using namespace dsml::literals;
  struct MyMachine { auto operator()() { return dsml::make_transition_table(

          dsml::initial_state = "A"_s

  ); } };
  dsml::Sm<MyMachine> sm{};

  EXPECT_FALSE(sm.is<decltype(dsml::initial_state)>());
  EXPECT_TRUE(sm.is<decltype("A"_s)>());
}

//--------------------------------------------------------------------------

TEST(Sm_Is, OnlyInitialStateAndTransition_IsInTheInitialState)
{
  using namespace dsml::literals;
  struct MyMachine { auto operator()() { return dsml::make_transition_table(

          dsml::initial_state + "e1"_e = "A"_s

  ); } };
  dsml::Sm<MyMachine> sm{};

  EXPECT_TRUE(sm.is<decltype(dsml::initial_state)>());
  EXPECT_FALSE(sm.is<decltype("A"_s)>());
}

//--------------------------------------------------------------------------

TEST(Sm, SingleTransition)
{
  using namespace dsml::literals;
  struct MyMachine { auto operator()() { return dsml::make_transition_table(

          dsml::initial_state + "e1"_e = "A"_s

  ); } };
  dsml::Sm<MyMachine> sm{};

  sm.process_event("e1"_e);

  EXPECT_FALSE(sm.is<decltype(dsml::initial_state)>());
  EXPECT_TRUE(sm.is<decltype("A"_s)>());
}

//--------------------------------------------------------------------------

TEST(Sm, SingleTransitionUnknownEvent_NoStateChange)
{
  using namespace dsml::literals;
  struct MyMachine { auto operator()() { return dsml::make_transition_table(

          dsml::initial_state + "e1"_e = "A"_s

  ); } };
  dsml::Sm<MyMachine> sm{};

  sm.process_event("eunk"_e);

  EXPECT_TRUE(sm.is<decltype(dsml::initial_state)>());
}

//--------------------------------------------------------------------------

TEST(Sm, MultipleTransitionsSameEvents)
{
  using namespace dsml::literals;
  struct MyMachine { auto operator()() { return dsml::make_transition_table(

          dsml::initial_state + "e1"_e = "A"_s
          ,"A"_s + "e1"_e = "B"_s
          ,"B"_s + "e1"_e = "C"_s

  ); } };

  {
    dsml::Sm<MyMachine> sm{};

    sm.process_event("e1"_e);

    EXPECT_TRUE(sm.is<decltype("A"_s)>());
  }
  {
    dsml::Sm<MyMachine> sm{};

    sm.process_event("e1"_e);
    sm.process_event("e1"_e);

    EXPECT_TRUE(sm.is<decltype("B"_s)>());
  }
  {
    dsml::Sm<MyMachine> sm{};

    sm.process_event("e1"_e);
    sm.process_event("e1"_e);
    sm.process_event("e1"_e);

    EXPECT_TRUE(sm.is<decltype("C"_s)>());
  }
}

//--------------------------------------------------------------------------

TEST(Sm, MultipleTransitionsDifferentEvents)
{
  using namespace dsml::literals;
  struct MyMachine { auto operator()() { return dsml::make_transition_table(

          dsml::initial_state + "e1"_e = "A"_s
          ,"A"_s + "e2"_e = "B"_s
          ,"B"_s + "e3"_e = "C"_s

  ); } };

  {
    dsml::Sm<MyMachine> sm{};

    sm.process_event("e1"_e);

    EXPECT_TRUE(sm.is<decltype("A"_s)>());
  }
  {
    dsml::Sm<MyMachine> sm{};

    sm.process_event("e1"_e);
    sm.process_event("e2"_e);

    EXPECT_TRUE(sm.is<decltype("B"_s)>());
  }
  {
    dsml::Sm<MyMachine> sm{};

    sm.process_event("e1"_e);
    sm.process_event("e2"_e);
    sm.process_event("e3"_e);

    EXPECT_TRUE(sm.is<decltype("C"_s)>());
  }
}

//--------------------------------------------------------------------------

TEST(Sm, AnonymousEventAfterNormalEvent)
{
  using namespace dsml::literals;
  struct MyMachine { auto operator()() { return dsml::make_transition_table(

          dsml::initial_state + "e1"_e = "A"_s
          ,"A"_s = "B"_s

  ); } };

  dsml::Sm<MyMachine> sm{};

  sm.process_event("e1"_e);

  EXPECT_TRUE(sm.is<decltype("B"_s)>());
}

//--------------------------------------------------------------------------

TEST(Sm, AnonymousEventBeforeNormalEvent)
{
  using namespace dsml::literals;
  struct MyMachine { auto operator()() { return dsml::make_transition_table(

          dsml::initial_state = "A"_s
          ,"A"_s + "e1"_e = "B"_s

  ); } };

  dsml::Sm<MyMachine> sm{};

  sm.process_event("e1"_e);

  EXPECT_TRUE(sm.is<decltype("B"_s)>());
}

//--------------------------------------------------------------------------

TEST(Sm, MultipleAnonymousEvents)
{
  using namespace dsml::literals;
  struct MyMachine { auto operator()() { return dsml::make_transition_table(

          dsml::initial_state = "A"_s
          ,"A"_s = "B"_s
          ,"B"_s = "C"_s
          ,"C"_s = "D"_s

  ); } };

  dsml::Sm<MyMachine> sm{};

  EXPECT_TRUE(sm.is<decltype("D"_s)>());
}

//--------------------------------------------------------------------------

TEST(Sm, MultipleAnonymousEventsAroundNormalEvent)
{
  using namespace dsml::literals;
  struct MyMachine { auto operator()() { return dsml::make_transition_table(

          dsml::initial_state = "A"_s
          ,"A"_s = "B"_s
          ,"B"_s = "C"_s
          ,"C"_s = "D"_s
          ,"D"_s + "e1"_e = "E"_s
          ,"E"_s = "F"_s
          ,"F"_s = "G"_s
          ,"G"_s = "H"_s

  ); } };

  dsml::Sm<MyMachine> sm{};

  sm.process_event("e1"_e);

  EXPECT_TRUE(sm.is<decltype("H"_s)>());
}

//--------------------------------------------------------------------------

TEST(Sm, DifferentEventsFromTheSameState)
{
  using namespace dsml::literals;
  struct MyMachine { auto operator()() { return dsml::make_transition_table(

          dsml::initial_state + "e1"_e = "A"_s
          , dsml::initial_state + "e2"_e = "B"_s

  ); } };

  {
    dsml::Sm<MyMachine> sm{};

    sm.process_event("e1"_e);

    EXPECT_TRUE(sm.is<decltype("A"_s)>());
  }
  {
    dsml::Sm<MyMachine> sm{};

    sm.process_event("e2"_e);

    EXPECT_TRUE(sm.is<decltype("B"_s)>());
  }
}

//--------------------------------------------------------------------------

TEST(Sm, TransitionLoop)
{
  using namespace dsml::literals;
  struct MyMachine { auto operator()() { return dsml::make_transition_table(

          dsml::initial_state + "e1"_e = dsml::initial_state
          , dsml::initial_state + "e2"_e = "A"_s

  ); } };

  dsml::Sm<MyMachine> sm{};

  sm.process_event("e1"_e);
  EXPECT_TRUE(sm.is<decltype(dsml::initial_state)>());
  sm.process_event("e1"_e);
  EXPECT_TRUE(sm.is<decltype(dsml::initial_state)>());
  sm.process_event("e2"_e);
  EXPECT_TRUE(sm.is<decltype("A"_s)>());
}

//--------------------------------------------------------------------------

TEST(Sm, TransitionAction)
{
  struct Data
  {
    bool called{false};
  };

  using namespace dsml::literals;
  struct MyMachine { auto operator()() { return dsml::make_transition_table(

          dsml::initial_state + "e1"_e
                    / [](Data& data){ data.called = true; }
                    = "A"_s

  ); } };

  Data data{};
  dsml::Sm<MyMachine, Data> sm{data};

  sm.process_event("e1"_e);
  EXPECT_TRUE(sm.is<decltype("A"_s)>());
  EXPECT_TRUE(data.called);
}

//--------------------------------------------------------------------------

TEST(Sm, AnonymousTransitionAction)
{
  struct Data
  {
    bool called{false};
  };

  using namespace dsml::literals;
  struct MyMachine { auto operator()() { return dsml::make_transition_table(

          dsml::initial_state
                    / [](Data& data){ data.called = true; }
                    = "A"_s

  ); } };

  Data data{};
  dsml::Sm<MyMachine, Data> sm{data};

  sm.process_event("e1"_e);
  EXPECT_TRUE(sm.is<decltype("A"_s)>());
  EXPECT_TRUE(data.called);
}

//--------------------------------------------------------------------------

TEST(Sm, TransitionActionDifferentRow)
{
  struct Data
  {
    bool called{false};
  };

  using namespace dsml::literals;
  struct MyMachine { auto operator()() { return dsml::make_transition_table(

          dsml::initial_state + "e1"_e
                    = "A"_s
          ,"A"_s + "e2"_e
                    / [](Data& data){ data.called = true; }
                    = "B"_s

  ); } };

  Data data{};
  dsml::Sm<MyMachine, Data> sm{data};

  sm.process_event("e1"_e);
  EXPECT_TRUE(sm.is<decltype("A"_s)>());
  EXPECT_FALSE(data.called);
  sm.process_event("e2"_e);
  EXPECT_TRUE(sm.is<decltype("B"_s)>());
  EXPECT_TRUE(data.called);
}

//--------------------------------------------------------------------------

TEST(Sm, TransitionGuard)
{
  struct Data
  {
    enum {F1, F2, F3} flag{F2};
  };

  using namespace dsml::literals;
  struct MyMachine { auto operator()() { return dsml::make_transition_table(

          dsml::initial_state + "e1"_e
                    [ ([](const Data& d){ return d.flag == Data::F1; }) ]
                    = "A"_s
          ,dsml::initial_state + "e1"_e
                    [ ([](const Data& d){ return d.flag == Data::F2; }) ]
                    = "B"_s
          ,dsml::initial_state + "e1"_e
                    [ ([](const Data& d){ return d.flag == Data::F3; }) ]
                    = "C"_s

  ); } };

  Data data{};
  dsml::Sm<MyMachine, Data> sm{data};

  sm.process_event("e1"_e);
  EXPECT_TRUE(sm.is<decltype("B"_s)>());
}

//--------------------------------------------------------------------------

TEST(Sm, AnonymousTransitionGuard)
{
  struct Data
  {
    enum {F1, F2, F3} flag{F2};
  };

  using namespace dsml::literals;
  struct MyMachine { auto operator()() { return dsml::make_transition_table(

          dsml::initial_state
                    [ ([](const Data& d){ return d.flag == Data::F1; }) ]
                    = "A"_s
          ,dsml::initial_state
                    [ ([](const Data& d){ return d.flag == Data::F2; }) ]
                    = "B"_s
          ,dsml::initial_state
                    [ ([](const Data& d){ return d.flag == Data::F3; }) ]
                    = "C"_s

  ); } };

  Data data{};
  dsml::Sm<MyMachine, Data> sm{data};

  EXPECT_TRUE(sm.is<decltype("B"_s)>());
}

//--------------------------------------------------------------------------

TEST(Sm, TransitionGuardAndAction)
{
  using V = std::vector<int>;

  using namespace dsml::literals;
  struct MyMachine { auto operator()() { return dsml::make_transition_table(

          dsml::initial_state = "A"_s
          , "A"_s + "e1"_e [ false_guard ] / ([](V& v){v.push_back(1);}) = "B"_s
          , "A"_s + "e1"_e [ true_guard ] / ([](V& v){v.push_back(2);}) = "C"_s

  ); } };

  V calls{};
  dsml::Sm<MyMachine, V> sm{calls};

  sm.process_event("e1"_e);
  EXPECT_EQ(std::vector<int>{{2}}, calls);
  EXPECT_TRUE(sm.is<decltype("C"_s)>());
}

//--------------------------------------------------------------------------

TEST(Sm, AnonymousTransitionGuardAndAction)
{
  using V = std::vector<int>;

  using namespace dsml::literals;
  struct MyMachine { auto operator()() { return dsml::make_transition_table(

          dsml::initial_state = "A"_s
          , "A"_s [ false_guard ] / ([](V& v){v.push_back(1);}) = "B"_s
          , "A"_s [ true_guard ] / ([](V& v){v.push_back(2);}) = "C"_s

  ); } };

  V calls{};
  dsml::Sm<MyMachine, V> sm{calls};

  sm.process_event("e1"_e);
  EXPECT_EQ(std::vector<int>{{2}}, calls);
  EXPECT_TRUE(sm.is<decltype("C"_s)>());
}

//--------------------------------------------------------------------------

TEST(Sm, AnonymousTransitionsDynamicGuardsAndActions)
{
  struct Data
  {
    enum {F1, F2, F3} flag{F2};
    std::vector<int> calls{};
  };

  using namespace dsml::literals;
  struct MyMachine { auto operator()() { return dsml::make_transition_table(

          dsml::initial_state = "A"_s
          , "A"_s
                    [ ([](const Data& d){return d.flag == Data::F1;}) ]
                    / ([](Data& d){
                          d.calls.push_back(1);
                          d.flag = Data::F2;
                        })
                    = "B"_s
          , "A"_s
                    [ ([](const Data& d){return d.flag == Data::F2;}) ]
                    / ([](Data& d){
                          d.calls.push_back(2);
                          d.flag = Data::F3;
                        }) = "C"_s
          , "C"_s
                    [ ([](const Data& d){return d.flag == Data::F2;}) ]
                    / ([](Data& d){
                          d.calls.push_back(3);
                        }) = "D"_s
          , "C"_s
                    [ ([](const Data& d){return d.flag == Data::F3;}) ]
                    / ([](Data& d){
                          d.calls.push_back(4);
                        }) = "E"_s

  ); } };

  Data d;
  dsml::Sm<MyMachine, Data> sm{d};

  EXPECT_EQ((std::vector<int>{{2, 4}}), d.calls);
  EXPECT_TRUE(sm.is<decltype("E"_s)>());
}

//==========================================================================

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
