#include "EvenChecker.hpp"

bool EvenChecker::even(uint16_t n) {
    bool retVal{true};
    if (n%2==0){retVal = true;}
    else{retVal = false;}
    return retVal;
}
