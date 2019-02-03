#ifndef _MSC_VER
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wdeprecated"
  #pragma GCC diagnostic ignored "-Wconversion"
#endif

#include "gtest/gtest.h"

#ifndef _MSC_VER
  #pragma GCC diagnostic pop
#endif

#include "dsml.hpp"

//==========================================================================

const auto true_guard = [](){ return true; };
const auto false_guard = [](){ return false; };
const auto no_action = [](){};
bool true_guard_func() { return true; }
void no_action_func() {}

void callable1() {}
void callable1noexcept() noexcept {}
auto callable2(double) { return std::make_tuple(4, 5.9f); }
auto callable3(int a, int b) { return a + b; }
const auto const_callable2_ptr = &callable2;

/// just a warning silencer
void dont_warn_about_not_needed()
{
  (void) const_callable2_ptr;
}

//==========================================================================

#define STATE(x) static constexpr auto x = dsml::State<struct x##_>{}
STATE(A);
STATE(B);
STATE(C);
STATE(D);
STATE(E);
STATE(F);
STATE(G);
STATE(H);
STATE(SubA);

#define EVENT(x) static constexpr auto x = dsml::Event<struct x##_>{}
EVENT(e1);
EVENT(e2);
EVENT(e3);
EVENT(e4);

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

TEST(ConcatTypes, Misc)
{
  EXPECT_TRUE((std::is_same<
        dsml::detail::ConcatTypes_t<std::tuple<>>,
        std::tuple<>
      >::value));
  EXPECT_TRUE((std::is_same<
        dsml::detail::ConcatTypes_t<std::tuple<int>>,
        std::tuple<int>
      >::value));
  EXPECT_TRUE((std::is_same<
        dsml::detail::ConcatTypes_t<std::tuple<int, char>>,
        std::tuple<int, char>
      >::value));
  EXPECT_TRUE((std::is_same<
        dsml::detail::ConcatTypes_t<std::tuple<>, std::tuple<int, char>>,
        std::tuple<int, char>
      >::value));
  EXPECT_TRUE((std::is_same<
        dsml::detail::ConcatTypes_t<std::tuple<int, char>, std::tuple<>>,
        std::tuple<int, char>
      >::value));
  EXPECT_TRUE((std::is_same<
        dsml::detail::ConcatTypes_t<std::tuple<int, char>, std::tuple<bool>>,
        std::tuple<int, char, bool>
      >::value));
  EXPECT_TRUE((std::is_same<
        dsml::detail::ConcatTypes_t<std::tuple<int, char>, std::tuple<bool, int>>,
        std::tuple<int, char, bool, int>
      >::value));
  EXPECT_TRUE((std::is_same<
        dsml::detail::ConcatTypes_t<
                            std::tuple<int, char>,
                            std::tuple<bool, int>,
                            std::tuple<>
                          >,
        std::tuple<int, char, bool, int>
      >::value));
  EXPECT_TRUE((std::is_same<
        dsml::detail::ConcatTypes_t<
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
        dsml::detail::UniqueTypes_t<std::tuple<>>,
        std::tuple<>
      >::value));

  EXPECT_TRUE((std::is_same<
        dsml::detail::UniqueTypes_t<std::tuple<int>>,
        std::tuple<int>
      >::value));

  EXPECT_TRUE((std::is_same<
        dsml::detail::UniqueTypes_t<std::tuple<int, bool>>,
        std::tuple<int, bool>
      >::value));

  EXPECT_TRUE((std::is_same<
        dsml::detail::UniqueTypes_t<std::tuple<int, bool, double>>,
        std::tuple<int, bool, double>
      >::value));

  EXPECT_TRUE((std::is_same<
        dsml::detail::UniqueTypes_t<std::tuple<int, int>>,
        std::tuple<int>
      >::value));

  EXPECT_TRUE((std::is_same<
        dsml::detail::UniqueTypes_t<std::tuple<int, int, bool>>,
        std::tuple<int, bool>
      >::value));

  EXPECT_TRUE((std::is_same<
        dsml::detail::UniqueTypes_t<std::tuple<int, bool, int>>,
        std::tuple<bool, int>
      >::value));

  EXPECT_TRUE((std::is_same<
        dsml::detail::UniqueTypes_t<std::tuple<bool, int, int>>,
        std::tuple<bool, int>
      >::value));

  EXPECT_TRUE((std::is_same<
        dsml::detail::UniqueTypes_t<std::tuple<
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
  EXPECT_FALSE(dsml::IsState<int>::value);
  EXPECT_TRUE(dsml::IsState<dsml::State<int>>::value);
  EXPECT_TRUE(dsml::IsState<dsml::State<struct S>>::value);
  EXPECT_TRUE((dsml::IsState<dsml::State<struct S>>::value));
}

TEST(TypeTraits, IsEvent)
{
  EXPECT_FALSE(dsml::IsEvent<int>::value);
  EXPECT_TRUE(dsml::IsEvent<dsml::Event<int>>::value);
  EXPECT_TRUE(dsml::IsEvent<dsml::Event<struct S>>::value);
}

TEST(TypeTraits, IsGuard)
{
  EXPECT_FALSE(dsml::detail::IsGuard<int>::value);
  const auto lambda_val = dsml::detail::IsGuard<decltype(true_guard)>::value;
  EXPECT_TRUE(lambda_val);
  const auto func_val = dsml::detail::IsGuard<decltype(&true_guard_func)>::value;
  EXPECT_TRUE(func_val);
}

TEST(TypeTraits, IsAction)
{
  EXPECT_FALSE(dsml::detail::IsAction<int>::value);
  const auto lambda_val = dsml::detail::IsAction<decltype(no_action)>::value;
  EXPECT_TRUE(lambda_val);
  const auto func_val = dsml::detail::IsAction<decltype(&no_action_func)>::value;
  EXPECT_TRUE(func_val);
}

//==========================================================================

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
  EXPECT_TRUE((std::is_same<std::tuple<int, float>,
                dsml::detail::Callable<decltype(const_callable2_ptr)>::ret_t>::value));
  EXPECT_TRUE((std::is_same<std::tuple<double>,
                dsml::detail::Callable<decltype(const_callable2_ptr)>::args_t>::value));
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
#ifdef _MSC_VER
  EXPECT_STREQ("class std::tuple<char>", dsml::get_type_name<std::tuple<char>>());
#else
  EXPECT_STREQ("std::tuple<char>", dsml::get_type_name<std::tuple<char>>());
#endif
}

//==========================================================================

struct WithCStr
{
  static const char* c_str() { return "foo"; };
};


TEST(HasStaticCStr, Value)
{
  using namespace dsml::literals;

  EXPECT_FALSE(dsml::detail::HasStaticCStr<int>::value);
#ifndef _MSC_VER
  constexpr auto udl_cstr = dsml::detail::HasStaticCStr<decltype("hello"_s)>::value;
  EXPECT_TRUE(udl_cstr);
#endif
  constexpr auto s_cstr = dsml::detail::HasStaticCStr<WithCStr>::value;
  EXPECT_TRUE(s_cstr);
}

TEST(Stringify, ReturnsString)
{
  using namespace dsml::literals;

  EXPECT_STREQ("int", dsml::detail::c_str<int>());
#ifndef _MSC_VER
  EXPECT_STREQ("hello", dsml::detail::c_str<decltype("hello"_s)>());
#endif
  EXPECT_STREQ("foo", dsml::detail::c_str<WithCStr>());
}

//==========================================================================

TEST(RowWithEvent, NoMatch_EmptyTuple)
{
  const auto rows = std::make_tuple(

          dsml::initial_state = A
          , A + e1 = B
          , B + e2 = C

    );

  const auto transitions = dsml::detail::rows_with_event(rows, e4);

  EXPECT_EQ(0u, std::tuple_size<decltype(transitions)>::value);
}

//--------------------------------------------------------------------------

TEST(RowWithEvent, SingleTransition)
{
  const auto rows = std::make_tuple(

          dsml::initial_state = A
          , A + e1 = B
          , B + e2 = C

    );

  const auto transitions = dsml::detail::rows_with_event(rows, e1);

  EXPECT_EQ(1u, std::tuple_size<decltype(transitions)>::value);
  using item0_t = std::tuple_element_t<0, decltype(transitions)>;
  EXPECT_TRUE((std::is_same<
        std::decay_t<item0_t>::src_state_t,
        std::decay_t<decltype(A)>
      >::value));
  EXPECT_TRUE((std::is_same<
        std::decay_t<item0_t>::dst_state_t,
        std::decay_t<decltype(B)>
      >::value));
}

//--------------------------------------------------------------------------

TEST(RowWithEvent, MultipleTransitions)
{
  const auto rows = std::make_tuple(

          dsml::initial_state = A
          , A + e1 = B
          , B + e2 = C
          , B + e1 = D
          , B + e3 = A

    );

  const auto transitions = dsml::detail::rows_with_event(rows, e1);

  EXPECT_EQ(2u, std::tuple_size<decltype(transitions)>::value);

  using item0_t = std::tuple_element_t<0, decltype(transitions)>;
  EXPECT_TRUE((std::is_same<
        std::decay_t<item0_t>::src_state_t,
        std::decay_t<decltype(A)>
      >::value));
  EXPECT_TRUE((std::is_same<
        std::decay_t<item0_t>::dst_state_t,
        std::decay_t<decltype(B)>
      >::value));

  using item1_t = std::tuple_element_t<1, decltype(transitions)>;
  EXPECT_TRUE((std::is_same<
        std::decay_t<item1_t>::src_state_t,
        std::decay_t<decltype(B)>
      >::value));
  EXPECT_TRUE((std::is_same<
        std::decay_t<item1_t>::dst_state_t,
        std::decay_t<decltype(D)>
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
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state = A

  ); } };

  EXPECT_TRUE((dsml::detail::HasTableOperator<MyMachine>::value));
}

