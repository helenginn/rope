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
#include <type_traits>
#include <vector>
#include <string>

using namespace rope::force_calc;

TEST_CASE("ReportTable - construction properties")
{
	static_assert(std::is_aggregate_v<TableStressStrain>);
	static_assert(std::is_aggregate_v<TableTargetEstimate>);
	static_assert(std::is_aggregate_v<TableTorsionElectromagnetic>);

	static_assert(!std::is_default_constructible_v<TableStressStrain>);
	static_assert(!std::is_default_constructible_v<TableTargetEstimate>);
	static_assert(!std::is_default_constructible_v<TableTorsionElectromagnetic>);

	static_assert(std::is_aggregate_v<ForceCalcResultReport>);
	static_assert(!std::is_default_constructible_v<ForceCalcResultReport>);

	CHECK(true);
}

TEST_CASE("ReportTable - stress strain schema")
{
	auto table = TableStressStrain::create();

	auto headers = table.data.headers();

	REQUIRE(headers.size() == 3);
	CHECK(headers[0] == "desc");
	CHECK(headers[1] == "stress");
	CHECK(headers[2] == "strain");

	CHECK(table.data.typeForHeader("desc") == TabulatedData::Text);
	CHECK(table.data.typeForHeader("stress") == TabulatedData::Number);
	CHECK(table.data.typeForHeader("strain") == TabulatedData::Number);
}

TEST_CASE("ReportTable - target estimate schema")
{
	auto table = TableTargetEstimate::create();

	auto headers = table.data.headers();

	REQUIRE(headers.size() == 2);
	CHECK(headers[0] == "target");
	CHECK(headers[1] == "estimate");

	CHECK(table.data.typeForHeader("target") == TabulatedData::Number);
	CHECK(table.data.typeForHeader("estimate") == TabulatedData::Number);
}

TEST_CASE("ReportTable - torsion electromagnetic schema")
{
	auto table = TableTorsionElectromagnetic::create();

	auto headers = table.data.headers();

	REQUIRE(headers.size() == 2);
	CHECK(headers[0] == "normalized_dot_product");
	CHECK(headers[1] == "desc");

	CHECK(table.data.typeForHeader("normalized_dot_product") ==
	      TabulatedData::Number);
	CHECK(table.data.typeForHeader("desc") == TabulatedData::Text);
}

TEST_CASE("ReportTable - add stress strain entry")
{
	auto table = TableStressStrain::create();

	table.add({"rod A", 1.25f, -3.5f});

	REQUIRE(table.data.entryCount() == 1);

	auto entry = table.data.entry(0);

	REQUIRE(entry.size() == 3);
	CHECK(entry[0] == "rod A");
	CHECK(std::stof(entry[1]) == doctest::Approx(1.25f));
	CHECK(std::stof(entry[2]) == doctest::Approx(-3.5f));
}

TEST_CASE("ReportTable - add target estimate entry")
{
	auto table = TableTargetEstimate::create();

	table.add({1.25f, -3.5f});

	REQUIRE(table.data.entryCount() == 1);

	auto entry = table.data.entry(0);

	REQUIRE(entry.size() == 2);
	CHECK(std::stof(entry[0]) == doctest::Approx(1.25f));
	CHECK(std::stof(entry[1]) == doctest::Approx(-3.5f));
}

TEST_CASE("ReportTable - add torsion electromagnetic entry")
{
	auto table = TableTorsionElectromagnetic::create();

	table.add({0.75f, "rod B"});

	REQUIRE(table.data.entryCount() == 1);

	auto entry = table.data.entry(0);

	REQUIRE(entry.size() == 2);
	CHECK(std::stof(entry[0]) == doctest::Approx(0.75f));
	CHECK(entry[1] == "rod B");
}

TEST_CASE("ForceCalcResultReport - create")
{
	auto report = ForceCalcResultReport::create();

	CHECK(report.stress_strain_correlation == 0.f);

	CHECK(report.stress_strain.data.entryCount() == 0);
	CHECK(report.target_estimate.data.entryCount() == 0);
	CHECK(report.torsion_electromagnetic.data.entryCount() == 0);

	CHECK(report.stress_strain.data.headers() ==
	      std::vector<std::string>{"desc", "stress", "strain"});

	CHECK(report.target_estimate.data.headers() ==
	      std::vector<std::string>{"target", "estimate"});

	CHECK(report.torsion_electromagnetic.data.headers() ==
	      std::vector<std::string>{"normalized_dot_product", "desc"});
}

TEST_CASE("ForceCalcResultReport - tables accept report data")
{
	auto report = ForceCalcResultReport::create();

	report.stress_strain.add({"rod A", 10.f, 20.f});
	report.target_estimate.add({30.f, 40.f});
	report.torsion_electromagnetic.add({0.5f, "rod B"});

	CHECK(report.stress_strain.data.entryCount() == 1);
	CHECK(report.target_estimate.data.entryCount() == 1);
	CHECK(report.torsion_electromagnetic.data.entryCount() == 1);
}

#endif
