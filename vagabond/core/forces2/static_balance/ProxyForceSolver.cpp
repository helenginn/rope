#include "ProxyForceSolver.h"
#include "BalanceEquations.h"

#include <utility>
#include <vector>
#include <vagabond/utils/Eigen/SparseQR>

namespace rope::force_calc
{
	namespace
	{
		using SparseMatrix = Eigen::SparseMatrix<float>;
		using SolveResult = rope::utils::Result<ProxyForceSolution, std::string_view>;

		SolveResult solveSparseSystem(const SparseMatrix &matrix,
		                              const Eigen::VectorXf &target)
		{
			Eigen::SparseQR<SparseMatrix, Eigen::COLAMDOrdering<int>> qr(matrix);
			if (qr.info() != Eigen::Success)
			{
				return rope::utils::Err{"proxy force factorization failed"};
			}

			Eigen::VectorXf components = qr.solve(target);
			if (!components.allFinite())
			{
				return rope::utils::Err{"proxy force solution is non-finite"};
			}

			return rope::utils::Ok{
			    ProxyForceSolution{std::move(components), qr.rank()}};
		}
	}

	SolveResult solveProxyForces(const BalanceEquations &equations)
	{
		const auto &[matrix, target, proxy_atoms] = equations;
		if (!matrix.isCompressed() || matrix.rows() != target.size() ||
		    matrix.cols() % 3 != 0 ||
		    static_cast<std::size_t>(matrix.cols() / 3) != proxy_atoms.size())
		{
			return rope::utils::Err{"invalid proxy force system"};
		}

		if (!matrix.coeffs().allFinite() || !target.allFinite())
		{
			return rope::utils::Err{"proxy force system is non-finite"};
		}

		if (matrix.nonZeros() == 0)
		{
			return rope::utils::Ok{
			    ProxyForceSolution{Eigen::VectorXf::Zero(matrix.cols()), 0}};
		}

		// Empty rows contribute a constant to the least-squares error.
		// Eigen SparseQR requires them to be removed before factorization.
		std::vector<int> compact_row(matrix.rows(), -1);
		for (Eigen::Index i = 0; i < matrix.nonZeros(); ++i)
		{
			compact_row[matrix.innerIndexPtr()[i]] = 0;
		}

		int row_count = 0;
		for (auto &row : compact_row)
		{
			if (row >= 0)
			{
				row = row_count++;
			}
		}

		if (row_count == matrix.rows())
		{
			return solveSparseSystem(matrix, target);
		}

		Eigen::VectorXf compact_target(row_count);
		for (Eigen::Index row = 0; row < matrix.rows(); ++row)
		{
			if (compact_row[row] >= 0)
			{
				compact_target(compact_row[row]) = target(row);
			}
		}

		SparseMatrix compact_matrix(row_count, matrix.cols());
		compact_matrix.reserve(matrix.nonZeros());
		for (Eigen::Index column = 0; column < matrix.cols(); ++column)
		{
			compact_matrix.startVec(column);
			for (SparseMatrix::InnerIterator entry(matrix, column); entry; ++entry)
			{
				compact_matrix.insertBack(compact_row[entry.row()], column) =
				    entry.value();
			}
		}
		compact_matrix.finalize();

		return solveSparseSystem(compact_matrix, compact_target);
	}
}

#ifdef ROPE_INLINE_TESTS
#include <doctest/doctest.h>

TEST_CASE("solveProxyForces fits a rank-deficient balance system with an empty row")
{
	using namespace rope::force_calc;

	BalanceEquations equations;
	equations.matrix.resize(4, 3);
	equations.matrix.insert(1, 0) = 1.f;
	equations.matrix.insert(2, 0) = 1.f;
	equations.matrix.insert(3, 1) = 1.f;
	equations.matrix.makeCompressed();
	equations.target = Eigen::Vector4f(8.f, 1.f, 3.f, -2.f);
	equations.proxy_atoms = {4};

	const auto result = solveProxyForces(equations);
	REQUIRE(result.is_ok());
	const auto &solution = result.unwrap();
	CHECK(solution.rank == 2);
	REQUIRE(solution.components.size() == 3);
	CHECK(solution.components(0) == doctest::Approx(2.f));
	CHECK(solution.components(1) == doctest::Approx(-2.f));
	CHECK(solution.components(2) == 0.f);
}
#endif // ROPE_INLINE_TESTS
