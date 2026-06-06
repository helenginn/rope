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

#include "Correlative.h"
#include "ProbeResult.h"
#include "CertainStates.h"

using namespace Eigen;

Correlative::Correlative(const OpSet<ProbeTypePair> &all, float ave_score) 
: _probes(all), _ave_score(ave_score)
{
	size_t accumulative = 0;
	for (const ProbeTypePair &ptp : all)
	{
		int dim = dim_for_type(ptp.second);
		_insertions[ptp] = {accumulative, dim};
		for (int i = accumulative; i <= accumulative + dim; i++)
		{
			_lookup[i] = ptp;
		}
		accumulative += dim;
	}
	std::cout << "Lookup: " << _lookup.size() << std::endl;
	_size = accumulative;
	
	auto lookup = _lookup;
	_matrixLookup = [lookup, accumulative](float x, float y) -> std::string
	{
		int xi = x * accumulative;
		int yi = -y * accumulative;

		if (lookup.count(xi) == 0)
		{
			return "";
		}
		if (lookup.count(yi) == 0)
		{
			return "";
		}
		const ProbeTypePair &ptpx = lookup.at(xi);
		const ProbeTypePair &ptpy = lookup.at(yi);
		std::string xstr = ptpx.first->desc();
		std::string ystr = ptpy.first->desc();
		std::string info = xstr + " / " + ystr;
		return info;
	};

	_overall = MatrixXf(_size, _size);
	_written = MatrixXf(_size, _size);
	_overall.setZero();
	_written.setZero();
}

void Correlative::addStates(const CertainStates &states)
{
	for (const ProbeTypePair &left : states.ptps())
	{
		int x = _insertions[left].first;
		int m = _insertions[left].second;

		for (const ProbeTypePair &right : states.ptps())
		{
			int y = _insertions[right].first;
			int n = _insertions[right].second;
			float ave = states.average_score();
			ProbeCorrelation c = states.correlate(left, right, _ave_score);

			float w = states.state_count();
			Eigen::MatrixXf cc = c.mat * w;
			Eigen::MatrixXf csq = c.mat;

			for (int j = 0; j < cc.cols(); j++)
			{
				for (int i = 0; i < cc.rows(); i++)
				{
					cc(i, j) = cc(i, j);
					csq(i, j) = w;
				}
			}

			_overall(seqN(x, m), seqN(y, n)) += cc;
			_written(seqN(x, m), seqN(y, n)) += csq;

			_overall(seqN(y, n), seqN(x, m)) += cc.transpose();
			_written(seqN(y, n), seqN(x, m)) += csq.transpose();
		}
	}
}

Eigen::MatrixXf Correlative::acquireMatrix()
{
	for (int i = 0; i < _overall.rows(); i++)
	{
		for (int j = 0; j < _overall.cols(); j++)
		{
			if (_written(i, j) > 1e-6)
			{
				_overall(i, j) /= _written(i, j);
			}
		}
	}
	
	return _overall;
}
