#include <cstdint>
#include <chrono>
#include <iostream>
#include <sstream>

#include "cluon/OD4Session.hpp"
#include "cluon/Envelope.hpp"
#include "messages.hpp"

int main(int /*argc*/, char** /*argv*/) {

  cluon::OD4Session od4(111,
      [](cluon::data::Envelope &&envelope) noexcept {});
      using namespace std::literals::chrono_literals;
        while (od4.isRunning()) {
          uint16_t value;
          std::cout << "Enter a number to send: ";
          std::cin >> value;
          MyTestMessage1 msg;
          msg.myValue(value);
          od4.send(msg);
        }
    return 0;
}