//==========================================================================

TEST(Callee, FreeFunction)
{
  const auto c = dsml::callee(callable3);

  EXPECT_EQ(5, c(2, 3));
}

//--------------------------------------------------------------------------

TEST(Callee, Lambda)
{
  const auto l = [](int a, int b){ return a * b; };

  const auto c = dsml::callee(l);

  EXPECT_EQ(6, c(2, 3));
}

//--------------------------------------------------------------------------

TEST(Callee, MemberFunction)
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

TEST(Callee, ConstVolatileMemberFunctions)
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
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state = A

  ); } };
  dsml::Sm<MyMachine> sm{};

  EXPECT_FALSE(sm.is(dsml::initial_state));
  EXPECT_TRUE(sm.is(A));
}

//--------------------------------------------------------------------------

TEST(Sm, OnlyInitialStateAndTransition_IsInTheInitialState)
{
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state + e1 = A

  ); } };
  dsml::Sm<MyMachine> sm{};

  EXPECT_TRUE(sm.is(dsml::initial_state));
  EXPECT_FALSE(sm.is(A));
}

//--------------------------------------------------------------------------

TEST(Sm, SingleTransition)
{
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state + e1 = A

  ); } };
  dsml::Sm<MyMachine> sm{};

  sm.process_event(e1);

  EXPECT_FALSE(sm.is(dsml::initial_state));
  EXPECT_TRUE(sm.is(A));
}

