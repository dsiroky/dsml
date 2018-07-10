#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
#pragma GCC diagnostic ignored "-Wconversion"
#include "gtest/gtest.h"
#pragma GCC diagnostic pop

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

TEST(TypeTraits, IsState)
{
  EXPECT_FALSE(dsml::is_state_v<int>);
  EXPECT_TRUE(dsml::is_state_v<dsml::State<int>>);
  EXPECT_TRUE(dsml::is_state_v<dsml::State<struct S>>);
  EXPECT_TRUE((dsml::is_state_v<dsml::State<struct S>>));
}

TEST(TypeTraits, IsEvent)
{
  EXPECT_FALSE(dsml::is_event_v<int>);
  EXPECT_TRUE(dsml::is_event_v<dsml::Event<int>>);
  EXPECT_TRUE(dsml::is_event_v<dsml::Event<struct S>>);
}

//==========================================================================

void callable1() {}
void callable1noexcept() noexcept {}
auto callable2(double) { return std::make_tuple(4, 5.9f); }

TEST(Callable, Functions)
{
  EXPECT_TRUE((std::is_same<void,
                dsml::detail::Callable<decltype(&callable1)>::ret_t>::value));
  EXPECT_TRUE((std::is_same<std::tuple<>,
                dsml::detail::Callable<decltype(&callable1)>::args_t>::value));
  EXPECT_TRUE((std::is_same<void,
                dsml::detail::Callable<decltype(&callable1noexcept)>::ret_t>::value));
  EXPECT_TRUE((std::is_same<std::tuple<>,
                dsml::detail::Callable<decltype(&callable1noexcept)>::args_t>::value));
  EXPECT_TRUE((std::is_same<std::tuple<int, float>,
                dsml::detail::Callable<decltype(&callable2)>::ret_t>::value));
  EXPECT_TRUE((std::is_same<std::tuple<double>,
                dsml::detail::Callable<decltype(&callable2)>::args_t>::value));
}

//--------------------------------------------------------------------------

TEST(Callable, CallOperators)
{
  struct C { void operator()(int) {} };
  struct CConst { void operator()(int) const {} };
  struct CVolatile { void operator()(int) volatile {} };
  struct CConstVolatile { void operator()(int) const volatile {} };
  struct CNoexcept { void operator()(int) noexcept {} };
  struct CConstNoexcept { void operator()(int) const noexcept {} };

  EXPECT_TRUE((std::is_same<void,
                dsml::detail::Callable<decltype(C{})>::ret_t>::value));
  EXPECT_TRUE((std::is_same<std::tuple<int>,
                dsml::detail::Callable<decltype(C{})>::args_t>::value));
  EXPECT_TRUE((std::is_same<void,
                dsml::detail::Callable<decltype(CConst{})>::ret_t>::value));
  EXPECT_TRUE((std::is_same<std::tuple<int>,
                dsml::detail::Callable<decltype(CConst{})>::args_t>::value));
  EXPECT_TRUE((std::is_same<void,
                dsml::detail::Callable<decltype(CVolatile{})>::ret_t>::value));
  EXPECT_TRUE((std::is_same<std::tuple<int>,
                dsml::detail::Callable<decltype(CVolatile{})>::args_t>::value));
  EXPECT_TRUE((std::is_same<void,
                dsml::detail::Callable<decltype(CConstVolatile{})>::ret_t>::value));
  EXPECT_TRUE((std::is_same<std::tuple<int>,
                dsml::detail::Callable<decltype(CConstVolatile{})>::args_t>::value));
  EXPECT_TRUE((std::is_same<void,
                dsml::detail::Callable<decltype(CNoexcept{})>::ret_t>::value));
  EXPECT_TRUE((std::is_same<std::tuple<int>,
                dsml::detail::Callable<decltype(CNoexcept{})>::args_t>::value));
  EXPECT_TRUE((std::is_same<void,
                dsml::detail::Callable<decltype(CConstNoexcept{})>::ret_t>::value));
  EXPECT_TRUE((std::is_same<std::tuple<int>,
                dsml::detail::Callable<decltype(CConstNoexcept{})>::args_t>::value));
}

//--------------------------------------------------------------------------

