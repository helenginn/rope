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

#include "Clique.h"
#include "Correlative.h"
#include "ProbeResult.h"
#include "CertainStates.h"

using namespace Eigen;

OpSet<ProbeTypePair> Correlative::probeTypePairs
(const std::list<Clique> &cliques, float &all_ave)
{
	OpSet<ProbeTypePair> all;
	float all_sum = 0;
	float all_count = 0;
	for (const Clique &clique : cliques)
	{
		if (!clique.states()) continue;

		const CertainStates &states = *clique.states();
		float sum = states.average_score() * states.state_count();
		if (sum != sum)
		{
			continue;
		}
		all_sum += sum;
		all_count += states.state_count();
		all += states.ptps();
	}

	all_ave = all_sum / all_count;
	return all;
}

Correlative::Correlative(const OpSet<ProbeTypePair> &all, float ave_score,
                         bool relative, bool loggy) 
: _probes(all), _ave_score(ave_score), _relative(relative), _loggy(loggy)
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
		int yi = accumulative * (y);

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
		if (_probes.count(left) == 0)
		{
			continue;
		}
		int x = _insertions[left].first;
		int m = _insertions[left].second;

		for (const ProbeTypePair &right : states.ptps())
		{
			if (_probes.count(right) == 0)
			{
				continue;
			}
			int y = _insertions[right].first;
			int n = _insertions[right].second;
			ProbeCorrelation c = states.correlate(left, right, _ave_score,
			                                      _relative);
			
			if (_loggy)
			{
				c.mat *= (float)states.state_count();
				auto copy_c = c.mat;
				for (int i = 0; i < c.mat.rows(); i++)
				{
					for (int j = 0; j < c.mat.cols(); j++)
					{
						c.mat(i, j) = log(c.mat(i, j));
						float &f = c.mat(i, j);

						if (f != f || !std::isfinite(f))
						{
							f = -10;
						}
					}
				}

				_overall(seqN(x, m), seqN(y, n)) += c.mat;
				_overall(seqN(y, n), seqN(x, m)) += c.mat.transpose();
				
				Eigen::MatrixXf copy = c.mat; copy.setOnes();
//				copy *= states.state_count();

				_written(seqN(x, m), seqN(y, n)) += copy;
				_written(seqN(y, n), seqN(x, m)) += copy.transpose();
			}
			else
			{
				Eigen::MatrixXf cc = c.mat;
				Eigen::MatrixXf csq = c.mat;
				csq.setOnes();

				_overall(seqN(x, m), seqN(y, n)) += cc;
				_written(seqN(x, m), seqN(y, n)) += csq;

//				_overall(seqN(y, n), seqN(x, m)) += cc.transpose();
//				_written(seqN(y, n), seqN(x, m)) += csq.transpose();
			}
		}
	}
}

Eigen::MatrixXf Correlative::acquireMatrix()
{
	Eigen::MatrixXf ret = _overall;

	// get rid of nans
	float sum = 0;
	for (int i = 0; i < ret.rows(); i++)
	{
		for (int j = 0; j < ret.cols(); j++)
		{
			if (ret(i, j) != ret(i, j) || !isfinite(ret(i, j)))
			{
				ret(i, j) = 0;
			}

			sum += fabs(ret(i, j));
		}
	}

	float ave = sum / (float)(ret.cols() * ret.rows());
	float k = log(3) / (ave * 3);
	std::cout << "Sum: " << sum << std::endl;
	std::cout << "k: " << k << std::endl;

	for (int i = 0; i < ret.rows(); i++)
	{
		if (!_loggy)
		{
			for (int j = 0; j < ret.cols(); j++)
			{
				float x = ret(i, j);
				float val = 2 / (1 + exp(-x * k)) - 1;
				ret(i, j) = val;
			}
		}
		
		if (_loggy)
		{
			for (int j = 0; j < ret.cols(); j++)
			{
				ret(i, j) /= _written(i, j);
				ret(i, j) = exp(ret(i, j));
				if (ret(i, j) != ret(i, j))
				{
					ret(i, j) = 0;
				}
			}
		}
	}

	for (int i = 0; i < ret.rows() && _loggy; i++)
	{
		ret.row(i) /= ret(i, i);

		for (int j = 0; j < ret.cols(); j++)
		{
			if (ret(i, j) > 1)
			{
				ret(i, j) = 1;
			}
		}
	}
	
	return ret;
}

Eigen::MatrixXf Correlative::rowsFor(const ProbeTypePair &ptp)
{
	int x = _insertions[ptp].first;
	int m = _insertions[ptp].second;
	
	Eigen::MatrixXf everything;
	if (_cache.rows() > 0)
	{
		everything = _cache;
	}
	else
	{
		everything = acquireMatrix();
		_cache = everything;
	}

	Eigen::MatrixXf cut = everything(seqN(x, m), Eigen::all);
	return cut;
}
