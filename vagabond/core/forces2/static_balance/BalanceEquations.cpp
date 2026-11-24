#include "BalanceEquations.h"

#include "../geometry/ForceTopology.h"
#include <cmath>
#include <limits>
#include <utility>

namespace rope::force_calc
{
	namespace
	{
		using Coefficient = Eigen::Triplet<float>;
		using Error = std::string_view;

		constexpr int components = 3;
		constexpr int rows_per_bond = 2 * components;
		constexpr int entries_per_bond = 18;

		struct BondBalance
		{
			glm::vec3 half_bond;
			glm::vec3 force_target;
			glm::vec3 moment_target;
		};

		rope::utils::Result<std::vector<int>, Error>
		indexProxyColumns(std::size_t atom_count,
		                  std::span<const std::size_t> proxy_atoms)
		{
			std::vector<int> columns(atom_count, -1);
			for (std::size_t i = 0; i < proxy_atoms.size(); ++i)
			{
				const auto atom = proxy_atoms[i];
				if (atom >= atom_count || columns[atom] >= 0)
				{
					return rope::utils::Err{
					    "proxy atom indices must be valid and unique"};
				}
				columns[atom] = static_cast<int>(i * components);
			}
			return rope::utils::Ok{std::move(columns)};
		}

		bool isFinite(const glm::vec3 &v)
		{
			return std::isfinite(v.x) && std::isfinite(v.y) &&
			       std::isfinite(v.z);
		}

		rope::utils::Result<BondBalance, Error>
		calculateBondBalance(BondEndpoints bond, const BalanceInput &input,
		                     const glm::vec3 &applied_torque)
		{
			const auto [left, right] = bond;
			const auto positions = input.initial_positions_angstroms;
			if (left >= positions.size() || right >= positions.size() ||
			    left == right)
			{
				return rope::utils::Err{"invalid bond endpoints"};
			}

			const glm::vec3 bond_vector = positions[right] - positions[left];
			const auto &left_force = input.applied_forces[left];
			const auto &right_force = input.applied_forces[right];

			// Preserve the legacy sign for the applied-force couple.
			const glm::vec3 known_couple =
			    glm::cross(bond_vector, 0.5f * (left_force - right_force));

			const BondBalance balance{
			    0.5f * bond_vector,
			    -(left_force + right_force),
			    -applied_torque - known_couple,
			};
			if (!isFinite(balance.half_bond) ||
			    !isFinite(balance.force_target) ||
			    !isFinite(balance.moment_target))
			{
				return rope::utils::Err{
				    "balance equations contain non-finite values"};
			}
			return rope::utils::Ok{balance};
		}

		void appendProxyColumns(std::vector<Coefficient> &entries, int row,
		                        int column, const glm::vec3 &arm)
		{
			if (column < 0)
			{
				return;
			}

			for (int axis = 0; axis < components; ++axis)
			{
				entries.emplace_back(row + axis, column + axis, 1.f);

				const int first = (axis + 1) % components;
				const int second = (axis + 2) % components;
				entries.emplace_back(row + components + axis, column + first,
				                     -arm[second]);
				entries.emplace_back(row + components + axis, column + second,
				                     arm[first]);
			}
		}
	}

	// Eigen allocates an empty sparse matrix before swap.
	// NOLINTNEXTLINE(performance-noexcept-move-constructor)
	BalanceEquations::BalanceEquations(BalanceEquations &&other)
	: target(std::move(other.target)), proxy_atoms(std::move(other.proxy_atoms))
	{
		matrix.swap(other.matrix);
	}

