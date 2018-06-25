#include "gtest/gtest.h"

#include "dsml.hpp"

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

TEST(FindCompatible, Single_UserByValue)
{
  {
    using t = dsml::detail::FindCompatible_t<int, int&>;
    EXPECT_TRUE((std::is_same<t, int&>::value));
  }
  {
    using t = dsml::detail::FindCompatible_t<int, const int&>;
    EXPECT_TRUE((std::is_same<t, const int&>::value));
  }
  {
    using t = dsml::detail::FindCompatible_t<int, int&&>;
    EXPECT_TRUE((std::is_same<t, int&&>::value));
  }
}

TEST(FindCompatible, Single_UserByReference)
{
  {
    using t = dsml::detail::FindCompatible_t<int&, int&>;
    EXPECT_TRUE((std::is_same<t, int&>::value));
  }
  {
    using t = dsml::detail::FindCompatible_t<int&, const int&>;
    EXPECT_TRUE((std::is_same<t, void>::value));
  }
  {
    using t = dsml::detail::FindCompatible_t<int&, int&&>;
    EXPECT_TRUE((std::is_same<t, int&&>::value));
  }
}

TEST(FindCompatible, Single_UserByConstReference)
{
  {
    using t = dsml::detail::FindCompatible_t<const int&, int&>;
    EXPECT_TRUE((std::is_same<t, int&>::value));
  }
  {
    using t = dsml::detail::FindCompatible_t<const int&, const int&>;
    EXPECT_TRUE((std::is_same<t, const int&>::value));
  }
  {
    using t = dsml::detail::FindCompatible_t<const int&, int&&>;
    EXPECT_TRUE((std::is_same<t, int&&>::value));
  }
}

TEST(FindCompatible, Misc)
{
  {
    using t = dsml::detail::FindCompatible_t<const int&, float&, int&&, const char&>;
    EXPECT_TRUE((std::is_same<t, int&&>::value));
  }
}

//==========================================================================

TEST(MakeStorage, EmptyBaseType)
{
  using user_t = std::tuple<>;

  {
    auto storage = dsml::detail::make_storage<user_t>();
    EXPECT_TRUE((std::is_same<user_t, decltype(storage)>::value));
  }
  {
    auto storage = dsml::detail::make_storage<user_t>(3);
    EXPECT_TRUE((std::is_same<user_t, decltype(storage)>::value));
  }
  {
    auto storage = dsml::detail::make_storage<user_t>(3, false);
    EXPECT_TRUE((std::is_same<user_t, decltype(storage)>::value));
  }
}

//--------------------------------------------------------------------------

TEST(MakeStorage, SingleType)
{
  using user_t = std::tuple<int>;

  {
    //auto storage = dsml::detail::make_storage<user_t>(3);
    //EXPECT_TRUE((std::is_same<std::tuple<int>, decltype(storage)>::value));
    //EXPECT_EQ(3, std::get<0>(tup));
  }
//  {
//    auto storage = dsml::detail::make_storage<user_t>(3, false);
//    EXPECT_TRUE((std::is_same<user_t, decltype(storage)>::value));
//    EXPECT_EQ(3, std::get<0>(tup));
//  }
//  {
//    auto storage = dsml::detail::make_storage<user_t>(6.2, 3, false);
//    EXPECT_TRUE((std::is_same<user_t, decltype(storage)>::value));
//    EXPECT_EQ(3, std::get<0>(tup));
//  }
//  {
//    const int i{8};
//    const int& ref{i};
//    auto storage = dsml::detail::make_storage<user_t>(6.2, ref, false);
//    EXPECT_TRUE((std::is_same<user_t, decltype(storage)>::value));
//    EXPECT_EQ(8, std::get<0>(tup));
//  }
}

//--------------------------------------------------------------------------

TEST(MakeStorage, SingleTypeReference)
{
  using user_t = std::tuple<int&>;

//  {
//    int i{34};
//    auto storage = dsml::detail::make_storage<user_t>(i);
//    EXPECT_TRUE((std::is_same<user_t, decltype(storage)>::value));
//    EXPECT_EQ(34, std::get<0>(tup));
//    std::get<0>() = 4;
//    EXPECT_EQ(4, i);
//  }
//  {
//    int i{34};
//    auto storage = dsml::detail::make_storage<user_t>(i, false);
//    EXPECT_TRUE((std::is_same<user_t, decltype(storage)>::value));
//    EXPECT_EQ(34, std::get<0>(tup));
//    std::get<0>() = 4;
//    EXPECT_EQ(4, i);
//  }
//  {
//    int i{34};
//    auto storage = dsml::detail::make_storage<user_t>(6.2, i, false);
//    EXPECT_TRUE((std::is_same<user_t, decltype(storage)>::value));
//    EXPECT_EQ(34, std::get<0>(tup));
//    std::get<0>() = 4;
//    EXPECT_EQ(4, i);
//  }
//  {
//    int i{8};
//    int& ref{i};
//    auto storage = dsml::detail::make_storage<user_t>(6.2, ref, false);
//    EXPECT_TRUE((std::is_same<user_t, decltype(storage)>::value));
//    EXPECT_EQ(8, std::get<0>(tup));
//    std::get<0>() = 4;
//    EXPECT_EQ(4, i);
//  }
}

//--------------------------------------------------------------------------

TEST(MakeStorage, SingleTypeConstReference)
{
  
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

TEST(Sm_ProcessEvent, SingleTransition)
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

TEST(Sm_ProcessEvent, SingleTransitionUnknownEvent_NoStateChange)
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

TEST(Sm_ProcessEvent, MultipleTransitionsSameEvents)
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

TEST(Sm_ProcessEvent, MultipleTransitionsDifferentEvents)
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

TEST(Sm_ProcessEvent, AnonymousEventAfterNormalEvent)
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

TEST(Sm_ProcessEvent, AnonymousEventBeforeNormalEvent)
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

TEST(Sm_ProcessEvent, MultipleAnonymousEvents)
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

TEST(Sm_ProcessEvent, MultipleAnonymousEventsAroundNormalEvent)
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

TEST(Sm_ProcessEvent, DifferentEventsFromTheSameState)
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

TEST(Sm_ProcessEvent, TransitionLoop)
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

TEST(Sm_ProcessEvent, TransitionAction)
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
  dsml::Sm<MyMachine> sm{};

  sm.process_event("e1"_e);
  EXPECT_TRUE(sm.is<decltype("A"_s)>());
  EXPECT_TRUE(data.called);
}

//==========================================================================

auto g = [](int){return true;};

int main(int argc, char *argv[])
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
  dsml::Sm<MyMachine> sm{};

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
