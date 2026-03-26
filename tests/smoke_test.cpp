#include <doctest/doctest.h>

TEST_CASE("smoke: basic arithmetic but wrong") {
  CHECK(2+2==5);
}
TEST_CASE("smoke: basic arithmetic but right") {
  CHECK(1+1==2);
}
