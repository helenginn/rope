#include <doctest/doctest.h>
#include <vagabond/core/PathManager.h>
#include <cmath>

static PathManager::PathEnergies energies(float vdw, float torsion)
{
	PathManager::PathEnergies e;
	e.vdw = vdw;
	e.torsion = torsion;
	e.paths = 1;
	return e;
}

TEST_CASE("path matrix: energies are relative to the first instance's own path")
{
	PathManager::EnergyMatrix absolute;
	absolute["a"]["a"] = energies(10, 1);
	absolute["a"]["b"] = energies(12, 4);
	absolute["b"]["b"] = energies(5, 2);
	absolute["b"]["a"] = energies(9, 3);

	PathManager::EnergyMatrix relative = PathManager::relativeEnergies(absolute);

	CHECK(relative.at("a").at("a").vdw == 0);
	CHECK(relative.at("a").at("b").vdw == 2);
	CHECK(relative.at("a").at("b").torsion == 3);
	CHECK(relative.at("b").at("b").vdw == 0);
	CHECK(relative.at("b").at("a").vdw == 4);
	CHECK(relative.at("b").at("a").torsion == 1);
}

TEST_CASE("path matrix: nothing is subtracted without a path to itself")
{
	PathManager::EnergyMatrix absolute;
	absolute["a"]["b"] = energies(7, 3);

	PathManager::EnergyMatrix relative = PathManager::relativeEnergies(absolute);

	CHECK(relative.at("a").at("b").vdw == 7);
	CHECK(relative.at("a").at("b").torsion == 3);
	// the reference entry is reported as zero, as path-matrix= does
	CHECK(relative.at("a").at("a").vdw == 0);
}

TEST_CASE("path matrix: Boltzmann probabilities relative to the lowest energy")
{
	const float kT = 2.57;

	PathManager::EnergyMatrix relative;
	relative["a"]["a"] = energies(0, 0);
	relative["a"]["b"] = energies(kT, 0);
	relative["b"]["b"] = energies(-kT, 0);
	relative["b"]["c"] = energies(INFINITY, 0);

	std::vector<std::string> names;
	float lowest = 0;
	float highest = 0;
	Eigen::MatrixXf p = PathManager::boltzmannMatrix(relative, names, kT,
	                                                 &lowest, &highest);

	REQUIRE(names.size() == 3);
	CHECK(names[0] == "a");
	CHECK(names[1] == "b");
	CHECK(names[2] == "c");
	REQUIRE(p.rows() == 3);
	REQUIRE(p.cols() == 3);

	CHECK(lowest == doctest::Approx(-kT));
	CHECK(highest == doctest::Approx(kT));

	// the lowest energy is the most probable, at 1
	CHECK(p(1, 1) == doctest::Approx(1.0));
	CHECK(p(0, 0) == doctest::Approx(exp(-1.0)));
	CHECK(p(0, 1) == doctest::Approx(exp(-2.0)));

	// all between 0 and 1 where there is a result
	CHECK(p(0, 0) > 0);
	CHECK(p(0, 1) <= 1);

	// no paths, or a result which is not a number, gives NaN
	CHECK(std::isnan(p(1, 0)));
	CHECK(std::isnan(p(0, 2)));
	CHECK(std::isnan(p(1, 2)));
}

TEST_CASE("path matrix: no energies gives an empty matrix")
{
	PathManager::EnergyMatrix none;
	std::vector<std::string> names;
	Eigen::MatrixXf p = PathManager::boltzmannMatrix(none, names);

	CHECK(names.size() == 0);
	CHECK(p.rows() == 0);
}
