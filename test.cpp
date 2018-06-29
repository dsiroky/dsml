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
  EXPECT_EQ(std::vector<int>{{2}}, calls);
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
  EXPECT_EQ(std::vector<int>{{2}}, calls);
  EXPECT_TRUE(sm.is("C"_s));
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
