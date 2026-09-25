#include "ForceScope.h"

#ifdef ROPE_INLINE_TESTS
#include <doctest/doctest.h>

#include <limits>
#include <utility>

auto radius_pair = rope::force_calc::scope::RadiusPair::from_constants(2, 3);

namespace
{
	using rope::force_calc::scope::RadiusError;
	using rope::force_calc::scope::RadiusPair;

	constexpr double inf = std::numeric_limits<double>::infinity();
	constexpr double not_a_number = std::numeric_limits<double>::quiet_NaN();
	constexpr double dmax = std::numeric_limits<double>::max();

	void expect_error(double inner, double outer, RadiusError::Code expected)
	{
		auto result = RadiusPair::create(inner, outer);
		REQUIRE(result.is_err());
		CHECK(result.unwrap_err() == RadiusError{expected});
	}

	void expect_ok(double inner, double outer)
	{
		auto result = RadiusPair::create(inner, outer);
		REQUIRE(!result.is_err());
		RadiusPair pair = std::move(result).unwrap();
		CHECK(pair.inner_radius() == doctest::Approx(inner));
		CHECK(pair.outer_radius() == doctest::Approx(outer));
	}
} // namespace

TEST_CASE("RadiusPair::create accepts valid radii")
{
	SUBCASE("inner smaller than outer")
	{
		expect_ok(1.0, 2.0);
	}

	SUBCASE("inner equal to outer")
	{
		expect_ok(3.0, 3.0);
	}

	SUBCASE("both zero")
	{
		expect_ok(0.0, 0.0);
	}

	SUBCASE("inner zero")
	{
		expect_ok(0.0, 5.0);
	}

	SUBCASE("negative zero counts as non-negative")
	{
		expect_ok(-0.0, 1.0);
	}

	SUBCASE("largest finite values")
	{
		expect_ok(dmax, dmax);
	}
}

TEST_CASE("RadiusPair::create rejects non-finite values")
{
	SUBCASE("NaN as inner")
	{
		expect_error(not_a_number, 1.0, RadiusError::Code::NonFinite);
	}

	SUBCASE("NaN as outer")
	{
		expect_error(1.0, not_a_number, RadiusError::Code::NonFinite);
	}

	SUBCASE("infinity as inner")
	{
		expect_error(inf, inf, RadiusError::Code::NonFinite);
	}

	SUBCASE("infinity as outer")
	{
		expect_error(1.0, inf, RadiusError::Code::NonFinite);
	}

	SUBCASE("negative infinity")
	{
		expect_error(-inf, 1.0, RadiusError::Code::NonFinite);
		expect_error(1.0, -inf, RadiusError::Code::NonFinite);
	}
}

TEST_CASE("RadiusPair::create rejects negative radii")
{
	SUBCASE("negative inner")
	{
		expect_error(-1.0, 2.0, RadiusError::Code::NegativeRadius);
	}

	SUBCASE("negative outer")
	{
		expect_error(0.0, -2.0, RadiusError::Code::NegativeRadius);
	}

	SUBCASE("both negative")
	{
		expect_error(-2.0, -1.0, RadiusError::Code::NegativeRadius);
	}
}

TEST_CASE("RadiusPair::create rejects inner greater than outer")
{
	SUBCASE("clearly greater")
	{
		expect_error(5.0, 1.0, RadiusError::Code::InnerExceedsOuter);
	}

	SUBCASE("barely greater")
	{
		expect_error(1.0 + 1e-12, 1.0, RadiusError::Code::InnerExceedsOuter);
	}
}

TEST_CASE("RadiusPair::create error precedence")
{
	SUBCASE("non-finite wins over negative")
	{
		expect_error(not_a_number, -1.0, RadiusError::Code::NonFinite);
	}

	SUBCASE("negative wins over inner exceeding outer")
	{
		// inner > outer, but outer is negative
		expect_error(1.0, -1.0, RadiusError::Code::NegativeRadius);
	}
}

TEST_CASE("RadiusPair::from_constants works at compile time")
{
	constexpr RadiusPair pair = RadiusPair::from_constants(1.5, 4.5);

	static_assert(pair.inner_radius() == 1.5);
	static_assert(pair.outer_radius() == 4.5);

	CHECK(pair.inner_radius() == doctest::Approx(1.5));
	CHECK(pair.outer_radius() == doctest::Approx(4.5));

	// RadiusPair::from_constants(5.0, 1.0) must not compile. That cannot be
	// checked at runtime, so a compile-fail test would be needed for it.
}

TEST_CASE("RadiusError carries constexpr descriptions")
{
	constexpr RadiusError order_error{RadiusError::Code::InnerExceedsOuter};
	static_assert(order_error.toString() == "inner > outer");
	CHECK(order_error.toString() == "inner > outer");
	CHECK(RadiusError{RadiusError::Code::NonFinite}.toString() ==
	      "radii must be finite");
	CHECK(RadiusError{RadiusError::Code::NegativeRadius}.toString() ==
	      "radii must be non-negative");
}

TEST_CASE("RadiusPair errors appear in Result diagnostics")
{
	auto result = RadiusPair::create(5, 3);
	CHECK_THROWS_WITH_AS(result.unwrap(),
	                     "Called unwrap on an Err value!: inner > outer",
	                     std::runtime_error);
	CHECK_THROWS_WITH_AS(std::move(result).expect("Invalid radii"),
	                     "Invalid radii: inner > outer", std::runtime_error);
}

TEST_CASE("RadiusPair::create is usable in constant expressions")
{
	constexpr auto check = []() constexpr {
		auto result = RadiusPair::create(1.0, 2.0);
		return !result.is_err();
	};
	static_assert(check());

	constexpr auto check_invalid = []() constexpr {
		auto result = RadiusPair::create(2.0, 1.0);
		return result.is_err();
	};
	static_assert(check_invalid());
}

#endif // ROPE_INLINE_TESTS