//--------------------------------------------------------------------------

TEST(Sm, SingleTransitionUnknownEvent_NoStateChange)
{
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state + e1 = A

  ); } };
  dsml::Sm<MyMachine> sm{};

  EVENT(eunk);
  sm.process_event(eunk);

  EXPECT_TRUE(sm.is(dsml::initial_state));
}

//--------------------------------------------------------------------------

TEST(Sm, MultipleTransitionsSameEvents)
{
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state + e1 = A
          ,A + e1 = B
          ,B + e1 = C

  ); } };

  {
    dsml::Sm<MyMachine> sm{};

    sm.process_event(e1);

    EXPECT_TRUE(sm.is(A));
  }
  {
    dsml::Sm<MyMachine> sm{};

    sm.process_event(e1);
    sm.process_event(e1);

    EXPECT_TRUE(sm.is(B));
  }
  {
    dsml::Sm<MyMachine> sm{};

    sm.process_event(e1);
    sm.process_event(e1);
    sm.process_event(e1);

    EXPECT_TRUE(sm.is(C));
  }
}

//--------------------------------------------------------------------------

TEST(Sm, MultipleTransitionsDifferentEvents)
{
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state + e1 = A
          ,A + e2 = B
          ,B + e3 = C

  ); } };

  {
    dsml::Sm<MyMachine> sm{};

    sm.process_event(e1);

    EXPECT_TRUE(sm.is(A));
  }
  {
    dsml::Sm<MyMachine> sm{};

    sm.process_event(e1);
    sm.process_event(e2);

    EXPECT_TRUE(sm.is(B));
  }
  {
    dsml::Sm<MyMachine> sm{};

    sm.process_event(e1);
    sm.process_event(e2);
    sm.process_event(e3);

    EXPECT_TRUE(sm.is(C));
  }
}

//--------------------------------------------------------------------------

TEST(Sm, AnonymousEventAfterNormalEvent)
{
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state + e1 = A
          ,A = B

  ); } };

  dsml::Sm<MyMachine> sm{};

  sm.process_event(e1);

  EXPECT_TRUE(sm.is(B));
}

//--------------------------------------------------------------------------

TEST(Sm, AnonymousEventBeforeNormalEvent)
{
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state = A
          ,A + e1 = B

  ); } };

  dsml::Sm<MyMachine> sm{};

  sm.process_event(e1);

  EXPECT_TRUE(sm.is(B));
}

//--------------------------------------------------------------------------

TEST(Sm, MultipleAnonymousEvents)
{
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state = A
          ,A = B
          ,B = C
          ,C = D

  ); } };

  dsml::Sm<MyMachine> sm{};

  EXPECT_TRUE(sm.is(D));
}

//--------------------------------------------------------------------------

TEST(Sm, MultipleAnonymousEventsAroundNormalEvent)
{
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state = A
          ,A = B
          ,B = C
          ,C = D
          ,D + e1 = E
          ,E = F
          ,F = G
          ,G = H

  ); } };

  dsml::Sm<MyMachine> sm{};

  sm.process_event(e1);

  EXPECT_TRUE(sm.is(H));
}

//--------------------------------------------------------------------------

TEST(Sm, DifferentEventsFromTheSameState)
{
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state + e1 = A
          , dsml::initial_state + e2 = B

  ); } };

  {
    dsml::Sm<MyMachine> sm{};

    sm.process_event(e1);

    EXPECT_TRUE(sm.is(A));
  }
  {
    dsml::Sm<MyMachine> sm{};

    sm.process_event(e2);

    EXPECT_TRUE(sm.is(B));
  }
}

//--------------------------------------------------------------------------

TEST(Sm, TransitionLoop)
{
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state + e1 = dsml::initial_state
          , dsml::initial_state + e2 = A

  ); } };

  dsml::Sm<MyMachine> sm{};

  sm.process_event(e1);
  EXPECT_TRUE(sm.is(dsml::initial_state));
  sm.process_event(e1);
  EXPECT_TRUE(sm.is(dsml::initial_state));
  sm.process_event(e2);
  EXPECT_TRUE(sm.is(A));
}

//--------------------------------------------------------------------------

TEST(Sm, TransitionAction)
{
  struct Data
  {
    bool called{false};
  };

  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state + e1
                    / [](Data& data){ data.called = true; }
                    = A

  ); } };

  Data data{};
  dsml::Sm<MyMachine, Data> sm{data};

  sm.process_event(e1);
  EXPECT_TRUE(sm.is(A));
  EXPECT_TRUE(data.called);
}

//--------------------------------------------------------------------------

