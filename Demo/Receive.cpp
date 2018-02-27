#include <cstdint>
#include <chrono>
#include <iostream>
#include <sstream>

#include "cluon/OD4Session.hpp"
#include "cluon/Envelope.hpp"
#include "messages.hpp"
#include "EvenChecker.hpp"

int main(int /*argc*/, char** /*argv*/) {
  std::cout << "Waiting to recieve numbers..." << '\n';
    cluon::OD4Session od4(111,
        [](cluon::data::Envelope &&envelope) noexcept {
        if (envelope.dataType() == 2001) {
          MyTestMessage1 receivedMsg = cluon::extractMessage<MyTestMessage1>(std::move(envelope));
          EvenChecker ec;
          if(ec.even(receivedMsg.myValue())){
              std::cout << receivedMsg.myValue() << " is even!" << std::endl;
          }else{
            std::cout << receivedMsg.myValue() << " is odd!" << std::endl;
          }
        }
    });
    using namespace std::literals::chrono_literals;
      while (od4.isRunning()) {
          std::this_thread::sleep_for(1s);
      }
  return 0;
}
