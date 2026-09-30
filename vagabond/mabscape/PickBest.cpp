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

#include "PickBest.h"
#include <algorithm>
#include <cmath>
#include <numeric>

std::vector<size_t> pickBest(const Eigen::MatrixXf &similarity,
                             const std::vector<float> &scores,
                             float fraction)
{
	size_t n = std::min((size_t)similarity.rows(), scores.size());
	std::vector<size_t> chosen;

	if (n == 0)
	{
		return chosen;
	}

	size_t count = (size_t)std::lround(fraction * n);
	count = std::min(n, std::max((size_t)2, count));

	// mark the best-scoring results to begin with
	std::vector<size_t> order(n);
	std::iota(order.begin(), order.end(), 0);
	std::stable_sort(order.begin(), order.end(), [&scores](size_t a, size_t b)
	{
		return scores[a] < scores[b];
	});

	std::vector<bool> marked(n, false);
	for (size_t i = 0; i < count; i++)
	{
		marked[order[i]] = true;
	}

	// how alike a result is to every marked result other than `skip`
	auto similarity_to_set = [&](size_t idx, size_t skip)
	{
		float sum = 0;
		for (size_t j = 0; j < n; j++)
		{
			if (marked[j] && j != idx && j != skip)
			{
				sum += similarity(idx, j);
			}
		}
		return sum;
	};

	const float epsilon = 1e-6;
	bool swapped = true;

	while (swapped && count < n)
	{
		swapped = false;

		// the marked result which is least like the others...
		size_t worst = n;
		float worst_sum = 0;
		for (size_t i = 0; i < n; i++)
		{
			if (!marked[i])
			{
				continue;
			}

			float sum = similarity_to_set(i, n);
			if (worst == n || sum < worst_sum)
			{
				worst = i;
				worst_sum = sum;
			}
		}

		// ...gives way to the unmarked result most like the rest of them
		size_t next = n;
		float next_sum = 0;
		for (size_t i = 0; i < n; i++)
		{
			if (marked[i])
			{
				continue;
			}

			float sum = similarity_to_set(i, worst);
			if (next == n || sum > next_sum)
			{
				next = i;
				next_sum = sum;
			}
		}

		if (worst < n && next < n && next_sum > worst_sum + epsilon)
		{
			marked[worst] = false;
			marked[next] = true;
			swapped = true;
		}
	}

	for (size_t i = 0; i < n; i++)
	{
		if (marked[i])
		{
			chosen.push_back(i);
		}
	}

	return chosen;
}