TEST(Sm, TransitionMultiAction)
{
  using V = std::vector<int>;

  using namespace dsml::operators;
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state + e1
                    / (
                        [](V& v){ v.push_back(1); },
                        [](V& v){ v.push_back(2); },
                        [](V& v){ v.push_back(3); }
                      )
                    = A

  ); } };

  V calls{};
  dsml::Sm<MyMachine, V> sm{calls};

  sm.process_event(e1);
  EXPECT_TRUE(sm.is(A));
  EXPECT_EQ((std::vector<int>{{1, 2, 3}}), calls);
}

//--------------------------------------------------------------------------

TEST(Sm, AnonymousTransitionAction)
{
  struct Data
  {
    bool called{false};
  };

  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state
                    / [](Data& data){ data.called = true; }
                    = A

  ); } };

  Data data{};
  dsml::Sm<MyMachine, Data> sm{data};

  sm.process_event(e1);
  EXPECT_TRUE(sm.is(A));
  EXPECT_TRUE(data.called);
}

//--------------------------------------------------------------------------

TEST(Sm, TransitionActionDifferentRow)
{
  struct Data
  {
    bool called{false};
  };

  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state + e1
                    = A
          ,A + e2
                    / [](Data& data){ data.called = true; }
                    = B

  ); } };

  Data data{};
  dsml::Sm<MyMachine, Data> sm{data};

  sm.process_event(e1);
  EXPECT_TRUE(sm.is(A));
  EXPECT_FALSE(data.called);
  sm.process_event(e2);
  EXPECT_TRUE(sm.is(B));
  EXPECT_TRUE(data.called);
}

//--------------------------------------------------------------------------

TEST(Sm, TransitionGuard)
{
  struct Data
  {
    enum {F1, F2, F3} flag{F2};
  };

  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state + e1
                    [ ([](const Data& d){ return d.flag == Data::F1; }) ]
                    = A
          ,dsml::initial_state + e1
                    [ ([](const Data& d){ return d.flag == Data::F2; }) ]
                    = B
          ,dsml::initial_state + e1
                    [ ([](const Data& d){ return d.flag == Data::F3; }) ]
                    = C

  ); } };

  Data data{};
  dsml::Sm<MyMachine, Data> sm{data};

  sm.process_event(e1);
  EXPECT_TRUE(sm.is(B));
}

//--------------------------------------------------------------------------

TEST(Sm, TransitionGuardWithNotOperator)
{
  bool flag{true};

  using namespace dsml::operators;

  struct MyMachine { auto operator()() const noexcept {
          auto guard = [](const bool& flag_){ return flag_; };
          return dsml::make_transition_table(
                          dsml::initial_state + e1 [ ! guard ] = A
                      );
  } };

  dsml::Sm<MyMachine, bool> sm{flag};

  sm.process_event(e1);
  EXPECT_TRUE(sm.is(dsml::initial_state));
  flag = false;
  sm.process_event(e1);
  EXPECT_TRUE(sm.is(A));
}

//--------------------------------------------------------------------------

TEST(Sm, TransitionGuardWithAndOperator)
{
  struct Data
  {
    bool flag1{false};
    bool flag2{false};
  };

  using namespace dsml::operators;

  struct MyMachine { auto operator()() const noexcept {
          auto guard1 = [](Data& d){ return d.flag1; };
          auto guard2 = [](Data& d){ return d.flag2; };
          return dsml::make_transition_table(
                      dsml::initial_state + e1 [ guard1 && guard2 ] = A
                  );
  } };

  {
    Data data{};
    data.flag1 = false;
    data.flag2 = false;
    dsml::Sm<MyMachine, Data> sm{data};
    sm.process_event(e1);
    EXPECT_TRUE(sm.is(dsml::initial_state));
  }
  {
    Data data{};
    dsml::Sm<MyMachine, Data> sm{data};
    data.flag1 = true;
    data.flag2 = false;
    sm.process_event(e1);
    EXPECT_TRUE(sm.is(dsml::initial_state));
  }
  {
    Data data{};
    dsml::Sm<MyMachine, Data> sm{data};
    data.flag1 = false;
    data.flag2 = true;
    sm.process_event(e1);
    EXPECT_TRUE(sm.is(dsml::initial_state));
  }
  {
    Data data{};
    dsml::Sm<MyMachine, Data> sm{data};
    data.flag1 = true;
    data.flag2 = true;
    sm.process_event(e1);
    EXPECT_TRUE(sm.is(A));
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

  using namespace dsml::operators;

  struct MyMachine { auto operator()() const noexcept {
          auto guard1 = [](const Data& d){ return d.flag1; };
          auto guard2 = [](const Data& d){ return d.flag2; };
          return dsml::make_transition_table(
                      dsml::initial_state + e1 [ guard1 || guard2 ] = A
                  );
  } };

  {
    Data data{};
    data.flag1 = false;
    data.flag2 = false;
    dsml::Sm<MyMachine, Data> sm{data};
    sm.process_event(e1);
    EXPECT_TRUE(sm.is(dsml::initial_state));
  }
  {
    Data data{};
    dsml::Sm<MyMachine, Data> sm{data};
    data.flag1 = true;
    data.flag2 = false;
    sm.process_event(e1);
    EXPECT_TRUE(sm.is(A));
  }
  {
    Data data{};
    dsml::Sm<MyMachine, Data> sm{data};
    data.flag1 = false;
    data.flag2 = true;
    sm.process_event(e1);
    EXPECT_TRUE(sm.is(A));
  }
  {
    Data data{};
    dsml::Sm<MyMachine, Data> sm{data};
    data.flag1 = true;
    data.flag2 = true;
    sm.process_event(e1);
    EXPECT_TRUE(sm.is(A));
  }
}

//--------------------------------------------------------------------------

TEST(Sm, AnonymousTransitionGuard)
{
  struct Data
  {
    enum {F1, F2, F3} flag{F2};
  };

  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state
                    [ ([](const Data& d){ return d.flag == Data::F1; }) ]
                    = A
          ,dsml::initial_state
                    [ ([](const Data& d){ return d.flag == Data::F2; }) ]
                    = B
          ,dsml::initial_state
                    [ ([](const Data& d){ return d.flag == Data::F3; }) ]
                    = C

  ); } };

  Data data{};
  dsml::Sm<MyMachine, Data> sm{data};

  EXPECT_TRUE(sm.is(B));
}