	rope::utils::Result<BalanceEquations, std::string_view>
	buildBalanceEquations(const ForceTopology &topology,
	                      const BalanceInput &input)
	{
		const auto bond_count = topology.bonds.size();
		if (input.initial_positions_angstroms.size() != topology.atom_count ||
		    input.applied_forces.size() != topology.atom_count ||
		    input.applied_torques.size() != bond_count)
		{
			return rope::utils::Err{
			    "positions and loads must match the topology"};
		}

		constexpr auto max_index = std::numeric_limits<int>::max();
		if (bond_count >
		        static_cast<std::size_t>(max_index / entries_per_bond) ||
		    input.proxy_atoms.size() >
		        static_cast<std::size_t>(max_index / components))
		{
			return rope::utils::Err{
			    "balance system exceeds Eigen's index range"};
		}

		const auto indexed_columns =
		    indexProxyColumns(topology.atom_count, input.proxy_atoms);
		if (indexed_columns.is_err())
		{
			return rope::utils::Err{indexed_columns.unwrap_err()};
		}
		const auto &proxy_columns = indexed_columns.unwrap();

		const int rows = static_cast<int>(bond_count * rows_per_bond);
		const int columns =
		    static_cast<int>(input.proxy_atoms.size() * components);
		BalanceEquations equations;
		equations.target.resize(rows);
		std::vector<Coefficient> entries;
		entries.reserve(bond_count * entries_per_bond);

		for (std::size_t i = 0; i < bond_count; ++i)
		{
			const auto bond = topology.bonds[i];
			const auto result =
			    calculateBondBalance(bond, input, input.applied_torques[i]);
			if (result.is_err())
			{
				return rope::utils::Err{result.unwrap_err()};
			}
			const auto &balance = result.unwrap();
			const int row = static_cast<int>(i * rows_per_bond);
			for (int axis = 0; axis < components; ++axis)
			{
				equations.target(row + axis) = balance.force_target[axis];
				equations.target(row + components + axis) =
				    balance.moment_target[axis];
			}

			appendProxyColumns(entries, row, proxy_columns[bond.left_atom],
			                   -balance.half_bond);
			appendProxyColumns(entries, row, proxy_columns[bond.right_atom],
			                   balance.half_bond);
		}

		equations.matrix.resize(rows, columns);
		equations.matrix.setFromTriplets(entries.begin(), entries.end());
		equations.proxy_atoms.assign(input.proxy_atoms.begin(),
		                             input.proxy_atoms.end());
		return rope::utils::Ok{std::move(equations)};
	}
}

#ifdef ROPE_INLINE_TESTS
#include <array>
#include <doctest/doctest.h>

TEST_CASE("buildBalanceEquations assembles force and moment rows")
{
	using namespace rope::force_calc;

	const ForceTopology topology{3, {{0, 1}, {1, 2}}, {{0, 1}}};
	const std::array positions{
	    glm::vec3(0.f, 0.f, 0.f),
	    glm::vec3(2.f, 0.f, 0.f),
	    glm::vec3(2.f, 2.f, 0.f),
	};
	const std::array torques{glm::vec3(0.f, 0.f, 4.f),
	                         glm::vec3(0.f, 0.f, -4.f)};
	const std::array<std::size_t, 3> proxy_atoms{0, 1, 2};
	std::array forces{glm::vec3(0.f), glm::vec3(0.f), glm::vec3(0.f)};
	const BalanceInput input{positions, forces, torques, proxy_atoms};

	SUBCASE("one angle adds opposite moments to its two bonds")
	{
		auto result = buildBalanceEquations(topology, input);
		REQUIRE(result.is_ok());
		const auto &equations = result.unwrap();
		CHECK(equations.matrix.rows() == 12);
		CHECK(equations.matrix.cols() == 9);
		CHECK(equations.matrix.coeff(1, 1) == 1.f);
		CHECK(equations.matrix.coeff(1, 4) == 1.f);
		CHECK(equations.matrix.coeff(5, 1) == -1.f);
		CHECK(equations.matrix.coeff(5, 4) == 1.f);
		CHECK(equations.target(5) == -4.f);
		CHECK(equations.target(11) == 4.f);
	}

	SUBCASE("an applied force affects both the force and moment targets")
	{
		forces[0] = {0.f, 2.f, 0.f};
		auto result = buildBalanceEquations(topology, input);
		REQUIRE(result.is_ok());
		CHECK(result.unwrap().target(1) == -2.f);
		CHECK(result.unwrap().target(5) == -6.f);
	}

	SUBCASE("proxy columns follow the requested atom order")
	{
		const std::array<std::size_t, 2> selected{2, 0};
		auto result = buildBalanceEquations(
		    topology, BalanceInput{positions, forces, torques, selected});
		REQUIRE(result.is_ok());
		const auto &equations = result.unwrap();
		CHECK(equations.matrix.cols() == 6);
		CHECK(equations.proxy_atoms == std::vector<std::size_t>{2, 0});
		CHECK(equations.matrix.coeff(0, 0) == 0.f);
		CHECK(equations.matrix.coeff(0, 3) == 1.f);
		CHECK(equations.matrix.coeff(6, 0) == 1.f);
	}
}
#endif // ROPE_INLINE_TESTS