TEST(Callable, Lambdas)
{
  auto l = [](){};
  auto l_noexcept = []() noexcept {};
  auto lx = [](int, char&) { return 3.5f; };

  EXPECT_TRUE((std::is_same<void,
                dsml::detail::Callable<decltype(l)>::ret_t>::value));
  EXPECT_TRUE((std::is_same<std::tuple<>,
                dsml::detail::Callable<decltype(l)>::args_t>::value));
  EXPECT_TRUE((std::is_same<void,
                dsml::detail::Callable<decltype(l_noexcept)>::ret_t>::value));
  EXPECT_TRUE((std::is_same<std::tuple<>,
                dsml::detail::Callable<decltype(l_noexcept)>::args_t>::value));
  EXPECT_TRUE((std::is_same<float,
                dsml::detail::Callable<decltype(lx)>::ret_t>::value));
  EXPECT_TRUE((std::is_same<std::tuple<int, char&>,
                dsml::detail::Callable<decltype(lx)>::args_t>::value));
}

//==========================================================================

TEST(GetTypeName, ReturnsString)
{
  EXPECT_STREQ("int", dsml::get_type_name<int>());
  EXPECT_STREQ("float", dsml::get_type_name<float>());
  EXPECT_STREQ("std::tuple<char>", dsml::get_type_name<std::tuple<char>>());
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

//--------------------------------------------------------------------------

TEST(HasTableOperator, False)
{
  EXPECT_FALSE((dsml::detail::HasTableOperator<void>::value));
  EXPECT_FALSE((dsml::detail::HasTableOperator<int>::value));
  EXPECT_FALSE((dsml::detail::HasTableOperator<struct S>::value));

  struct WithCallOperator { int operator()() const noexcept { return 42; } };
  EXPECT_FALSE((dsml::detail::HasTableOperator<WithCallOperator>::value));
}

TEST(HasTableOperator, True)
{
  using namespace dsml::literals;
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state = "A"_s

  ); } };

  EXPECT_TRUE((dsml::detail::HasTableOperator<MyMachine>::value));
}

//==========================================================================

TEST(Callee, NormalMethod)
{
  struct S
  {
    int f()
    {
      x = 5;
      return 3;
    }

    int x{};
  };

  const auto c = dsml::callee(&S::f);
  S s{};

  EXPECT_EQ(3, c(s));
  EXPECT_EQ(5, s.x);
}

//--------------------------------------------------------------------------

TEST(Callee, ConstVolatileMethods)
{
  struct S
  {
    int f_c() const { return 3; }
    int f_v() volatile { return 4; }
    int f_cv() const volatile { return 5; }
  };

  S s{};

  EXPECT_EQ(3, dsml::callee(&S::f_c)(s));
  EXPECT_EQ(4, dsml::callee(&S::f_v)(s));
  EXPECT_EQ(5, dsml::callee(&S::f_cv)(s));
}

//--------------------------------------------------------------------------

TEST(Callee, ReturnTypeIsVoid)
{
  struct S
  {
    void f()
    {
      x = 99;
    }

    int x{};
  };

  const auto c = dsml::callee(&S::f);
  S s{};

  c(s);
  EXPECT_EQ(99, s.x);
}

//==========================================================================

TEST(Sm, OnlyInitialStateAndAnonymousTransition_IsInTheSecondState)
{
  using namespace dsml::literals;
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state = "A"_s

  ); } };
  dsml::Sm<MyMachine> sm{};

  EXPECT_FALSE(sm.is(dsml::initial_state));
  EXPECT_TRUE(sm.is("A"_s));
}

//--------------------------------------------------------------------------

TEST(Sm, OnlyInitialStateAndTransition_IsInTheInitialState)
{
  using namespace dsml::literals;
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state + "e1"_e = "A"_s

  ); } };
  dsml::Sm<MyMachine> sm{};

  EXPECT_TRUE(sm.is(dsml::initial_state));
  EXPECT_FALSE(sm.is("A"_s));
}

//--------------------------------------------------------------------------

TEST(Sm, SingleTransition)
{
  using namespace dsml::literals;
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state + "e1"_e = "A"_s

  ); } };
  dsml::Sm<MyMachine> sm{};

  sm.process_event("e1"_e);

  EXPECT_FALSE(sm.is(dsml::initial_state));
  EXPECT_TRUE(sm.is("A"_s));
}

//--------------------------------------------------------------------------

