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

#include "OccupancyPropagation.h"
#include <algorithm>
#include <cmath>
#include <unordered_map>

namespace OccupancyPropagation
{

namespace
{

// pooled correlation (and shared weight) of one pair of atoms - sparse,
// since only atoms that were ever in a subnetwork together have an entry.
struct PairStats
{
	double corr = 0; // sum over subnetworks of (mass * weighted correlation)
	double weight = 0; // sum of mass over the subnetworks it was defined in
};

uint64_t pair_key(int a, int b)
{
	if (a > b)
	{
		std::swap(a, b);
	}
	return ((uint64_t)a << 32) | (uint64_t)b;
}

}

std::vector<Eigen::VectorXf>
correct(const std::vector<Subnetwork> &subs, const Eigen::VectorXf &theta,
        const Settings &settings)
{
	std::vector<Eigen::VectorXf> corrected;
	corrected.reserve(subs.size());

	if (subs.empty())
	{
		return corrected;
	}

	// masses are only meaningful relative to one another, so express each
	// in units of the average subnetwork - this is what makes
	// Settings::shrinkWeight a scale-free "number of typical subnetworks".
	double mean_mass = 0;
	for (const Subnetwork &sub : subs)
	{
		mean_mass += sub.mass;
	}
	mean_mass /= (double)subs.size();
	if (!(mean_mass > 0))
	{
		mean_mass = 1;
	}

	// pass 1: pool the within-subnetwork weighted *correlation* of every
	// pair of atoms across every subnetwork that contains both. Correlations
	// (not covariances) are pooled because they are scale-free: an atom's
	// variance differs wildly between subnetworks where it is evenly split
	// and ones where it is nearly fixed, and pooling that variance made the
	// regression slope Cov/Var overshoot - two perfectly correlated atoms
	// are the same variable, and must give a slope of exactly 1. A pair only
	// contributes from a subnetwork where both atoms actually vary (a
	// correlation is undefined otherwise).
	std::unordered_map<uint64_t, PairStats> pooled;

	for (const Subnetwork &sub : subs)
	{
		int k = (int)sub.ids.size();
		if (k == 0 || sub.X.rows() == 0)
		{
			continue;
		}

		Eigen::VectorXf p = sub.p;
		float total = p.sum();
		if (!(total > 0))
		{
			continue;
		}
		p /= total;

		Eigen::RowVectorXf mean = p.transpose() * sub.X;
		Eigen::MatrixXf centred = sub.X.rowwise() - mean;
		Eigen::MatrixXf cov = centred.transpose() * p.asDiagonal() * centred;

		double m = sub.mass / mean_mass;

		for (int i = 0; i < k; i++)
		{
			for (int j = i + 1; j < k; j++)
			{
				float denom = std::sqrt(cov(i, i) * cov(j, j));
				if (!(denom > 1e-8f))
				{
					continue; // one of them is constant here
				}

				PairStats &stats = pooled[pair_key(sub.ids[i], sub.ids[j])];
				stats.corr += m * (cov(i, j) / denom);
				stats.weight += m;
			}
		}
	}

	// pooled, shrunk correlation of a pair: shrunk towards zero by how
	// little weight it was seen with (see Settings::shrinkWeight).
	auto correlation = [&](int a, int b) -> float
	{
		if (a == b)
		{
			return 1.f;
		}

		auto it = pooled.find(pair_key(a, b));
		if (it == pooled.end() || !(it->second.weight > 0))
		{
			return 0.f;
		}

		const PairStats &stats = it->second;
		return (float)(stats.corr / (stats.weight + settings.shrinkWeight));
	};

	// pass 2: per subnetwork, regress every atom on all the others at once
	// and correct it by how far those others sit from their global values.
	//
	// With K the pooled (ridge-regularised) covariance restricted to this
	// subnetwork's atoms and P = K^-1, the regression of atom i on all the
	// others has slopes beta_ij = -P_ij / P_ii. So
	//     sum_{j != i} beta_ij * d_j = d_i - (P d)_i / P_ii
	// with d_j = theta_j - f_j, which gives every atom's correction from
	// the one inversion, rather than one regression per atom.
	for (const Subnetwork &sub : subs)
	{
		int k = (int)sub.ids.size();
		Eigen::VectorXf f = sub.fraction;

		if (k < 2)
		{
			corrected.push_back(f);
			continue;
		}

		// this subnetwork's own local standard deviations turn the pooled
		// correlations back into covariances in its own units - so a slope
		// is (correlation) x (target's local spread / reference's local
		// spread), exactly 1 for two identical atoms.
		Eigen::VectorXf sd(k);
		{
			Eigen::VectorXf p = sub.p;
			p /= std::max(p.sum(), 1e-20f);
			Eigen::RowVectorXf mean = p.transpose() * sub.X;
			Eigen::MatrixXf centred = sub.X.rowwise() - mean;
			for (int i = 0; i < k; i++)
			{
				sd(i) = std::sqrt(std::max(0.f,
				                  centred.col(i).cwiseAbs2().dot(p)));
			}
		}

		Eigen::MatrixXf K(k, k);
		for (int i = 0; i < k; i++)
		{
			for (int j = i; j < k; j++)
			{
				float c = correlation(sub.ids[i], sub.ids[j]) * sd(i) * sd(j);
				K(i, j) = K(j, i) = c;
			}
		}

		// ridge on each reference's own variance, with a small floor so an
		// atom that never varies (zero variance) still gets a well-defined
		// diagonal rather than a singular row.
		float mean_var = K.diagonal().mean();
		float floor = std::max(mean_var * 1e-3f, 1e-6f);
		for (int i = 0; i < k; i++)
		{
			K(i, i) += settings.ridge * std::max(K(i, i), floor);
		}

		// pairwise-complete correlations stitched from different subsets of
		// subnetworks needn't form a valid (positive semi-definite) matrix,
		// so invert through an eigendecomposition with the eigenvalues
		// floored, which guarantees P is a proper positive-definite inverse.
		Eigen::SelfAdjointEigenSolver<Eigen::MatrixXf> solver(K);
		Eigen::VectorXf values = solver.eigenvalues();
		float biggest = values.maxCoeff();
		float min_value = std::max(biggest * 1e-6f, 1e-9f);
		for (int i = 0; i < k; i++)
		{
			values(i) = 1.f / std::max(values(i), min_value);
		}

		Eigen::MatrixXf P = solver.eigenvectors() * values.asDiagonal() *
		solver.eigenvectors().transpose();

		Eigen::VectorXf d(k);
		for (int i = 0; i < k; i++)
		{
			int id = sub.ids[i];
			float t = (id >= 0 && id < theta.size()) ? theta(id) : f(i);
			d(i) = std::isfinite(t) ? t - f(i) : 0.f;
		}

		Eigen::VectorXf g = P * d;
		Eigen::VectorXf result(k);

		for (int i = 0; i < k; i++)
		{
			float delta = d(i) - g(i) / P(i, i);
			if (!std::isfinite(delta))
			{
				delta = 0;
			}

			result(i) = std::min(1.f, std::max(0.f, f(i) + delta));
		}

		corrected.push_back(result);
	}

	return corrected;
}

}
