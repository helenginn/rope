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

#include "ForceCalcReports.h"

#ifdef ROPE_INLINE_TESTS

#include <doctest/doctest.h>

using namespace rope::force_calc;

TEST_CASE("ForceCalcResultReport - create")
{
	auto report = ForceCalcResultReport::create();

	CHECK_FALSE(report.stress_strain_correlation.has_value());
	CHECK(report.stress_strain.empty());
	CHECK(report.target_estimate.empty());
	CHECK(report.torsion_electromagnetic.empty());
}

TEST_CASE("ForceCalcResultReport - stress strain rows")
{
	auto report = ForceCalcResultReport::create();

	report.stress_strain.push_back({"rod A", 1.25f, -3.5f});

	REQUIRE(report.stress_strain.size() == 1);
	const auto &row = report.stress_strain.front();
	CHECK(row.desc == "rod A");
	CHECK(row.stress == doctest::Approx(1.25f));
	CHECK(row.strain == doctest::Approx(-3.5f));
}

TEST_CASE("ForceCalcResultReport - target estimate rows")
{
	auto report = ForceCalcResultReport::create();

	report.target_estimate.push_back({1.25f, -3.5f});

	REQUIRE(report.target_estimate.size() == 1);
	const auto &row = report.target_estimate.front();
	CHECK(row.target == doctest::Approx(1.25f));
	CHECK(row.estimate == doctest::Approx(-3.5f));
}

TEST_CASE("ForceCalcResultReport - torsion electromagnetic rows")
{
	auto report = ForceCalcResultReport::create();

	report.torsion_electromagnetic.push_back({0.75f, "rod B"});

	REQUIRE(report.torsion_electromagnetic.size() == 1);
	const auto &row = report.torsion_electromagnetic.front();
	CHECK(row.normalized_dot_product == doctest::Approx(0.75f));
	CHECK(row.desc == "rod B");
}

TEST_CASE("ForceCalcResultReport - zero correlation is a present value")
{
	auto report = ForceCalcResultReport::create();

	report.stress_strain_correlation = 0.f;

	REQUIRE(report.stress_strain_correlation.has_value());
	CHECK(*report.stress_strain_correlation == doctest::Approx(0.f));
}

#endif