TEST(Sm, SingleTransitionUnknownEvent_NoStateChange)
{
  using namespace dsml::literals;
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state + "e1"_e = "A"_s

  ); } };
  dsml::Sm<MyMachine> sm{};

  sm.process_event("eunk"_e);

  EXPECT_TRUE(sm.is(dsml::initial_state));
}

//--------------------------------------------------------------------------

TEST(Sm, MultipleTransitionsSameEvents)
{
  using namespace dsml::literals;
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state + "e1"_e = "A"_s
          ,"A"_s + "e1"_e = "B"_s
          ,"B"_s + "e1"_e = "C"_s

  ); } };

  {
    dsml::Sm<MyMachine> sm{};

    sm.process_event("e1"_e);

    EXPECT_TRUE(sm.is("A"_s));
  }
  {
    dsml::Sm<MyMachine> sm{};

    sm.process_event("e1"_e);
    sm.process_event("e1"_e);

    EXPECT_TRUE(sm.is("B"_s));
  }
  {
    dsml::Sm<MyMachine> sm{};

    sm.process_event("e1"_e);
    sm.process_event("e1"_e);
    sm.process_event("e1"_e);

    EXPECT_TRUE(sm.is("C"_s));
  }
}

//--------------------------------------------------------------------------

TEST(Sm, MultipleTransitionsDifferentEvents)
{
  using namespace dsml::literals;
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state + "e1"_e = "A"_s
          ,"A"_s + "e2"_e = "B"_s
          ,"B"_s + "e3"_e = "C"_s

  ); } };

  {
    dsml::Sm<MyMachine> sm{};

    sm.process_event("e1"_e);

    EXPECT_TRUE(sm.is("A"_s));
  }
  {
    dsml::Sm<MyMachine> sm{};

    sm.process_event("e1"_e);
    sm.process_event("e2"_e);

    EXPECT_TRUE(sm.is("B"_s));
  }
  {
    dsml::Sm<MyMachine> sm{};

    sm.process_event("e1"_e);
    sm.process_event("e2"_e);
    sm.process_event("e3"_e);

    EXPECT_TRUE(sm.is("C"_s));
  }
}

//--------------------------------------------------------------------------

TEST(Sm, AnonymousEventAfterNormalEvent)
{
  using namespace dsml::literals;
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state + "e1"_e = "A"_s
          ,"A"_s = "B"_s

  ); } };

  dsml::Sm<MyMachine> sm{};

  sm.process_event("e1"_e);

  EXPECT_TRUE(sm.is("B"_s));
}

//--------------------------------------------------------------------------

TEST(Sm, AnonymousEventBeforeNormalEvent)
{
  using namespace dsml::literals;
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state = "A"_s
          ,"A"_s + "e1"_e = "B"_s

  ); } };

  dsml::Sm<MyMachine> sm{};

  sm.process_event("e1"_e);

  EXPECT_TRUE(sm.is("B"_s));
}

//--------------------------------------------------------------------------

TEST(Sm, MultipleAnonymousEvents)
{
  using namespace dsml::literals;
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state = "A"_s
          ,"A"_s = "B"_s
          ,"B"_s = "C"_s
          ,"C"_s = "D"_s

  ); } };

  dsml::Sm<MyMachine> sm{};

  EXPECT_TRUE(sm.is("D"_s));
}

//--------------------------------------------------------------------------

TEST(Sm, MultipleAnonymousEventsAroundNormalEvent)
{
  using namespace dsml::literals;
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

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

  EXPECT_TRUE(sm.is("H"_s));
}

//--------------------------------------------------------------------------

TEST(Sm, DifferentEventsFromTheSameState)
{
  using namespace dsml::literals;
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state + "e1"_e = "A"_s
          , dsml::initial_state + "e2"_e = "B"_s

  ); } };

  {
    dsml::Sm<MyMachine> sm{};

    sm.process_event("e1"_e);

    EXPECT_TRUE(sm.is("A"_s));
  }
  {
    dsml::Sm<MyMachine> sm{};

    sm.process_event("e2"_e);

    EXPECT_TRUE(sm.is("B"_s));
  }
}

//--------------------------------------------------------------------------

TEST(Sm, TransitionLoop)
{
  using namespace dsml::literals;
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state + "e1"_e = dsml::initial_state
          , dsml::initial_state + "e2"_e = "A"_s

  ); } };

  dsml::Sm<MyMachine> sm{};

  sm.process_event("e1"_e);
  EXPECT_TRUE(sm.is(dsml::initial_state));
  sm.process_event("e1"_e);
  EXPECT_TRUE(sm.is(dsml::initial_state));
  sm.process_event("e2"_e);
  EXPECT_TRUE(sm.is("A"_s));
}

