#include "StaticForceAnalysis.h"

#include "geometry/ForceTopology.h"
#include "terms/BondAngleTorque.h"

#include <cstddef>
#include <utility>
#include <vector>
#include <vagabond/core/Atom.h>
#include <vagabond/core/AtomGroup.h>

namespace rope::force_calc
{
	Eigen::VectorXf StaticForceResult::estimates() const
	{
		return equations.matrix * solution.components;
	}

	rope::utils::Result<StaticForceResult, std::string>
	analyzeStaticForces(const AtomGroup &group)
	{
		if (!std::in_range<int>(group.bondAngleCount()) ||
		    !std::in_range<int>(group.bondLengthCount()))
		{
			return rope::utils::Err{"too many restraints for static force analysis"};
		}

		const auto topology_result = readForceTopology(group);
		if (topology_result.is_err())
		{
			return rope::utils::Err{topology_result.unwrap_err().toString()};
		}
		const auto &topology = topology_result.unwrap();

		std::vector<glm::vec3> torques(topology.bonds.size(), glm::vec3(0.f));
		const auto added = addBondAngleTorques(group, topology, torques);
		if (added.is_err())
		{
			return rope::utils::Err{added.unwrap_err()};
		}

		const auto &atoms = group.atomVector();
		std::vector<glm::vec3> positions;
		std::vector<std::size_t> proxy_atoms;
		positions.reserve(atoms.size());
		proxy_atoms.reserve(atoms.size());
		for (std::size_t i = 0; i < atoms.size(); ++i)
		{
			const Atom &atom = *atoms[i];
			positions.push_back(atom.initialPosition());
			if (atom.bondLengthCount() > 0 && atom.symmetryCopyOf() == nullptr)
			{
				proxy_atoms.push_back(i);
			}
		}

		const std::vector<glm::vec3> forces(atoms.size(), glm::vec3(0.f));
		const BalanceInput input{positions, forces, torques, proxy_atoms};
		auto equations = buildBalanceEquations(topology, input);
		if (equations.is_err())
		{
			return rope::utils::Err{std::string(equations.unwrap_err())};
		}

		auto solution = solveProxyForces(equations.unwrap());
		if (solution.is_err())
		{
			return rope::utils::Err{std::string(solution.unwrap_err())};
		}

		return rope::utils::Ok{StaticForceResult{
		    std::move(equations.unwrap()),
		    std::move(solution.unwrap()),
		}};
	}
}

#ifdef ROPE_INLINE_TESTS
#include <doctest/doctest.h>
#include <vagabond/core/BondAngle.h>
#include <vagabond/core/BondLength.h>

TEST_CASE("analyzeStaticForces solves the bond-angle balance")
{
	using rope::force_calc::analyzeStaticForces;

	Atom left("GLY", "N"), centre("GLY", "CA"), right("GLY", "C");
	left.setInitialPosition({2.f, 0.f, 0.f});
	centre.setInitialPosition({0.f, 0.f, 0.f});
	right.setInitialPosition({0.f, 2.f, 0.f});

	AtomGroup group;
	group.add(&left);
	group.add(&centre);
	group.add(&right);
	BondLength left_bond(&group, &left, &centre, 2.0);
	BondLength right_bond(&group, &centre, &right, 2.0);
	BondAngle angle(&group, &left, &centre, &right, 100.0, 2.0);

	SUBCASE("balances opposite torques on the two bonds")
	{
		const auto result = analyzeStaticForces(group);
		REQUIRE_MESSAGE(result.is_ok(), result.unwrap_err());
		const auto &analysis = result.unwrap();
		CHECK(analysis.equations.proxy_atoms == std::vector<std::size_t>{0, 1, 2});
		REQUIRE(analysis.equations.target.size() == 12);
		CHECK(analysis.equations.target(5) == doctest::Approx(-5.f));
		CHECK(analysis.equations.target(11) == doctest::Approx(5.f));
		CHECK(analysis.estimates().isApprox(analysis.equations.target));
	}

	SUBCASE("skips angles with hydrogen at an end")
	{
		right.setElementSymbol("H");
		const auto result = analyzeStaticForces(group);
		REQUIRE_MESSAGE(result.is_ok(), result.unwrap_err());
		CHECK(result.unwrap().equations.target.isZero());
	}
}
#endif // ROPE_INLINE_TESTS
