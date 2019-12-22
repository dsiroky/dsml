#pragma once
#ifndef LOGIC_HPP__CMOZAIL4
#define LOGIC_HPP__CMOZAIL4

#include "dsml.hpp"

//==========================================================================

struct IWashingMachineObserver
{
  virtual ~IWashingMachineObserver() = default;

  virtual void display_state(const std::string&) = 0;
};

//==========================================================================

class WashingMachine
{
public:
  enum class Program { COTTON, WOOL, RINSE_ONLY };

  //--------------------------------

  WashingMachine(IWashingMachineObserver& observer);

  WashingMachine(const WashingMachine&) = delete;
  WashingMachine& operator=(const WashingMachine&) = delete;

  void power_on();
  void set_program(const Program program);
  void start();
  void stop();

  //--------------------------------
private:
  struct StateMachineDecl;
  friend struct StateMachineDecl;

  //--------------------------------

  void set_washing_program(const Program& program);
  void set_wash_timeout();
  void set_rinse_timeout();

  //--------------------------------

  Program m_program{Program::COTTON};
  /// This member can't be initialized here because `StateMachineDecl` is
  /// defined in the cpp file.
  dsml::Sm<StateMachineDecl, WashingMachine, IWashingMachineObserver> m_sm;
};

#endif /* include guard */