//--------------------------------------------------------------------------

TEST(Sm, TransitionAction)
{
  struct Data
  {
    bool called{false};
  };

  using namespace dsml::literals;
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state + "e1"_e
                    / [](Data& data){ data.called = true; }
                    = "A"_s

  ); } };

  Data data{};
  dsml::Sm<MyMachine, Data> sm{data};

  sm.process_event("e1"_e);
  EXPECT_TRUE(sm.is("A"_s));
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
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state
                    / [](Data& data){ data.called = true; }
                    = "A"_s

  ); } };

  Data data{};
  dsml::Sm<MyMachine, Data> sm{data};

  sm.process_event("e1"_e);
  EXPECT_TRUE(sm.is("A"_s));
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
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state + "e1"_e
                    = "A"_s
          ,"A"_s + "e2"_e
                    / [](Data& data){ data.called = true; }
                    = "B"_s

  ); } };

  Data data{};
  dsml::Sm<MyMachine, Data> sm{data};

  sm.process_event("e1"_e);
  EXPECT_TRUE(sm.is("A"_s));
  EXPECT_FALSE(data.called);
  sm.process_event("e2"_e);
  EXPECT_TRUE(sm.is("B"_s));
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
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

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
  EXPECT_TRUE(sm.is("B"_s));
}

//--------------------------------------------------------------------------

TEST(Sm, TransitionGuardWithNotOperator)
{
  bool flag{true};

  using namespace dsml::literals;
  using namespace dsml::guard_operators;

  struct MyMachine { auto operator()() const noexcept {
          auto guard = [](const bool& flag_){ return flag_; };
          return dsml::make_transition_table(
                          dsml::initial_state + "e1"_e [ not guard ] = "A"_s
                      );
  } };

  dsml::Sm<MyMachine, bool> sm{flag};

  sm.process_event("e1"_e);
  EXPECT_TRUE(sm.is(dsml::initial_state));
  flag = false;
  sm.process_event("e1"_e);
  EXPECT_TRUE(sm.is("A"_s));
}

//--------------------------------------------------------------------------

TEST(Sm, TransitionGuardWithAndOperator)
{
  struct Data
  {
    bool flag1{false};
    bool flag2{false};
  };

  using namespace dsml::literals;
  using namespace dsml::guard_operators;

  struct MyMachine { auto operator()() const noexcept {
          auto guard1 = [](Data& d){ return d.flag1; };
          auto guard2 = [](Data& d){ return d.flag2; };
          return dsml::make_transition_table(
                      dsml::initial_state + "e1"_e [ guard1 and guard2 ] = "A"_s
                  );
  } };

  {
    Data data{};
    data.flag1 = false;
    data.flag2 = false;
    dsml::Sm<MyMachine, Data> sm{data};
    sm.process_event("e1"_e);
    EXPECT_TRUE(sm.is(dsml::initial_state));
  }
  {
    Data data{};
    dsml::Sm<MyMachine, Data> sm{data};
    data.flag1 = true;
    data.flag2 = false;
    sm.process_event("e1"_e);
    EXPECT_TRUE(sm.is(dsml::initial_state));
  }
  {
    Data data{};
    dsml::Sm<MyMachine, Data> sm{data};
    data.flag1 = false;
    data.flag2 = true;
    sm.process_event("e1"_e);
    EXPECT_TRUE(sm.is(dsml::initial_state));
  }
  {
    Data data{};
    dsml::Sm<MyMachine, Data> sm{data};
    data.flag1 = true;
    data.flag2 = true;
    sm.process_event("e1"_e);
    EXPECT_TRUE(sm.is("A"_s));
  }
}

//--------------------------------------------------------------------------

