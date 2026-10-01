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
#include <numeric>

namespace OccupancyPropagation
{

namespace
{

uint64_t pair_key(int a, int b)
{
	if (a > b)
	{
		std::swap(a, b);
	}
	return ((uint64_t)a << 32) | (uint64_t)b;
}

// weighted standard deviation of every column of sub.X.
Eigen::VectorXf local_sd(const Subnetwork &sub, const Eigen::VectorXf &p)
{
	Eigen::RowVectorXf mean = p.transpose() * sub.X;
	Eigen::MatrixXf centred = sub.X.rowwise() - mean;
	Eigen::VectorXf sd(sub.X.cols());

	for (int i = 0; i < sd.size(); i++)
	{
		sd(i) = std::sqrt(std::max(0.f, centred.col(i).cwiseAbs2().dot(p)));
	}

	return sd;
}

Eigen::VectorXf normalised_weights(const Subnetwork &sub)
{
	Eigen::VectorXf p = sub.p;
	float total = p.sum();
	if (total > 0)
	{
		p /= total;
	}
	return p;
}

}

Propagator::Propagator(const Settings &settings) : _settings(settings)
{

}

void Propagator::accumulate(const Subnetwork &sub)
{
	_totalMass += sub.mass;
	_count++;

	int k = (int)sub.ids.size();
	if (k < 2 || sub.X.rows() == 0 || !(sub.p.sum() > 0))
	{
		return;
	}

	Eigen::VectorXf p = normalised_weights(sub);
	Eigen::RowVectorXf mean = p.transpose() * sub.X;
	Eigen::MatrixXf centred = sub.X.rowwise() - mean;
	Eigen::MatrixXf cov = centred.transpose() * p.asDiagonal() * centred;

	for (int i = 0; i < k; i++)
	{
		for (int j = i + 1; j < k; j++)
		{
			// a correlation is undefined if either atom is constant here
			float denom = std::sqrt(cov(i, i) * cov(j, j));
			if (!(denom > 1e-8f))
			{
				continue;
			}

			PairStats &stats = _pooled[pair_key(sub.ids[i], sub.ids[j])];
			stats.corr += sub.mass * (cov(i, j) / denom);
			stats.weight += sub.mass;
		}
	}
}

float Propagator::correlation(int a, int b) const
{
	if (a == b)
	{
		return 1.f;
	}

	auto it = _pooled.find(pair_key(a, b));
	if (it == _pooled.end() || !(it->second.weight > 0))
	{
		return 0.f;
	}

	// shrinkWeight is in units of an average subnetwork's mass
	double mean_mass = (_count > 0 && _totalMass > 0) ?
	_totalMass / (double)_count : 1.0;

	return (float)(it->second.corr /
	               (it->second.weight + _settings.shrinkWeight * mean_mass));
}

Eigen::VectorXf Propagator::correct(const Subnetwork &sub,
                                    const Eigen::VectorXf &theta,
                                    std::vector<Diagnostic> *diagnostics)
const
{
	int k = (int)sub.ids.size();
	Eigen::VectorXf result = sub.fraction;

	if (diagnostics)
	{
		diagnostics->assign(k, Diagnostic());
	}

	if (k < 2 || sub.X.rows() == 0 || !(sub.p.sum() > 0))
	{
		return result;
	}

	Eigen::VectorXf sd = local_sd(sub, normalised_weights(sub));

	Eigen::VectorXf d(k);
	for (int i = 0; i < k; i++)
	{
		int id = sub.ids[i];
		float t = (id >= 0 && id < theta.size()) ? theta(id) : result(i);
		d(i) = std::isfinite(t) ? t - result(i) : 0.f;
	}

	// thresholded pooled correlations among atoms that vary here, and the
	// groups they connect (union-find): each group is solved on its own,
	// which is what keeps this cheap - most atoms are only correlated with
	// a handful of others.
	const float sd_min = 1e-4f;
	std::vector<int> parent(k);
	std::iota(parent.begin(), parent.end(), 0);
	auto find = [&](int x)
	{
		while (parent[x] != x)
		{
			parent[x] = parent[parent[x]];
			x = parent[x];
		}
		return x;
	};

	Eigen::MatrixXf R = Eigen::MatrixXf::Identity(k, k);
	for (int i = 0; i < k; i++)
	{
		for (int j = i + 1; j < k; j++)
		{
			if (sd(i) < sd_min || sd(j) < sd_min)
			{
				continue;
			}

			float r = correlation(sub.ids[i], sub.ids[j]);

			if (diagnostics)
			{
				float a = std::fabs(r);
				(*diagnostics)[i].maxCorrelation =
				std::max((*diagnostics)[i].maxCorrelation, a);
				(*diagnostics)[j].maxCorrelation =
				std::max((*diagnostics)[j].maxCorrelation, a);
			}

			if (std::fabs(r) < _settings.minCorrelation)
			{
				continue;
			}

			R(i, j) = R(j, i) = r;
			parent[find(i)] = find(j);
		}
	}

	std::vector<std::vector<int>> groups(k);
	for (int i = 0; i < k; i++)
	{
		if (sd(i) >= sd_min)
		{
			groups[find(i)].push_back(i);
		}
	}

	for (const std::vector<int> &group : groups)
	{
		int n = (int)group.size();
		if (n < 2)
		{
			continue;
		}

		Eigen::MatrixXf Rb(n, n);
		for (int a = 0; a < n; a++)
		{
			for (int b = 0; b < n; b++)
			{
				Rb(a, b) = R(group[a], group[b]);
			}
		}

		// repair: raise the eigenvalues to a floor, then restore the unit
		// diagonal. Without this, an indefinite matrix has near-zero (or
		// negative) eigenvalues whose inverse is enormous.
		Eigen::SelfAdjointEigenSolver<Eigen::MatrixXf> solver(Rb);
		Eigen::VectorXf values = solver.eigenvalues();
		for (int a = 0; a < n; a++)
		{
			values(a) = std::max(values(a), _settings.eigenFloor);
		}

		Eigen::MatrixXf fixed = solver.eigenvectors() * values.asDiagonal() *
		solver.eigenvectors().transpose();

		Eigen::MatrixXf K(n, n);
		for (int a = 0; a < n; a++)
		{
			for (int b = 0; b < n; b++)
			{
				float scale = std::sqrt(fixed(a, a) * fixed(b, b));
				float corr = fixed(a, b) / scale;
				K(a, b) = corr * sd(group[a]) * sd(group[b]);
			}
		}

		for (int a = 0; a < n; a++)
		{
			K(a, a) *= (1.f + _settings.ridge);
		}

		Eigen::MatrixXf P = K.ldlt().solve(Eigen::MatrixXf::Identity(n, n));

		Eigen::VectorXf db(n);
		for (int a = 0; a < n; a++)
		{
			db(a) = d(group[a]);
		}

		// regressing atom i on all the others has slopes -P_ij / P_ii, so
		// sum_{j != i} beta_ij d_j = d_i - (P d)_i / P_ii for every atom i
		// from the one inversion.
		Eigen::VectorXf g = P * db;

		for (int a = 0; a < n; a++)
		{
			int i = group[a];

			float biggest = 0;
			int useful = 0;
			for (int b = 0; b < n; b++)
			{
				if (b == a || R(i, group[b]) == 0.f)
				{
					continue;
				}

				float dev = std::fabs(db(b));
				biggest = std::max(biggest, dev);
				if (dev > 0.02f)
				{
					useful++;
				}
			}

			float delta = db(a) - g(a) / P(a, a);
			if (!std::isfinite(delta))
			{
				delta = 0;
			}

			bool capped = false;
			if (std::fabs(delta) > biggest)
			{
				delta = (delta > 0 ? biggest : -biggest);
				capped = true;
			}

			result(i) = std::min(1.f, std::max(0.f, result(i) + delta));

			if (diagnostics)
			{
				Diagnostic &diag = (*diagnostics)[i];
				diag.maxDeviation = biggest;
				diag.usefulReferences = useful;
				diag.capped = capped;

				for (int b = 0; b < n; b++)
				{
					if (b == a || R(i, group[b]) == 0.f)
					{
						continue;
					}

					Diagnostic::Reference ref;
					ref.index = group[b];
					ref.correlation = R(i, group[b]);
					ref.slope = -P(a, b) / P(a, a);
					ref.deviation = db(b);
					diag.topReferences.push_back(ref);
				}

				std::sort(diag.topReferences.begin(), diag.topReferences.end(),
				          [](const Diagnostic::Reference &x,
				             const Diagnostic::Reference &y)
				          {
					          return std::fabs(x.slope * x.deviation) >
					          std::fabs(y.slope * y.deviation);
				          });
				if (diag.topReferences.size() > 3)
				{
					diag.topReferences.resize(3);
				}
			}
		}
	}

	return result;
}

std::vector<Eigen::VectorXf>
correct(const std::vector<Subnetwork> &subs, const Eigen::VectorXf &theta,
        const Settings &settings)
{
	Propagator propagator(settings);
	for (const Subnetwork &sub : subs)
	{
		propagator.accumulate(sub);
	}

	std::vector<Eigen::VectorXf> corrected;
	corrected.reserve(subs.size());
	for (const Subnetwork &sub : subs)
	{
		corrected.push_back(propagator.correct(sub, theta));
	}

	return corrected;
}

}