//--------------------------------------------------------------------------

TEST(Sm, TransitionGuardAndAction)
{
  using V = std::vector<int>;

  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state = A
          , A + e1 [ false_guard ] / ([](V& v){v.push_back(1);}) = B
          , A + e1 [ true_guard ] / ([](V& v){v.push_back(2);}) = C

  ); } };

  V calls{};
  dsml::Sm<MyMachine, V> sm{calls};

  sm.process_event(e1);
  EXPECT_EQ(std::vector<int>{2}, calls);
  EXPECT_TRUE(sm.is(C));
}

//--------------------------------------------------------------------------

TEST(Sm, AnonymousTransitionGuardAndAction)
{
  using V = std::vector<int>;

  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state = A
          , A [ false_guard ] / ([](V& v){v.push_back(1);}) = B
          , A [ true_guard ] / ([](V& v){v.push_back(2);}) = C

  ); } };

  V calls{};
  dsml::Sm<MyMachine, V> sm{calls};

  sm.process_event(e1);
  EXPECT_EQ(std::vector<int>{2}, calls);
  EXPECT_TRUE(sm.is(C));
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

  struct MyMachine { auto operator()() const noexcept {

    using dsml::callee;
    using namespace dsml::operators;

    return dsml::make_transition_table(
          dsml::initial_state = A
          , A + e1
                [ callee(&Logic::guard) ]
                / callee(&Logic::action)
                = B
          , A + e1
                [ ! callee(&Logic::guard) ]
                = A
          , A + e1
                [ callee(&Logic::guard) && !callee(&Logic::guard) ]
                = A
  ); } };

  {
    Logic logic{};
    dsml::Sm<MyMachine, Logic> sm{logic};
    sm.process_event(e1);
    EXPECT_TRUE(sm.is(A));
  }
  {
    Logic logic{};
    dsml::Sm<MyMachine, Logic> sm{logic};
    logic.x = 5;
    sm.process_event(e1);
    EXPECT_TRUE(sm.is(B));
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

  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state = A
          , A
                    [ ([](const Data& d){return d.flag == Data::F1;}) ]
                    / ([](Data& d){
                          d.calls.push_back(1);
                          d.flag = Data::F2;
                        })
                    = B
          , A
                    [ ([](const Data& d){return d.flag == Data::F2;}) ]
                    / ([](Data& d){
                          d.calls.push_back(2);
                          d.flag = Data::F3;
                        }) = C
          , C
                    [ ([](const Data& d){return d.flag == Data::F2;}) ]
                    / ([](Data& d){
                          d.calls.push_back(3);
                        }) = D
          , C
                    [ ([](const Data& d){return d.flag == Data::F3;}) ]
                    / ([](Data& d){
                          d.calls.push_back(4);
                        }) = E

  ); } };

  Data d;
  dsml::Sm<MyMachine, Data> sm{d};

  EXPECT_EQ((std::vector<int>{{2, 4}}), d.calls);
  EXPECT_TRUE(sm.is(E));
}

//--------------------------------------------------------------------------

TEST(Sm, OnEntry)
{
  struct Data
  {
    bool called{false};
  };

  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state + e1 = A
          , A + dsml::on_entry / [](Data& data){ data.called = true; }

  ); } };

  Data data{};
  dsml::Sm<MyMachine, Data> sm{data};

  EXPECT_FALSE(data.called);
  sm.process_event(e1);
  EXPECT_TRUE(sm.is(A));
  EXPECT_TRUE(data.called);
}

//--------------------------------------------------------------------------

TEST(Sm, OnEntryOnExit)
{
  struct Data
  {
    std::vector<int> calls{};
  };

  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state + e1 = A
          , A + e2 = B

          , dsml::initial_state + dsml::on_entry / [](Data& data) { data.calls.push_back(1); }
          , dsml::initial_state + dsml::on_exit / [](Data& data) { data.calls.push_back(2); }
          , A + dsml::on_entry / [](Data& data){ data.calls.push_back(3); }
          , A + dsml::on_exit / [](Data& data){ data.calls.push_back(4); }
          , B + dsml::on_entry / [](Data& data) { data.calls.push_back(5); }
          , B + dsml::on_exit / [](Data& data) { data.calls.push_back(6); }

  ); } };

  Data data{};
  dsml::Sm<MyMachine, Data> sm{data};

  EXPECT_EQ((std::vector<int>{1}), data.calls);
  sm.process_event(e1);
  EXPECT_EQ((std::vector<int>{{1, 2, 3}}), data.calls);
  sm.process_event(e2);
  EXPECT_EQ((std::vector<int>{{1, 2, 3, 4, 5}}), data.calls);
}

