// helencore
// Copyright (C) 2019 Helen Ginn
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

#ifndef __helencore__Canonical__
#define __helencore__Canonical__

#include <vector>
#include <functional>
#include "Eigen/Dense"

// per-group preparation: builds a single group's sample matrix and its
// (thin) SVD once. A caller comparing every pair among a set of groups
// (e.g. CommunicationAnalysis, comparing every pair of signals) should
// build one of these per group and reuse them across every pair that
// group appears in, rather than rebuilding identical data/SVDs once per
// pair - see Canonical below, which only ever combines two already
// prepared groups.
class CanonicalGroup
{
public:
	CanonicalGroup(int dim);

	void sizeHint(int n);
	void addVec(const std::vector<double> &vec);

	// builds the sample matrix and its SVD, and works out how many of
	// its singular values are actually significant - the expensive
	// part, done once per group rather than once per pair.
	void prepare();

	int dim() const { return _dim; }
	int significantDim() const { return _sigDim; }
	int sampleCount() const { return _cc.rows(); }

	const Eigen::MatrixXf &cc() const { return _cc; }
	const Eigen::MatrixXf &u() const { return _u; }
	const Eigen::MatrixXf &v() const { return _v; }
	const Eigen::VectorXf &singularValues() const { return _singularValues; }

private:
	int _dim;
	int _sigDim = 0;
	std::vector<double> _vecs;
	int _nSamples = 0;

	Eigen::MatrixXf _cc;
	Eigen::MatrixXf _u, _v;
	Eigen::VectorXf _singularValues;
};

class Canonical
{
public:
	// m and n must already be prepare()d (see CanonicalGroup) - kept as
	// references, not owned, since the same prepared group is meant to
	// be reused across every pair it appears in, not rebuilt per pair.
	Canonical(const CanonicalGroup &m, const CanonicalGroup &n);

	typedef std::function<float(const int &idx)> GetWeight;
	void run();
	double correlation();

	Eigen::MatrixXf &u()
	{
		return _uDisplay;
	}

	Eigen::MatrixXf &v()
	{
		return _vDisplay;
	}

	void addWeights(const GetWeight &get_weight)
	{
		_getWeight = get_weight;
	}
private:
	const CanonicalGroup &_m;
	const CanonicalGroup &_n;

	Eigen::MatrixXf _u, _v;
	Eigen::MatrixXf _uDisplay, _vDisplay;
	GetWeight _getWeight;
};

#endif
