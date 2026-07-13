#include <doctest/doctest.h>
#include <vagabond/utils/OpSet.h>
#include <vector>

TEST_CASE("smoke: basic arithmetic") {
  CHECK(2+2==4);
}
TEST_CASE("smoke: basic arithmetic but right") {
  CHECK(1+1==2);
}
TEST_CASE("Even numbers filtered") {
  std::vector<int> start = {-1, 2,3,5,6,8};
  OpSet<int> numbers(start);
  numbers.filter([](const int &i) {return (i%2!=0);});
  int check =  *numbers.begin();
  CHECK(check==-1);
}
