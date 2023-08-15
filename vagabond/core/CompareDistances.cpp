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

bool acceptable(Atom *const &atom)
{
	return atom->atomName() == "CA";
}

CompareDistances::~CompareDistances()
{
	freeMatrix(&_matrix);
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

void CompareDistances::filter(const AtomPosList &apl)
{
	if (_leftAtoms.size() > 0 && _rightAtoms.size() > 0) { return; }
	
	_leftAtoms.clear(); _rightAtoms.clear();
	_leftIdxs.clear(); _rightIdxs.clear();

	int i = -1;
	for (const AtomWithPos &awp : apl)
	{
		i++;

		if (!_defaultFilter(awp.atom))
		{
//			continue;
		}
		
		if (!_left || _left(awp.atom))
		{
			_leftAtoms.push_back(awp.atom);
			_leftIdxs.push_back(i);
		}
		if (!_right || _right(awp.atom))
		{
			_rightAtoms.push_back(awp.atom);
			_rightIdxs.push_back(i);
		}
	}

	std::sort(_leftAtoms.begin(), _leftAtoms.end(), resi_num_comp);
	std::sort(_rightAtoms.begin(), _rightAtoms.end(), resi_num_comp);
}

void CompareDistances::setupMatrix()
{
	if (_matrix.rows == 0 || _matrix.cols == 0)
	{
		PCA::setupMatrix(&_matrix, _leftAtoms.size(), _rightAtoms.size());
	}
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
	int size = _matrix.cols * _matrix.rows;
	
	if (size == 0)
	{
		return 0;
	}

	for (int i = 0; i < size; i++)
	{
		sum += _matrix.vals[i] * _matrix.vals[i];
	}

	return sqrt(sum / (float)(size * _counter));
}

void CompareDistances::addToMatrix(const AtomPosList &apl)
{
	int i = 0; int j = 0;
	_counter++;
	
	for (const int &m : _leftIdxs)
	{
		const glm::vec3 &x = apl[m].wp.ave;
		const glm::vec3 &p = apl[m].wp.target;

		for (const int &n : _rightIdxs)
		{
			const glm::vec3 &y = apl[n].wp.ave;
			const glm::vec3 &q = apl[n].wp.target;

			float expected = glm::length(p - q);
			float acquired = glm::length(x - y);
			
			float diff = expected - acquired;
			if (diff == diff && abs(i - j) > 3)
			{
				_matrix[i][j] += _magnitude ? fabs(diff) : diff;
			}
			j++;
		}
		i++;
		j = 0;
	}
}

void CompareDistances::reset()
{
	_leftAtoms.clear(); _rightAtoms.clear();
	freeMatrix(&_matrix);
	_counter = 0;
}

PCA::Matrix CompareDistances::matrix()
{
	PCA::Matrix copy;
	PCA::setupMatrix(&copy, _leftAtoms.size(), _rightAtoms.size());
	copyMatrix(copy, _matrix);
	
	for (size_t i = 0; i < _matrix.rows * _matrix.cols; i++)
	{
		copy.vals[i] /= (float)_counter;
		copy.vals[i] *= 10.f;
	}

	return copy;
}

void CompareDistances::clearMatrix()
{
	if (_matrix.vals)
	{
		zeroMatrix(&_matrix);
	}
	_counter = 0;
}