//--------------------------------------------------------------------------

TEST(Sm, OnEntryOnExitLoopToSameState)
{
  struct Data
  {
    std::vector<int> calls{};
  };

  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state + e1 = dsml::initial_state
          , dsml::initial_state + e2 = A
          , A + e3 = A

          , dsml::initial_state + dsml::on_entry / [](Data& data) { data.calls.push_back(1); }
          , dsml::initial_state + dsml::on_exit / [](Data& data) { data.calls.push_back(2); }
          , A + dsml::on_entry / [](Data& data) { data.calls.push_back(3); }
          , A + dsml::on_exit / [](Data& data) { data.calls.push_back(4); }

  ); } };

  Data data{};
  dsml::Sm<MyMachine, Data> sm{data};

  EXPECT_EQ((std::vector<int>{1}), data.calls);
  sm.process_event(e1);
  EXPECT_EQ((std::vector<int>{{1, 2, 1}}), data.calls);
  sm.process_event(e2);
  EXPECT_EQ((std::vector<int>{{1, 2, 1, 2, 3}}), data.calls);
  sm.process_event(e3);
  EXPECT_EQ((std::vector<int>{{1, 2, 1, 2, 3, 4, 3}}), data.calls);
}

//--------------------------------------------------------------------------

TEST(Sm, OnEntryOnExitAndActionBetween)
{
  struct Data
  {
    std::vector<int> calls{};
  };

  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state + e1 / [](Data& data) { data.calls.push_back(0); } = A
          , A + dsml::on_entry / [](Data& data){ data.calls.push_back(2); }
          , dsml::initial_state + dsml::on_exit / [](Data& data) { data.calls.push_back(1); }

  ); } };

  Data data{};
  dsml::Sm<MyMachine, Data> sm{data};

  sm.process_event(e1);
  EXPECT_EQ((std::vector<int>{{1, 0, 2}}), data.calls);
}

//--------------------------------------------------------------------------

TEST(Sm, UnexpectedEventWithoutHandler_DoesNotChangeState)
{
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state + e1 = A
          , A + e2 = B

  ); } };

  dsml::Sm<MyMachine> sm{};

  sm.process_event(e4);
  EXPECT_TRUE(sm.is(dsml::initial_state));
  sm.process_event(e1);
  EXPECT_TRUE(sm.is(A));
  sm.process_event(e4);
  EXPECT_TRUE(sm.is(A));
}

//--------------------------------------------------------------------------

TEST(Sm, UnexpectedEventWithHandler_ChangesState)
{
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state + e1 = A
          , dsml::initial_state + dsml::unexpected_event = C
          , A + e2 = B
          , A + dsml::unexpected_event = D

  ); } };

  {
    dsml::Sm<MyMachine> sm{};
    sm.process_event(e1);
    EXPECT_TRUE(sm.is(A));
  }
  {
    dsml::Sm<MyMachine> sm{};
    sm.process_event(e4);
    EXPECT_TRUE(sm.is(C));
  }
  {
    dsml::Sm<MyMachine> sm{};
    sm.process_event(e1);
    sm.process_event(e2);
    EXPECT_TRUE(sm.is(B));
  }
  {
    dsml::Sm<MyMachine> sm{};
    sm.process_event(e1);
    sm.process_event(e4);
    EXPECT_TRUE(sm.is(D));
  }
}

//--------------------------------------------------------------------------

TEST(Sm, UnexpectedEventWithFalseGuard)
{
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state + e1 = A
          , dsml::initial_state + dsml::unexpected_event [ false_guard ] = B

  ); } };

  {
    dsml::Sm<MyMachine> sm{};
    sm.process_event(e1);
    EXPECT_TRUE(sm.is(A));
  }
  {
    dsml::Sm<MyMachine> sm{};
    sm.process_event(e2);
    EXPECT_TRUE(sm.is(dsml::initial_state));
  }
}

//--------------------------------------------------------------------------

TEST(Sm, UnexpectedEventWithTrueGuard)
{
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state + e1 = A
          , dsml::initial_state + dsml::unexpected_event [ true_guard ] = B

  ); } };

  {
    dsml::Sm<MyMachine> sm{};
    sm.process_event(e1);
    EXPECT_TRUE(sm.is(A));
  }
  {
    dsml::Sm<MyMachine> sm{};
    sm.process_event(e2);
    EXPECT_TRUE(sm.is(B));
  }
}

//--------------------------------------------------------------------------

