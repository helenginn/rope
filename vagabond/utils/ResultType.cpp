// vagabond
// Copyright (C) 2022 Helen Ginn
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Please email: vagabond @ hginn.co.uk for more details.

#include "ResultType.h"

#ifdef ROPE_INLINE_TESTS
#include <doctest/doctest.h>

// Error type with toString
struct CustomTestError {
  std::string msg;
  std::string toString() const { return msg; }
};

// Error type without toString
struct PlainTestError {};

TEST_CASE("Result - Basic Ok operations") {
  Result<int, std::string> res = Ok(42);

  REQUIRE(res.is_ok());
  CHECK(!res.is_err());
  CHECK(res.unwrap() == 42);

  CHECK(std::move(res).unwrap_or(0) == 42);
  CHECK(std::move(res).unwrap_or_default() == 42);
}

TEST_CASE("Result - Basic Err operations") {
  Result<int, std::string> res = Err(std::string("something went wrong"));

  REQUIRE(res.is_err());
  CHECK(!res.is_ok());
  CHECK(std::move(res).unwrap_or(100) == 100);
  CHECK(res.unwrap_err() == "something went wrong");
}

TEST_CASE("Result - Throws on invalid access") {
  Result<int, std::string> err_res = Err(std::string("Error"));
  Result<int, std::string> ok_res = Ok(10);

  CHECK_THROWS_AS(err_res.unwrap(), std::runtime_error);
  CHECK_THROWS_AS(ok_res.unwrap_err(), std::runtime_error);
}

TEST_CASE("Result - Move-only types") {
  auto ptr = std::make_unique<int>(1337);
  Result<std::unique_ptr<int>, std::string> res = Ok(std::move(ptr));

  auto val = std::move(res).unwrap();
  CHECK(*val == 1337);
}

TEST_CASE("Result - Concept expect() checks") {
  SUBCASE("With .toString()") {
    Result<int, CustomTestError> res = Err(CustomTestError{"Error details"});
    CHECK_THROWS_AS(std::move(res).expect("Error"), std::runtime_error);
  }

  SUBCASE("Without .toString()") {
    Result<int, PlainTestError> res = Err(PlainTestError{});
    CHECK_THROWS_AS(std::move(res).expect("Error"), std::runtime_error);
  }
}

TEST_CASE("Result - Monadic map operations") {
  SUBCASE("map on Ok") {
    Result<int, std::string> res = Ok(5);
    auto mapped = std::move(res).map([](int x) { return x * 2; });
    CHECK(mapped.unwrap() == 10);
  }

  SUBCASE("map_err on Err") {
    Result<int, std::string> res = Err(std::string("bad"));
    auto mapped = std::move(res).map_err([](std::string s) { return s + "!"; });
    CHECK(mapped.unwrap_err() == "bad!");
  }
}
#endif