TEST(Sm, TransitionGuardWithOrOperator)
{
  struct Data
  {
    bool flag1{false};
    bool flag2{false};
  };

  using namespace dsml::literals;
  using namespace dsml::guard_operators;

  struct MyMachine { auto operator()() const noexcept {
          auto guard1 = [](const Data& d){ return d.flag1; };
          auto guard2 = [](const Data& d){ return d.flag2; };
          return dsml::make_transition_table(
                      dsml::initial_state + "e1"_e [ guard1 or guard2 ] = "A"_s
                  );
  } };

  {
    Data data{};
    data.flag1 = false;
    data.flag2 = false;
    dsml::Sm<MyMachine, Data> sm{data};
    sm.process_event("e1"_e);
    EXPECT_TRUE(sm.is(dsml::initial_state));
  }
  {
    Data data{};
    dsml::Sm<MyMachine, Data> sm{data};
    data.flag1 = true;
    data.flag2 = false;
    sm.process_event("e1"_e);
    EXPECT_TRUE(sm.is("A"_s));
  }
  {
    Data data{};
    dsml::Sm<MyMachine, Data> sm{data};
    data.flag1 = false;
    data.flag2 = true;
    sm.process_event("e1"_e);
    EXPECT_TRUE(sm.is("A"_s));
  }
  {
    Data data{};
    dsml::Sm<MyMachine, Data> sm{data};
    data.flag1 = true;
    data.flag2 = true;
    sm.process_event("e1"_e);
    EXPECT_TRUE(sm.is("A"_s));
  }
}

//--------------------------------------------------------------------------

TEST(Sm, AnonymousTransitionGuard)
{
  struct Data
  {
    enum {F1, F2, F3} flag{F2};
  };

  using namespace dsml::literals;
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

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

  EXPECT_TRUE(sm.is("B"_s));
}

//--------------------------------------------------------------------------

TEST(Sm, TransitionGuardAndAction)
{
  using V = std::vector<int>;

  using namespace dsml::literals;
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state = "A"_s
          , "A"_s + "e1"_e [ false_guard ] / ([](V& v){v.push_back(1);}) = "B"_s
          , "A"_s + "e1"_e [ true_guard ] / ([](V& v){v.push_back(2);}) = "C"_s

  ); } };

  V calls{};
  dsml::Sm<MyMachine, V> sm{calls};

  sm.process_event("e1"_e);
  EXPECT_EQ(std::vector<int>{2}, calls);
  EXPECT_TRUE(sm.is("C"_s));
}

//--------------------------------------------------------------------------

TEST(Sm, AnonymousTransitionGuardAndAction)
{
  using V = std::vector<int>;

  using namespace dsml::literals;
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state = "A"_s
          , "A"_s [ false_guard ] / ([](V& v){v.push_back(1);}) = "B"_s
          , "A"_s [ true_guard ] / ([](V& v){v.push_back(2);}) = "C"_s

  ); } };

  V calls{};
  dsml::Sm<MyMachine, V> sm{calls};

  sm.process_event("e1"_e);
  EXPECT_EQ(std::vector<int>{2}, calls);
  EXPECT_TRUE(sm.is("C"_s));
}

//--------------------------------------------------------------------------

TEST(Sm, GuardAndActionAsMethodPointer)
{
  struct Logic
  {
    bool guard() const
    {
      return x > 0;
    }

    void action()
    {
      x = 10;
    }

    int x{0};
  };

  using namespace dsml::literals;
  struct MyMachine { auto operator()() const noexcept {

    using dsml::callee;
    using namespace dsml::guard_operators;

    return dsml::make_transition_table(
          dsml::initial_state = "A"_s
          , "A"_s + "evt"_e
                [ callee(&Logic::guard) ]
                / callee(&Logic::action)
                = "B"_s
          , "A"_s + "evt"_e
                [ not callee(&Logic::guard) ]
                = "A"_s
          , "A"_s + "evt"_e
                [ callee(&Logic::guard) and not callee(&Logic::guard) ]
                = "A"_s
  ); } };

  {
    Logic logic{};
    dsml::Sm<MyMachine, Logic> sm{logic};
    sm.process_event("evt"_e);
    EXPECT_TRUE(sm.is("A"_s));
  }
  {
    Logic logic{};
    dsml::Sm<MyMachine, Logic> sm{logic};
    logic.x = 5;
    sm.process_event("evt"_e);
    EXPECT_TRUE(sm.is("B"_s));
    EXPECT_EQ(10, logic.x);
  }
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
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

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
  EXPECT_TRUE(sm.is("E"_s));
}

//--------------------------------------------------------------------------