TEST(Sm, AnyState)
{
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::any_state + e3 = C
          , dsml::any_state + e4 = D
          , dsml::initial_state + e1 = A
          , A + e2 = B

  ); } };

  {
    dsml::Sm<MyMachine> sm{};
    sm.process_event(e1);
    sm.process_event(e1);
    EXPECT_TRUE(sm.is(A));
    sm.process_event(e2);
    sm.process_event(e2);
    EXPECT_TRUE(sm.is(B));
    sm.process_event(e3);
    EXPECT_TRUE(sm.is(C));
  }
  {
    dsml::Sm<MyMachine> sm{};
    sm.process_event(e2);
    EXPECT_TRUE(sm.is(dsml::initial_state));
  }
  {
    dsml::Sm<MyMachine> sm{};
    sm.process_event(e3);
    EXPECT_TRUE(sm.is(C));
  }
  {
    dsml::Sm<MyMachine> sm{};
    sm.process_event(e4);
    EXPECT_TRUE(sm.is(D));
  }
  {
    dsml::Sm<MyMachine> sm{};
    sm.process_event(e3);
    sm.process_event(e3);
    EXPECT_TRUE(sm.is(C));
  }
}

//--------------------------------------------------------------------------

TEST(Sm, AnyStateWithTheSameEvent)
{
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::any_state + e1 = C // lower priority
          , dsml::initial_state + e1 = A

  ); } };

  dsml::Sm<MyMachine> sm{};
  sm.process_event(e1);
  EXPECT_TRUE(sm.is(A));
}

//--------------------------------------------------------------------------

TEST(Sm, AnyStateWithUnexpectedEvent)
{
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          A + dsml::unexpected_event = C
          , dsml::any_state + dsml::unexpected_event = E // lowest priority
          , dsml::any_state + e3 = D // lower priority
          , dsml::initial_state + e1 = A
          , A + e2 = B

  ); } };

  {
    dsml::Sm<MyMachine> sm{};
    sm.process_event(e2);
    EXPECT_TRUE(sm.is(E));
    sm.process_event(e2);
    EXPECT_TRUE(sm.is(E));
  }
  {
    dsml::Sm<MyMachine> sm{};
    sm.process_event(e3);
    EXPECT_TRUE(sm.is(D));
  }
  {
    dsml::Sm<MyMachine> sm{};
    sm.process_event(e1);
    EXPECT_TRUE(sm.is(A));
    sm.process_event(e3);
    EXPECT_TRUE(sm.is(C));
    sm.process_event(e3);
    EXPECT_TRUE(sm.is(D));
  }
  {
    dsml::Sm<MyMachine> sm{};
    sm.process_event(e1);
    EXPECT_TRUE(sm.is(A));
    sm.process_event(e2);
    EXPECT_TRUE(sm.is(B));
    sm.process_event(e2);
    EXPECT_TRUE(sm.is(E));
  }
}

//--------------------------------------------------------------------------

TEST(Sm, AnyStateWithFalseGuard)
{
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

        dsml::initial_state + e1 = A
        , dsml::any_state + e2 [ false_guard ] = B

  ); } };

  {
    dsml::Sm<MyMachine> sm{};
    sm.process_event(e1);
    EXPECT_TRUE(sm.is(A));
  }
  {
    dsml::Sm<MyMachine> sm{};
    sm.process_event(e2);
    EXPECT_TRUE(sm.is(dsml::initial_state));
  }
}

//--------------------------------------------------------------------------

TEST(Sm, AnyStateWithTrueGuard)
{
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

        dsml::initial_state + e1 = A
        , dsml::any_state + e2 [ true_guard ] = B

  ); } };

  {
    dsml::Sm<MyMachine> sm{};
    sm.process_event(e1);
    EXPECT_TRUE(sm.is(A));
  }
  {
    dsml::Sm<MyMachine> sm{};
    sm.process_event(e2);
    EXPECT_TRUE(sm.is(B));
  }
}

//--------------------------------------------------------------------------

TEST(Sm, Reset)
{
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          A + e2 = B
          , dsml::initial_state + e1 = A

  ); } };

  dsml::Sm<MyMachine> sm{};
  sm.process_event(e1);
  sm.reset();
  EXPECT_TRUE(sm.is(dsml::initial_state));
}

//==========================================================================

TEST(SmComposite, AnonymousTransitions_IsInTheSubStateInitialState)
{
  struct Sub { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state = SubA

  ); } };
  struct Composite { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state = dsml::State<Sub>{}

  ); } };
  dsml::Sm<Composite> sm{};

  EXPECT_FALSE(sm.is(dsml::initial_state));
  EXPECT_TRUE(sm.is_sub(SubA, Sub{}));
}

//--------------------------------------------------------------------------

TEST(SmComposite, TransitionInComposite_IsInTheCompositeStateInitialState)
{
  struct Sub { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state = SubA

  ); } };
  struct Composite { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state + e1 = dsml::State<Sub>{}

  ); } };
  dsml::Sm<Composite> sm{};

  EXPECT_TRUE(sm.is(dsml::initial_state));
  EXPECT_FALSE(sm.is_sub(SubA, Sub{}));
}

//--------------------------------------------------------------------------

TEST(SmComposite, TransitionInComposite_ProcessEvent_IsInTheSubStateA)
{
  struct Sub { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state = SubA

  ); } };
  struct Composite { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state + e1 = dsml::State<Sub>{}

  ); } };
  dsml::Sm<Composite> sm{};

  sm.process_event(e1);

  EXPECT_FALSE(sm.is(dsml::initial_state));
  EXPECT_TRUE(sm.is_sub(SubA, Sub{}));
}

//--------------------------------------------------------------------------

