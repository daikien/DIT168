#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "EvenChecker.hpp"

TEST_CASE("Not even") {
    EvenChecker ec;
    bool f = false;
    REQUIRE(ec.even(5)==f);
}
TEST_CASE("Even") {
    EvenChecker ec;
    REQUIRE(ec.even(2));
}
