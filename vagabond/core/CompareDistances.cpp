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

#include "CompareDistances.h"
#include "Atom.h"
#define FIXED_MULTIPLY (1000000.f)

bool acceptable(Atom *const &atom)
{
	return atom->isReporterAtom();
}

CompareDistances::~CompareDistances()
{
	_counts.free();
}

CompareDistances::CompareDistances(bool magnitude)
{
	_defaultFilter = &acceptable;
	_magnitude = magnitude;
}

bool resi_num_comp(const Atom *a, const Atom *b)
{
	return (a->residueId().as_num() < b->residueId().as_num());
}

void CompareDistances::equalFilter(const AtomPosList &apl)
{
	if (_leftAtoms.size() > 0) { return; }
	
	_leftAtoms.clear(); 
	_leftIdxs.clear();

	int i = -1;
	for (const AtomWithPos &awp : apl)
	{
		i++;

		if (!_left || _left(awp.atom))
		{
			_leftAtoms.push_back(awp.atom);
			_leftIdxs.push_back(i);
		}
	}

	std::sort(_leftAtoms.begin(), _leftAtoms.end(), resi_num_comp);
}

void CompareDistances::filter(const AtomPosList &apl)
{
	equalFilter(apl);
}

void CompareDistances::setupMatrix()
{
	if (_set)
	{
		return;
	}

	std::unique_lock<std::mutex> lock(_setupLock);

	while (_setSignal <= 0)
	{
		_cv.wait(lock);
	}
	
	_setSignal--;

	if (!_counts.set())
	{
		_counts.setup(_leftAtoms.size());
	}
	
	_set = true;
	
	_cv.notify_all();
}

void CompareDistances::process(const AtomPosList &apl)
{
	filter(apl);
	setupMatrix();
	addToMatrix(apl);
}

float CompareDistances::quickScore()
{
	float sum = 0;
	int size = _counts.n() * _counts.n();
	
	if (size == 0)
	{
		return 0;
	}

	for (int i = 0; i < size; i++)
	{
		float add = _counts.vals[i] / FIXED_MULTIPLY;
		sum += add * add;
	}

	return sqrt(sum / (float)(size * _counter));
}

void CompareDistances::addEqualToMatrix(const AtomPosList &apl)
{
	_counter++;
	
	for (int i = 0; i < (int)_leftIdxs.size() - 1; i++)
	{
		const int &m = _leftIdxs[i];
		const glm::vec3 &x = apl[m].wp.ave;
		const glm::vec3 &p = apl[m].wp.target;

		for (int j = i + 1; j < _leftIdxs.size(); j++)
		{
			const int &n = _leftIdxs[j];
			if (abs(i - j) <= _minimum || abs(i - j) >= _maximum)
			{
				continue;
			}
			const glm::vec3 &y = apl[n].wp.ave;
			const glm::vec3 &q = apl[n].wp.target;

			float expected = glm::length(p - q);
			float acquired = glm::length(x - y);
			
			float diff = expected - acquired;
			float add = _magnitude ? fabs(diff) : diff;
			if (add == add)
			{
				_counts[i][j] += add * FIXED_MULTIPLY;
				_counts[j][i] += add * FIXED_MULTIPLY;
			}
		}
	}
}

void CompareDistances::addToMatrix(const AtomPosList &apl)
{
	addEqualToMatrix(apl);
}

void CompareDistances::reset()
{
	std::unique_lock<std::mutex> lock(_setupLock);
	_set = false;
	_setSignal = 1;
	_leftAtoms.clear();
	_counts.free();
	_counter = 0;
}

PCA::Matrix CompareDistances::matrix()
{
	PCA::Matrix copy;
	
	PCA::setupMatrix(&copy, _counts.n(), _counts.n());
	
	for (size_t i = 0; i < _counts.n() * _counts.n(); i++)
	{
		copy.vals[i] = _counts.vals[i] / FIXED_MULTIPLY;
		copy.vals[i] /= (float)_counter;
	}

	return copy;
}

void CompareDistances::clearMatrix()
{
	_counts.zero();
	_counter = 0;
}
