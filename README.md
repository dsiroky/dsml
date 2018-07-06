[![Build Status](https://travis-ci.org/dsiroky/dsml.svg?branch=master)](https://travis-ci.org/dsiroky/dsml)

# dsml - C++ state machine library

Heavily inspired by ["boost"-sml](https://github.com/boost-experimental/sml). Motivation to create another SM library was to have well implemented anonymous transitions and more straightforward approach to composite state machines. *dsml* aims to be almost UML compliant.

## Main goals

* Header only.
* Minimal resulting binary footprint - don't pay for what you don't need.
* Speed.
* Clear semantics.
* Well tested.

## Dependencies

- C++14 syntax
- few headers from STL

## Examples

### Hello world

![diagram](diagrams/hello_world.png)

```cpp
#include <dsml.hpp>

using namespace dsml::literals;

const auto guard = [](){ return true; };
const auto action = [](){ do_something(); };

struct MyMachine
{
  auto operator()() const noexcept
  {
    return dsml::make_transition_table(
          dsml::initial_state + "evt1"_e = "A"_s
        , "A"_s + "evt2"_e = "B"_s
        , "A"_s + "evt3"_e [ guard ] = "C"_s
        , "B"_s + "evt4"_e / action = "C"_s
        , "B"_s + "evt1"_e = "B"_s
        , "C"_s = "D"_s
      );
  }
};

int main()
{
  dsml::Sm<MyMachine> sm{};
  sm.process_event("evt1"_e);
  return sm.is("A"_s);
}
```

### Transitions

![diagram](diagrams/transition_anonymous.png)
```cpp
// anonymous
"A"_s = "B"_s
```

![diagram](diagrams/transition_with_event.png)
```cpp
// with event
"A"_s + "evt1"_e = "B"_s
```

### Guards

```cpp
const auto guard = [](){ return true; };
```
![diagram](diagrams/guard_anonymous.png)
```cpp
// anonymous transition
"A"_s [ guard ] = "B"_s
```
![diagram](diagrams/guard_event.png)
```cpp
// with event
"A"_s + "evt1"_e [ guard ] = "B"_s
```

#### Combined
You can combine guards into a logical expression:
```cpp
// add this into your scope to enable guard logical expressions
using dsml::guard_operators;
```
![diagram](diagrams/guard_combined.png)
```cpp
// with event
"A"_s + "evt1"_e [ guard1 && guard2 || !guard3 ] = "B"_s
```

### Actions

```cpp
const auto action = [](){ do_something(); };
```
![diagram](diagrams/action_anonymous.png)
```cpp
// anonymous transition
"A"_s / action = "B"_s
```
![diagram](diagrams/action_event.png)
```cpp
// with event
"A"_s + "evt1"_e / action = "B"_s
```

### State entry/exit actions

```cpp
const auto action = [](){ do_something(); };
```
![diagram](diagrams/state_entry_exit.png)
```cpp
"A"_s + dsml::on_entry / action,
"A"_s + dsml::on_exit / action
```

### Guards and actions together

```cpp
const auto guard = [](){ return true; };
const auto action = [](){ do_something(); };
```
![diagram](diagrams/guard_action_anonymous.png)
```cpp
// anonymous transition
"A"_s [ guard ] / action = "B"_s
```
![diagram](diagrams/guard_action_event.png)
```cpp
// with event
"A"_s + "evt1"_e [ guard ] / action = "B"_s
```

### Dependencies

Useful to connect the SM with non-global logic.

```cpp
using namespace dsml::literals;
using namespace dsml::guard_operators;

struct Data
{
  int x{};
};

const auto guard = [](const Data& data){ return data.x <= 5; };
const auto action = [](Data& data){ data.x += 2; };

struct MyMachine
{
  auto operator()() const noexcept
  {
    return dsml::make_transition_table(
          dsml::initial_state + "evt1"_e [ guard ] / action = "A"_s
        , dsml::initial_state + "evt1"_e [ ! guard ] = "B"_s
      );
  }
};

void func()
{
  Data data{};
  dsml::Sm<MyMachine, Data> sm{data};
  sm.process_event("evt1"_e);
}
```

### Composite state machines

![diagram](diagrams/composite.png)
```cpp
struct OtherMachine
{
  auto operator()() const noexcept
  {
    return dsml::make_transition_table(
          dsml::initial_state + "evt1"_e = "A"_s
        , dsml::initial_state + "evt2"_e = "B"_s
        , "A"_s + "evt1"_e = dsml::final_state
        , "B"_s + "evt1"_e = dsml::final_state
      );
  }
};

struct CompositeMachine
{
  auto operator()() const noexcept
  {
    return dsml::make_transition_table(
          dsml::initial_state + "evt1"_e = dsml::State<OtherMachine>{}
        , dsml::State<OtherMachine>{} + "evt2"_e = "A"_s
      );
  }
};
```

## TODO
- `unexpected_event`
- more static asserts (especially for actions and guards)
- logging
- move private stuff to detail
- compilation time optimizations
- comments
- automatic dependencies like boost::sml
- documentation, examples
