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

#include "FloydWarshall.h"
#include <iostream>
#include <vector>

FloydWarshall::FloydWarshall(Eigen::MatrixXf &sqMat, const CombineWeight &cw, 
                             bool maximise) 
: _sqMat(sqMat), _combineWeight(cw), _maximise(maximise)
{
	

}

void FloydWarshall::run() // symmetric matrix
{
	int size = _sqMat.rows();
	int update = (size / 5.f);
	if (update <= 10)
	{
		update = 10;
	}
	
	auto find_best = [&](float x, float y)
	{
		if (fabs(x) > fabs(y))
		{
			return _maximise ? x : y;
		}
		else
		{
			return _maximise ? y : x;
		}
	};

	auto refresh_display = [&]()
	{
		if (_mat && _mutex)
		{
			// only synced right before an actual GUI refresh, not on
			// every (i, j, k) triple - one O(n^2) copy instead of doing
			// the same n^2 writes redundantly on every one of the n
			// k-iterations in between refreshes.
			std::unique_lock<std::mutex> lock(*_mutex);
			*_mat = _sqMat;
		}
		_update();
	};

	// _combineWeight is pluggable in principle, but its only current
	// caller (ViewCorrelations.cpp) is plain multiplication - under
	// that (and any other "zero absorbs" combine function), a
	// near-zero operand guarantees a near-zero candidate regardless of
	// the other operand, so those (i, j, k) triples can be skipped
	// outright instead of computing and comparing them. If a future
	// caller ever passes a combine function without this property
	// (e.g. addition, where x + 0 = x), this skip would silently drop
	// real updates - worth revisiting this assumption if that happens.
	const float epsilon = 1e-6f;

	for (int k = 0; k < size; k++)
	{
		// row k and column k are never written to during this k-iteration
		// (the i == k / j == k skip below guarantees it), so snapshot
		// them once into contiguous buffers instead of re-reading them
		// out of _sqMat on every (i, j) pair below - row k in particular
		// is a strided read otherwise, since _sqMat is column-major.
		Eigen::VectorXf colK = _sqMat.col(k);
		Eigen::VectorXf rowK = _sqMat.row(k);

		// cached once per k-iteration and reused across every j below,
		// instead of re-scanning colK's zero/non-zero pattern from
		// scratch (or worse, paying for a combineWeight() call and a
		// comparison) for every single (i, j) pair - most entries are
		// ~0 for the correlation matrices this is actually run on, so
		// this list ends up far shorter than `size`.
		std::vector<int> nonzeroI;
		nonzeroI.reserve(size);
		for (int i = 0; i < size; i++)
		{
			if (i != k && fabs(colK(i)) > epsilon)
			{
				nonzeroI.push_back(i);
			}
		}

		for (int j = 0; j < size; j++)
		{
			if (k == j)
			{
				continue;
			}

			float kj = rowK(j);

			if (fabs(kj) < epsilon)
			{
				// every candidate for this j would combine to ~0 (see
				// the assumption noted above) and could never beat a
				// non-negligible current, so the entire inner loop
				// below is a no-op - skip straight to the next j.
				continue;
			}

			// i innermost to match _sqMat's column-major storage -
			// _sqMat(i, j) for fixed j, increasing i, is a contiguous
			// scan rather than a strided one.
			for (int i : nonzeroI)
			{
				if (i == j)
				{
					continue;
				}

				float current = _sqMat(i, j);
				float candidate = _combineWeight(colK(i), kj);

				float best = find_best(current, candidate);
				_sqMat(i, j) = best;
			}
		}

		if (_update && (k % update == 0))
		{
			refresh_display();
		}

		if (_tick)
		{
			_tick();
		}
	}

	if (_update)
	{
		refresh_display();
	}

	if (_done)
	{
		_done();
	}
}
