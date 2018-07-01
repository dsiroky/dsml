[![Build Status](https://travis-ci.org/dsiroky/dsml.svg?branch=master)](https://travis-ci.org/dsiroky/dsml)

# dsml - C++ state machine library

Heavily inspired by ["boost"-sml](https://github.com/boost-experimental/sml). Motivation to create another SM library was to have well implemented anonymous transitions and more straightforward approach to composite state machines. *dsml* does not aim to be fully UML compliant.

## Main goals

* Header only.
* Runtime speed - don't pay for what you don't need.
* Clear semantics.
* Well tested.

## Dependencies

- C++14 syntax
- few headers from STL

## Examples

### Hello world

```cpp
#include <dsml.hpp>

using namespace dsml::literals;

struct MyMachine
{
  auto operator()() const noexcept
  {
    return dsml::make_transition_table(
        dsml::initial_state + "evt1"_e = "A"_s
        , "A"_s + "evt2"_e = "B"_s
        , "A"_s + "evt3"_e = "C"_s
        , "B"_s + "evt4"_e = "C"_s
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

```cpp
// anonymous
"A"_s = "B"_s
```

```cpp
// with event
"A"_s + "evt1"_e = "B"_s
```

### Guards

```cpp
auto guard = [](){ return true; };
```
```cpp
// anonymous transition
"A"_s [ guard ] = "B"_s
```
```cpp
// with event
"A"_s + "evt1"_e [ guard ] = "B"_s
```

### Actions

```cpp
auto action = [](){ do_something(); };
```
```cpp
// anonymous transition
"A"_s / action = "B"_s
```
```cpp
// with event
"A"_s + "evt1"_e / action = "B"_s
```

### State entry/exit actions

```cpp
auto action = [](){ do_something(); };
```
```cpp
"A"_s + dsml::on_entry / action,
"A"_s + dsml::on_exit / action
```

### Guards and actions together

```cpp
auto guard = [](){ return true; };
auto action = [](){ do_something(); };
```
```cpp
// anonymous transition
"A"_s [ guard ] / action = "B"_s
```
```cpp
// with event
"A"_s + "evt1"_e [ guard ] / action = "B"_s
```

### Dependencies

```cpp
using namespace dsml::literals;

struct Data
{
  int x{};
};

auto guard = [](Data& data){ return data.x <= 5; };
auto inv_guard = [](Data& data){ return data.x > 5; };
auto action = [](Data& data){ data.x += 2; };

struct MyMachine
{
  auto operator()() const noexcept
  {
    return dsml::make_transition_table(
        dsml::initial_state + "evt1"_e [ guard ] / action = "A"_s
        , dsml::initial_state + "evt1"_e [ inv_guard ] = "B"_s
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

```cpp
struct Sub
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

struct Composite
{
  auto operator()() const noexcept
  {
    return dsml::make_transition_table(
        dsml::initial_state + "evt1"_e = dsml::State<Sub>{}
        , dsml::State<Sub>{} + "evt2"_e = "A"_s
      );
  }
};
```

## TODO
- operators for guards
- more static asserts (especially for actions and guards)
- logging
- move private stuff to detail
- compilation time optimizations
- comments
- automatic dependencies like boost::sml
- documentation, examples
