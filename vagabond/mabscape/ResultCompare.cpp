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

#include "ResultCompare.h"
#include <map>

float compareResults(const PosToComp::Result &left,
                     const PosToComp::Result &right,
                     float radius)
{
	std::map<std::string, const Positions::Placement *> lookup;
	for (const Positions::Placement &pl : right.placements)
	{
		lookup[pl.name] = &pl;
	}

	// fixed antibodies sit in the same place in every result, so they would
	// only inflate the agreement
	float sum = 0;
	int count = 0;

	for (const Positions::Placement &pl : left.placements)
	{
		if (pl.fixed || pl.copies.empty() || !lookup.count(pl.name) ||
		    Positions::ignored(pl.name))
		{
			continue;
		}

		const Positions::Placement &other = *lookup.at(pl.name);

		float nearest = FLT_MAX;
		for (const glm::vec3 &copy : other.copies)
		{
			nearest = std::min(nearest, glm::length(copy - pl.copies[0]));
		}

		if (nearest < FLT_MAX)
		{
			sum += PosToComp::closeness(nearest, radius);
			count++;
		}
	}

	return (count ? sum / count : 0.f);
}

VectorModel buildVectorModel(const std::vector<PosToComp::Result> &results)
{
	VectorModel model;

	if (results.empty())
	{
		return model;
	}

	std::vector<const std::vector<Positions::Placement> *> all;
	for (const PosToComp::Result &r : results)
	{
		all.push_back(&r.placements);
	}

	for (const Positions::Placement &first : *all[0])
	{
		Positions::Clusters clusters;
		if (!Positions::cluster(all, first.name, clusters))
		{
			continue;
		}

		size_t n = clusters.centres.size();
		std::vector<std::vector<glm::vec3>> sums(results.size(), 
		std::vector<glm::vec3>(n, glm::vec3(0.f)));
		std::vector<std::vector<int>> counts(results.size(), 
		std::vector<int>(n, 0));

		for (size_t p = 0; p < clusters.points.size(); p++)
		{
			size_t r = clusters.resultOf[p];
			size_t k = clusters.member[p];
			sums[r][k] += clusters.points[p] - clusters.centres[k];
			counts[r][k]++;
		}

		// should a result put two copies in one cluster, their mean
		for (size_t r = 0; r < results.size(); r++)
		{
			for (size_t k = 0; k < n; k++)
			{
				if (counts[r][k] > 1)
				{
					sums[r][k] /= (float)counts[r][k];
				}
			}
		}

		model.vectors.push_back(sums);
	}

	return model;
}

float compareByVectors(const VectorModel &model, size_t left, size_t right)
{
	double total = 0;

	for (size_t a = 0; a < model.vectors.size(); a++)
	{
		const std::vector<glm::vec3> &l = model.vectors[a][left];
		const std::vector<glm::vec3> &r = model.vectors[a][right];

		// the dot product over the product of the lengths: the cosine of the
		// angle between the two pulls, whatever their size. A pull of nothing
		// has no direction to agree with, so it scores 0.
		double dots = 0;
		for (size_t k = 0; k < l.size(); k++)
		{
			float lengths = glm::length(l[k]) * glm::length(r[k]);
			if (lengths > 1e-12f)
			{
				dots += glm::dot(l[k], r[k]) / lengths;
			}
		}

		total += dots / l.size();
	}

	return (model.vectors.size() ? total / model.vectors.size() : 0.f);
}
