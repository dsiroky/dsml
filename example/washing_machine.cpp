#include <iostream>

#include "washing_machine.hpp"

//==========================================================================

#define STATE(x) static constexpr dsml::State<struct x##_> x{}
STATE(S_SETUP);
STATE(S_IDLE);
STATE(S_ERROR);
STATE(S_WASH);
STATE(S_RINSE);
#undef STATE

#define EVENT(x) static constexpr dsml::Event<struct x##_> x{}
EVENT(E_INIT);
EVENT(E_START);
EVENT(E_STOP);
EVENT(E_TIMEOUT);
#undef EVENT
// this event can carry a value
static constexpr auto E_SET_PROGRAM = dsml::Event<WashingMachine::Program>{};

//--------------------------------------------------------------------------

struct WashingMachine::StateMachineDecl
{
  auto operator()() const noexcept
  {
    using namespace dsml::literals;
    using namespace dsml::operators;

    //--------------------------------

    const auto display = [](const std::string& msg) {
      return [msg](IWashingMachineObserver& observer) {
        observer.display_state(msg);
      };
    };
    const auto set_washing_program_gen = [](const Program program) {
      return [program](WashingMachine& wm) { wm.set_washing_program(program); };
    };
    const auto set_washing_program
        = dsml::callee(&WashingMachine::set_washing_program);
    const auto rinse_only =
        [](const WashingMachine& wm) {
          return wm.m_program == Program::RINSE_ONLY;
        };
    const auto set_wash_timeout = dsml::callee(&WashingMachine::set_wash_timeout);
    const auto set_rinse_timeout = dsml::callee(&WashingMachine::set_rinse_timeout);

    //--------------------------------

    return dsml::make_transition_table(
        dsml::initial_state + E_INIT = S_SETUP

        , S_SETUP
            + dsml::on_entry
                  / (display("idle"), set_washing_program_gen(Program::COTTON))
        , S_SETUP = S_IDLE

        , S_IDLE + E_SET_PROGRAM / set_washing_program = S_IDLE
        , S_IDLE + E_START[! rinse_only] = S_WASH
        , S_IDLE + E_START[rinse_only] = S_RINSE

        , S_WASH
            + dsml::on_entry
                  / (display("wash"), set_wash_timeout)
        , S_WASH + E_TIMEOUT = S_RINSE
        , S_WASH + E_STOP = S_SETUP

        , S_RINSE
            + dsml::on_entry
                  / (display("rinse"), set_rinse_timeout)
        , S_RINSE + E_TIMEOUT = S_SETUP
        , S_RINSE + E_STOP = S_SETUP
    );
  }
};

//==========================================================================

WashingMachine::WashingMachine(IWashingMachineObserver& observer)
  : m_sm{*this, observer}
{
}

//--------------------------------------------------------------------------

void WashingMachine::power_on()
{
  m_sm.process_event(E_INIT);
}

//--------------------------------------------------------------------------

void WashingMachine::set_program(const Program program)
{
  // process an event with a value
  m_sm.process_event(E_SET_PROGRAM(program));
}

//--------------------------------------------------------------------------

void WashingMachine::set_washing_program(const Program& program)
{
  std::cout << "setting program to " << static_cast<int>(program) << '\n';
}

//--------------------------------------------------------------------------

void WashingMachine::start()
{
  m_sm.process_event(E_START);
}

//--------------------------------------------------------------------------

void WashingMachine::stop()
{
  m_sm.process_event(E_STOP);
}

//--------------------------------------------------------------------------

void WashingMachine::set_wash_timeout()
{
  // TODO implement E_TIMEOUT generator
}

//--------------------------------------------------------------------------

void WashingMachine::set_rinse_timeout()
{
  // TODO implement E_TIMEOUT generator
}