TEST(Sm, OnEntry)
{
  struct Data
  {
    bool called{false};
  };

  using namespace dsml::literals;
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state + "e"_e = "A"_s
          , "A"_s + dsml::on_entry / [](Data& data){ data.called = true; }

  ); } };

  Data data{};
  dsml::Sm<MyMachine, Data> sm{data};

  EXPECT_FALSE(data.called);
  sm.process_event("e"_e);
  EXPECT_TRUE(sm.is("A"_s));
  EXPECT_TRUE(data.called);
}

//--------------------------------------------------------------------------

TEST(Sm, OnEntryOnExit)
{
  struct Data
  {
    std::vector<int> calls{};
  };

  using namespace dsml::literals;
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state + "e"_e = "A"_s
          , "A"_s + dsml::on_entry / [](Data& data){ data.calls.push_back(2); }
          , dsml::initial_state + dsml::on_exit / [](Data& data) { data.calls.push_back(1); }

  ); } };

  Data data{};
  dsml::Sm<MyMachine, Data> sm{data};

  sm.process_event("e"_e);
  EXPECT_EQ((std::vector<int>{{1, 2}}), data.calls);
}

//--------------------------------------------------------------------------

TEST(Sm, OnEntryOnExitAndActionBetween)
{
  struct Data
  {
    std::vector<int> calls{};
  };

  using namespace dsml::literals;
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state + "e"_e / [](Data& data) { data.calls.push_back(0); } = "A"_s
          , "A"_s + dsml::on_entry / [](Data& data){ data.calls.push_back(2); }
          , dsml::initial_state + dsml::on_exit / [](Data& data) { data.calls.push_back(1); }

  ); } };

  Data data{};
  dsml::Sm<MyMachine, Data> sm{data};

  sm.process_event("e"_e);
  EXPECT_EQ((std::vector<int>{{1, 0, 2}}), data.calls);
}

//--------------------------------------------------------------------------

TEST(Sm, OnEntryOnExitMultipleStates)
{
  struct Data
  {
    std::vector<int> calls{};
  };

  using namespace dsml::literals;
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state + "e1"_e = "A"_s
          , "A"_s + "e2"_e = "B"_s

          , dsml::initial_state + dsml::on_entry / [](Data& data) { data.calls.push_back(0); }
          , dsml::initial_state + dsml::on_exit / [](Data& data) { data.calls.push_back(1); }
          , "A"_s + dsml::on_entry / [](Data& data){ data.calls.push_back(2); }
          , "A"_s + dsml::on_exit / [](Data& data){ data.calls.push_back(3); }
          , "B"_s + dsml::on_entry / [](Data& data){ data.calls.push_back(4); }
          , "B"_s + dsml::on_exit / [](Data& data){ data.calls.push_back(5); }

  ); } };

  Data data{};
  dsml::Sm<MyMachine, Data> sm{data};

  sm.process_event("e1"_e);
  sm.process_event("e2"_e);
  EXPECT_EQ((std::vector<int>{{1, 2, 3, 4}}), data.calls);
}

//==========================================================================

TEST(SmComposite, AnonymousTransitions_IsInTheSubStateInitialState)
{
  using namespace dsml::literals;
  struct Sub { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state = "SubA"_s

  ); } };
  struct Composite { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state = dsml::State<Sub>{}

  ); } };
  dsml::Sm<Composite> sm{};

  EXPECT_FALSE(sm.is(dsml::initial_state));
  EXPECT_TRUE(sm.is_sub("SubA"_s, Sub{}));
}

//--------------------------------------------------------------------------

TEST(SmComposite, TransitionInComposite_IsInTheCompositeStateInitialState)
{
  using namespace dsml::literals;
  struct Sub { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state = "SubA"_s

  ); } };
  struct Composite { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state + "e1"_e = dsml::State<Sub>{}

  ); } };
  dsml::Sm<Composite> sm{};

  EXPECT_TRUE(sm.is(dsml::initial_state));
  EXPECT_FALSE(sm.is_sub("SubA"_s, Sub{}));
}

//--------------------------------------------------------------------------

TEST(SmComposite, TransitionInComposite_ProcessEvent_IsInTheSubStateA)
{
  using namespace dsml::literals;
  struct Sub { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state = "SubA"_s

  ); } };
  struct Composite { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state + "e1"_e = dsml::State<Sub>{}

  ); } };
  dsml::Sm<Composite> sm{};

  sm.process_event("e1"_e);

  EXPECT_FALSE(sm.is(dsml::initial_state));
  EXPECT_TRUE(sm.is_sub("SubA"_s, Sub{}));
}