TEST(SmMultilevelComposite, AnonymousTransitions_IsInTheSubStateInitialState)
{
  struct SubSub { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state = SubA

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
  EXPECT_TRUE(sm.is_sub(SubA, Sub{}, SubSub{}));
}

//--------------------------------------------------------------------------

TEST(SmMultilevelComposite, AnonymousTransitions_InAndOut)
{
  struct SubSub { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state = SubA
          , SubA = dsml::final_state

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
  EXPECT_FALSE(sm.is_sub(SubA, Sub{}, SubSub{}));
  EXPECT_FALSE(sm.is_sub(dsml::final_state, Sub{}, SubSub{}));
}

//--------------------------------------------------------------------------

TEST(SmComposite, OnEntryOnExit)
{
  struct Data
  {
    std::vector<int> calls{};
  };

  struct Sub { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state = SubA
          , SubA = dsml::final_state
          , SubA + dsml::on_entry / [](Data& data) { data.calls.push_back(1); }
          , SubA + dsml::on_exit / [](Data& data) { data.calls.push_back(2); }

  ); } };
  struct Composite { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state = A
          , A = dsml::State<Sub>{}
          , dsml::State<Sub>{} = B
          , A + dsml::on_entry / [](Data& data) { data.calls.push_back(3); }
          , A + dsml::on_exit / [](Data& data) { data.calls.push_back(4); }
          , B + dsml::on_entry / [](Data& data) { data.calls.push_back(5); }

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

  struct SubSub { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state + e1
              / [](Data& d){ d.calls.push_back(1); }
              = SubA
          , SubA = dsml::final_state

  ); } };
  struct Sub { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state = dsml::State<SubSub>{}
          , dsml::State<SubSub>{} = A
          , A [ false_guard ] / ([](Data& d){d.calls.push_back(2);}) = B
          , A [ true_guard ] / ([](Data& d){d.calls.push_back(3);}) = C
          , B = dsml::final_state
          , C = dsml::final_state

  ); } };
  struct MyMachine { auto operator()() const noexcept { return dsml::make_transition_table(

          dsml::initial_state = dsml::State<Sub>{}

  ); } };

  Data data{};
  dsml::Sm<MyMachine, Data> sm{data};

  sm.process_event(e1);
  EXPECT_EQ((std::vector<int>{{1, 3}}), data.calls);
}

//--------------------------------------------------------------------------

#ifndef _MSC_VER
TEST(SmMultilevelCompositeWithUDL, Mixture)
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
#endif

//==========================================================================

struct MyObserver : dsml::Observer
{
  std::vector<std::string> log{};

  template <typename TEvent>
  void event()
  {
    log.push_back(TEvent::c_str());
  }

  template <typename TGuard>
  void guard(const TGuard&, const bool result)
  {
    log.push_back("guard " + std::to_string(result));
  }

  template <typename TAction>
  void action(const TAction&)
  {
    log.push_back("action");
  }

  template <typename TSrcState, typename TDstState>
  void state_change()
  {
    log.push_back(std::string{TSrcState::c_str()} +
                  "->" +
                  std::string{TDstState::c_str()});
  }
};

//--------------------------------------------------------------------------

TEST(Sm, Observer)
{
  using namespace dsml::literals;
  struct MyMachine { auto operator()() const noexcept {
    return dsml::make_transition_table(

          dsml::initial_state = A
        , A + e1 [ false_guard ] / no_action = B
        , A + e1 [ true_guard ] / no_action = C

  ); } };

  MyObserver observer{};
  dsml::Sm<MyMachine, MyObserver> sm{observer};

  sm.process_event(e1);
#ifdef _MSC_VER
  EXPECT_EQ((std::vector<std::string>{
            "anonymous",
            "initial->struct A_",
            "anonymous",
            "struct e1_",
            "guard 0",
            "guard 1",
            "action",
            "struct A_->struct C_",
            "anonymous"
          }), observer.log);
#else
  EXPECT_EQ((std::vector<std::string>{
            "anonymous",
            "initial->A_",
            "anonymous",
            "e1_",
            "guard 0",
            "guard 1",
            "action",
            "A_->C_",
            "anonymous"
          }), observer.log);
#endif
}

//--------------------------------------------------------------------------

TEST(Sm, ObserverAndDependency)
{
  struct Logic
  {
    bool guard() const
    {
      return x == 0;
    }

    void action()
    {
      x = 10;
    }

    int x{0};
  };

  using namespace dsml::literals;
  struct MyMachine { auto operator()() const noexcept {
    return dsml::make_transition_table(

          dsml::initial_state = A
        , A + e1
              [ dsml::callee(&Logic::guard) ]
              / dsml::callee(&Logic::action)
              = B

  ); } };

  MyObserver observer{};
  Logic logic{};
  dsml::Sm<MyMachine, MyObserver, Logic> sm{observer, logic};

  sm.process_event(e1);
#ifdef _MSC_VER
  EXPECT_EQ((std::vector<std::string>{
            "anonymous",
            "initial->struct A_",
            "anonymous",
            "struct e1_",
            "guard 1",
            "action",
            "struct A_->struct B_",
            "anonymous"
          }), observer.log);
#else
  EXPECT_EQ((std::vector<std::string>{
            "anonymous",
            "initial->A_",
            "anonymous",
            "e1_",
            "guard 1",
            "action",
            "A_->B_",
            "anonymous"
          }), observer.log);
#endif
  EXPECT_EQ(10, logic.x);
}

//==========================================================================

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
