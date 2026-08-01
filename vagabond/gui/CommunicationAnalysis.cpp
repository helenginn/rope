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

#include "CommunicationAnalysis.h"
#include <vagabond/gui/elements/list/LineGroup.h>
#include <vagabond/gui/elements/list/ItemLine.h>
#include <vagabond/gui/elements/ScrollBox.h>
#include <vagabond/gui/elements/Window.h>
#include <vagabond/gui/MatrixPlot.h>
#include <vagabond/gui/MatrixBox.h>
#include <vagabond/core/protonic/Clique.h>
#include <vagabond/core/protonic/ProbeResult.h>
#include <vagabond/utils/FileReader.h>
#include <vagabond/utils/Canonical.h>
#include <cmath>

using Eigen::seqN;
using Eigen::seq;

CommunicationAnalysis::CommunicationAnalysis(Scene *scene, Clique *clique,
                                             const MatrixXf &mat,
                                             const std::map<ProbeTypePair, 
                                             std::pair<int, int>> &insertions)
: Scene(scene), _clique(clique), _mat(mat), _lookup(insertions)
{

}

CommunicationAnalysis::~CommunicationAnalysis()
{

}

void CommunicationAnalysis::prepareGroups()
{
	OpSet<std::string> names = _clique->allCommsNames();
	_parent.setDisplayName("Groups");
	for (const std::string &name : names)
	{
		Item *item = new Item();

		auto job = [this, item, name](bool left)
		{
			if (!left)
			{
				return;
			}
			if (_chosen.count(name))
			{
				_chosen -= name;
				_lg->display(item)->setColour(0.0, 0.0, 0.0);
			}
			else
			{
				_chosen += name;
				_lg->display(item)->setColour(0.4, 0.0, 0.4);
			}
		};

		item->setDisplayName(name);
		item->setSelectable(true);
		item->setSelectJob(job);
		_parent.addItem(item);
	}

	LineGroup *lg = new LineGroup(&_parent, this);
	lg->setLeft(0.1, 0.2);
	_lg = lg;

	ScrollBox *sb = new ScrollBox();
	sb->setContent(lg);
	sb->setBounds(glm::vec4(0.15, 0.0, 0.9, 0.35));
	addObject(sb);

}

std::optional<CanonicalGroup>
CommunicationAnalysis::prepareGroup(const std::string &name)
{
	auto check_pair = [this, &name](const ProbeTypePair &ptp,
	                                OpSet<ProbeTypePair> &which)
	{
		const OpSet<std::string> &descs = _clique->nodeDescsForGroup(name);

		if (descs.count(ptp.first->desc()))
		{
			if (ptp.first->is_bond() && ptp.second == hnet::Types::BondType)
			{
				which += ptp;
			}
			else if (ptp.first->is_atom() && ptp.second ==
			         hnet::Types::ExistenceType)
			{
				which += ptp;
			}
		}
	};

	OpSet<ProbeTypePair> members{};

	for (auto it = _lookup.begin(); it != _lookup.end(); it++)
	{
		check_pair(it->first, members);
	}

	int dim = 0;
	for (const ProbeTypePair &ptp : members)
	{
		dim += _lookup[ptp].second;
	}

	// matches the old ln == 0 / rn == 0 short-circuit in compare() -
	// no matching probes for this signal, so any pair involving it
	// must be skipped rather than attempting to build/prepare a group.
	if (dim == 0)
	{
		return std::nullopt;
	}

	MatrixXf mat(_overOne, dim);
	mat.setZero();

	int inc = 0;
	for (const ProbeTypePair &ptp : members)
	{
		int ins = _lookup[ptp].first;
		int pdim = _lookup[ptp].second;

		MatrixXf tmp = _wU(Eigen::all, seqN(ins, pdim));
		mat(Eigen::all, seqN(inc, pdim)) = tmp;

		inc += pdim;
	}

	CanonicalGroup group(dim);
	group.sizeHint(_overOne);

	std::vector<double> vec(dim);
	for (int r = 0; r < mat.rows(); r++)
	{
		for (int c = 0; c < dim; c++)
		{
			vec[c] = mat(r, c);
		}
		group.addVec(vec);
	}

	group.prepare();
	return group;
}

float CommunicationAnalysis::compare(const std::optional<CanonicalGroup> &m,
                                     const std::optional<CanonicalGroup> &n,
                                     const std::string &first,
                                     const std::string &second)
{
	if (!m || !n)
	{
		_emptyPairs++;
		return 0;
	}
	_ranPairs++;

	Canonical cc(*m, *n);

	auto get_weight = [this](const int &idx)
	{
		return _w[idx];
	};
	cc.addWeights(get_weight);

	float result = 0;
	try
	{
		cc.run();
		result = fabs(cc.correlation());
	}
	catch (int e)
	{
		std::cout << "CommunicationAnalysis::compare(" << first << ", "
		          << second << "): Canonical::run() threw - bailing out, "
		          << "result forced to 0" << std::endl;
	}
	return result;
}

void CommunicationAnalysis::svd()
{
	Eigen::BDCSVD<MatrixXf> svd(_mat, Eigen::ComputeFullU | 
	                            Eigen::ComputeFullV);
	Eigen::MatrixXf u = svd.matrixU().transpose();
	Eigen::VectorXf w = svd.singularValues();
	for (float &f : w)
	{
		f = sqrt(f);
	}
	_w = w;
	_wU = u;
	
	int n = 0;
	for (const float &val : w)
	{
		_wU.row(n) *= val;
		n++;
	}

	n = w.size();
	
	/*
	PCA::Matrix mat = PCA::Matrix(_wU);
	MatrixPlot *mp = new MatrixPlot(mat);
	mp->setCentre(0.75, 0.25);
	addObject(mp);
	*/

	_overOne = n;
}

