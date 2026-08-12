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
#include <memory>

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

TEST_CASE("Result - expect_err() concept checks") {
  SUBCASE("With .toString() on T") {
    Result<CustomTestError, int> res = Ok(CustomTestError{"unexpected value"});
    CHECK_THROWS_AS(std::move(res).expect_err("Expected error"),
                    std::runtime_error);
  }
  SUBCASE("Without .toString() on T") {
    Result<PlainTestError, int> res = Ok(PlainTestError{});
    CHECK_THROWS_AS(std::move(res).expect_err("Expected error"),
                    std::runtime_error);
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

TEST_CASE("Result - map/map_err pass-through") {
  SUBCASE("map on Err leaves error untouched") {
    Result<int, std::string> res = Err(std::string("bad"));
    auto mapped = std::move(res).map([](int x) { return x * 2; });
    CHECK(mapped.is_err());
    CHECK(mapped.unwrap_err() == "bad");
  }
  SUBCASE("map_err on Ok leaves value untouched") {
    Result<int, std::string> res = Ok(5);
    auto mapped = std::move(res).map_err([](std::string s) { return s + "!"; });
    CHECK(mapped.is_ok());
    CHECK(mapped.unwrap() == 5);
  }
}
TEST_CASE("Result<void,E> - Basic Ok operations") {
  Result<void, std::string> res = Ok();
  REQUIRE(res.is_ok());
  CHECK(!res.is_err());
  CHECK_NOTHROW(res.unwrap());
}

TEST_CASE("Result<void,E> - Basic Err operations") {
  Result<void, std::string> res = Err(std::string("failed"));
  REQUIRE(res.is_err());
  CHECK(!res.is_ok());
  CHECK(res.unwrap_err() == "failed");
}

TEST_CASE("Result<void,E> - Throws on invalid access") {
  Result<void, std::string> ok_res = Ok();
  Result<void, std::string> err_res = Err(std::string("Error"));
  CHECK_THROWS_AS(err_res.unwrap(), std::runtime_error);
  CHECK_THROWS_AS(ok_res.unwrap_err(), std::runtime_error);
}

TEST_CASE("Result<void,E> - expect()/expect_err()") {
  SUBCASE("expect on Err with HasToString") {
    Result<void, CustomTestError> res = Err(CustomTestError{"boom"});
    CHECK_THROWS_AS(std::move(res).expect("Expected success"),
                    std::runtime_error);
  }
  SUBCASE("expect on Err without HasToString") {
    Result<void, PlainTestError> res = Err(PlainTestError{});
    CHECK_THROWS_AS(std::move(res).expect("Expected success"),
                    std::runtime_error);
  }
  SUBCASE("expect_err on Ok") {
    Result<void, std::string> res = Ok();
    CHECK_THROWS_AS(std::move(res).expect_err("Expected failure"),
                    std::runtime_error);
  }
}

TEST_CASE("Result<void,E> - map/map_err") {
  SUBCASE("map on Ok produces value") {
    Result<void, std::string> res = Ok();
    auto mapped = std::move(res).map([]() { return 42; });
    CHECK(mapped.unwrap() == 42);
  }
  SUBCASE("map on Err passes through") {
    Result<void, std::string> res = Err(std::string("bad"));
    auto mapped = std::move(res).map([]() { return 42; });
    CHECK(mapped.is_err());
    CHECK(mapped.unwrap_err() == "bad");
  }
  SUBCASE("map_err on Err transforms error") {
    Result<void, std::string> res = Err(std::string("bad"));
    auto mapped = std::move(res).map_err([](std::string s) { return s + "!"; });
    CHECK(mapped.unwrap_err() == "bad!");
  }
  SUBCASE("map_err on Ok passes through") {
    Result<void, std::string> res = Ok();
    auto mapped = std::move(res).map_err([](std::string s) { return s + "!"; });
    CHECK(mapped.is_ok());
    CHECK_NOTHROW(mapped.unwrap());
  }
}
TEST_CASE("Result - map() with void-returning func") {
  SUBCASE("map on Ok with void func transitions to Result<void,E>") {
    Result<int, std::string> res = Ok(5);
    int captured = 0;
    auto mapped = std::move(res).map([&captured](int x) { captured = x; });
    CHECK(captured == 5);
    CHECK(mapped.is_ok());
    CHECK_NOTHROW(mapped.unwrap());
  }
  SUBCASE("map on Err with void func passes error through") {
    Result<int, std::string> res = Err(std::string("bad"));
    bool called = false;
    auto mapped = std::move(res).map([&called](int) { called = true; });
    CHECK(!called);
    CHECK(mapped.is_err());
    CHECK(mapped.unwrap_err() == "bad");
  }
}
#endif