//--------------------------------------------------------------------------

TEST(SmMultilevelComposite, AnonymousTransitions_IsInTheSubStateInitialState)
{
  using namespace dsml::literals;
  struct SubSub { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state = "SubA"_s

  ); } };
  struct Sub { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state = dsml::State<SubSub>{}

  ); } };
  struct Composite { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state = dsml::State<Sub>{}

  ); } };
  dsml::Sm<Composite> sm{};

  EXPECT_FALSE(sm.is(dsml::initial_state));
  EXPECT_FALSE(sm.is_sub(dsml::initial_state, Sub{}));
  EXPECT_TRUE(sm.is_sub("SubA"_s, Sub{}, SubSub{}));
}

//--------------------------------------------------------------------------

TEST(SmMultilevelComposite, AnonymousTransitions_InAndOut)
{
  using namespace dsml::literals;
  struct SubSub { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state = "SubA"_s
          , "SubA"_s = dsml::final_state

  ); } };
  struct Sub { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state = dsml::State<SubSub>{}
          , dsml::State<SubSub>{} = dsml::final_state

  ); } };
  struct Composite { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state = dsml::State<Sub>{}
          , dsml::State<Sub>{} = dsml::final_state

  ); } };
  dsml::Sm<Composite> sm{};

  EXPECT_FALSE(sm.is(dsml::initial_state));
  EXPECT_TRUE(sm.is(dsml::final_state));
  EXPECT_FALSE(sm.is_sub(dsml::initial_state, Sub{}));
  EXPECT_FALSE(sm.is_sub(dsml::final_state, Sub{}));
  EXPECT_FALSE(sm.is_sub(dsml::initial_state, Sub{}, SubSub{}));
  EXPECT_FALSE(sm.is_sub("SubA"_s, Sub{}, SubSub{}));
  EXPECT_FALSE(sm.is_sub(dsml::final_state, Sub{}, SubSub{}));
}

//--------------------------------------------------------------------------

TEST(SmComposite, OnEntryOnExit)
{
  struct Data
  {
    std::vector<int> calls{};
  };

  using namespace dsml::literals;
  struct Sub { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state = "SubA"_s
          , "SubA"_s = dsml::final_state
          , "SubA"_s + dsml::on_entry / [](Data& data) { data.calls.push_back(1); }
          , "SubA"_s + dsml::on_exit / [](Data& data) { data.calls.push_back(2); }

  ); } };
  struct Composite { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state = "A"_s
          , "A"_s = dsml::State<Sub>{}
          , dsml::State<Sub>{} = "B"_s
          , "A"_s + dsml::on_entry / [](Data& data) { data.calls.push_back(3); }
          , "A"_s + dsml::on_exit / [](Data& data) { data.calls.push_back(4); }
          , "B"_s + dsml::on_entry / [](Data& data) { data.calls.push_back(5); }

  ); } };

  Data data{};
  dsml::Sm<Composite, Data> sm{data};

  EXPECT_EQ((std::vector<int>{{3, 4, 1, 2, 5}}), data.calls);
}

//--------------------------------------------------------------------------

TEST(SmMultilevelComposite, Mixture)
{
  struct Data
  {
    enum {F1, F2, F3} flag{F2};
    std::vector<int> calls{};
  };

  using namespace dsml::literals;

  struct SubSub { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state + "e1"_e
              / [](Data& d){ d.calls.push_back(1); }
              = "SubA"_s
          , "SubA"_s = dsml::final_state

  ); } };
  struct Sub { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state = dsml::State<SubSub>{}
          , dsml::State<SubSub>{} = "A"_s
          , "A"_s [ false_guard ] / ([](Data& d){d.calls.push_back(2);}) = "B"_s
          , "A"_s [ true_guard ] / ([](Data& d){d.calls.push_back(3);}) = "C"_s
          , "B"_s = dsml::final_state
          , "C"_s = dsml::final_state

  ); } };
  using namespace dsml::literals;
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state = dsml::State<Sub>{}

  ); } };

  Data data{};
  dsml::Sm<MyMachine, Data> sm{data};

  sm.process_event("e1"_e);
  EXPECT_EQ((std::vector<int>{{1, 3}}), data.calls);
}

//==========================================================================

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