void CommunicationAnalysis::setup()
{
	addTitle("Communication Analysis");
	
	svd();

	// a signal whose probe never made it into the assembled correlation
	// matrix (_lookup) - e.g. filtered out upstream, or from a residue
	// this clique's subdivisions never actually covered - would otherwise
	// contribute an all-zero row/column (compare() short-circuits to 0
	// whenever it finds nothing to correlate against), so it's dropped
	// before the matrix is even built rather than just left empty.
	auto has_usable_probe = [this](const std::string &name)
	{
		const OpSet<std::string> &descs = _clique->nodeDescsForGroup(name);

		for (auto it = _lookup.begin(); it != _lookup.end(); it++)
		{
			const ProbeTypePair &ptp = it->first;
			if (!descs.count(ptp.first->desc()))
			{
				continue;
			}

			bool bondOk = ptp.first->is_bond() &&
			ptp.second == hnet::Types::BondType;
			bool atomOk = ptp.first->is_atom() &&
			ptp.second == hnet::Types::ExistenceType;
			if (bondOk || atomOk)
			{
				return true;
			}
		}

		return false;
	};

	std::vector<std::string> names;
	int excluded = 0;

	for (const std::string &name : _clique->allCommsNames())
	{
		if (has_usable_probe(name))
		{
			names.push_back(name);
		}
		else
		{
			excluded++;
		}
	}

	if (excluded)
	{
		std::cout << "CommunicationAnalysis::setup(): excluded " << excluded
		          << " signal(s) with no probe found in the matrix"
		          << std::endl;
	}

	Eigen::MatrixXf mat(names.size(), names.size());
	mat.setZero();

	// used to live inside the old per-pair compare() (called O(names^2)
	// times, clearing nothing new each time since compare() never adds
	// temp objects of its own) - moved out here to run once instead.
	deleteTemps();

	// prepared once per signal and reused across every pair it appears
	// in, instead of being rebuilt (and re-SVD'd) once per pair - see
	// prepareGroup()'s own comment for why this is safe (a group's
	// preparation depends only on its own name, never on which other
	// signal it is being compared against).
	std::vector<std::optional<CanonicalGroup>> groups;
	groups.reserve(names.size());
	for (const std::string &name : names)
	{
		groups.push_back(prepareGroup(name));
	}

	// compare(A, B) and compare(B, A) are the same canonical correlation
	// (symmetric in the two groups being compared), so only the upper
	// triangle (including the diagonal) is actually computed - the
	// lower triangle is filled by mirroring instead of repeating the
	// work.
	for (size_t m = 0; m < names.size(); m++)
	{
		for (size_t n = m; n < names.size(); n++)
		{
			float r = compare(groups[m], groups[n], names[m], names[n]);
			mat(m, n) = r;
			if (n != m)
			{
				mat(n, m) = r;
			}
		}
	}

	{
		int n = names.size();
		int diagBad = 0;
		int offDiag = 0;
		int nanOrInf = 0;
		int above01 = 0, above03 = 0, above05 = 0;
		float offMin = 1e9, offMax = -1e9, offSum = 0;

		for (int i = 0; i < n; i++)
		{
			for (int j = 0; j < n; j++)
			{
				float v = mat(i, j);

				if (v != v || std::isinf(v))
				{
					nanOrInf++;
					continue;
				}

				if (i == j)
				{
					if (fabs(v - 1.f) > 1e-2)
					{
						diagBad++;
					}
					continue;
				}

				offDiag++;
				offSum += v;
				offMin = std::min(offMin, v);
				offMax = std::max(offMax, v);
				if (v > 0.1) above01++;
				if (v > 0.3) above03++;
				if (v > 0.5) above05++;
			}
		}

		std::cout << "CommunicationAnalysis::setup(): " << n << "x" << n
		          << " groups (" << (n * n) << " pairs), "
		          << diagBad << " self-pairs not ~1, "
		          << nanOrInf << " NaN/Inf entries" << std::endl;
		std::cout << "CommunicationAnalysis::setup(): " << _ranPairs
		          << " pairs reached Canonical::run(), " << _emptyPairs
		          << " short-circuited before it (ln==0 or rn==0 in "
		          << "compare() - no matching probes in one/both groups)"
		          << std::endl;
		std::cout << "CommunicationAnalysis::setup(): off-diagonal ("
		          << offDiag << " pairs): min=" << offMin
		          << " max=" << offMax
		          << " mean=" << (offDiag ? offSum / offDiag : 0)
		          << " | >0.1: " << above01
		          << " >0.3: " << above03
		          << " >0.5: " << above05 << std::endl;
	}

	Eigen::JacobiSVD<MatrixXf> svd(mat, Eigen::ComputeFullU |
	                               Eigen::ComputeFullV);
	
	_cc = mat;
	MatrixPlot *mp = new MatrixPlot(_cc);
	MatrixBox *mBox = new MatrixBox(mp, names, names, true, this);
	mBox->setCentre(0.5, 0.6);
	mBox->resize(1.2);
	addObject(mBox);
}
