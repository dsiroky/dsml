#include <iostream>

#include "washing_machine.hpp"

class Observer: public IWashingMachineObserver
{
public:
  void display_state(const std::string& msg) override
  {
    std::cout << "observer: " << msg << '\n';
  };

private:
};

int main()
{
  Observer observer{};
  WashingMachine washmach{observer};
  washmach.power_on();
  washmach.set_program(WashingMachine::Program::WOOL);
  washmach.start();
  return 0;
}
