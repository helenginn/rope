// vagabond
// Copyright (C) 2026 Helen Ginn
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

#include "OccupancyPredictor.h"
#include "Clique.h"
#include "ProbeResult.h"
#include "CertainStates.h"

OccupancyPredictor::OccupancyPredictor(Clique *clique) : _clique(clique)
{

}

OccupancyPredictor::EstimateMap OccupancyPredictor::estimates()
{
	struct OccupancyEstimate // one per clique
	{
		std::map<int, float> results{}; // one per state
		float sum = 0;
		size_t samples = 0;

		// the originating Clique's own sampleWeight() (see its own
		// comment) - how many independent Subdivide::subdivide() walks
		// collapsed into this one searched subdivision, so it counts
		// proportionally more towards the pooled estimate below than a
		// subdivision only a single walk ever reached.
		float sampleWeight = 1.f;
	};

	std::map<ProbeTypePair, std::vector<OccupancyEstimate>> occupancies;
	EstimateMap ret;

	auto process_clique = [&occupancies](const Clique &clique)
	{
		if (!clique.states()) return;
		const CertainStates &states = *clique.states();

		// computed once per clique, not per ptp - see proportions()'s own
		// comment for why (score() is a live callback, and every ptp in
		// this loop shares the same ave). probsForLocalAve() also avoids
		// calling score(i) a second time just to get ave, unlike the
		// average_score() + probsForAve(ave) pair this used to be.
		float ave = 0;
		std::vector<float> probs = states.probsForLocalAve(ave);

		for (const ProbeTypePair &ptp : states.ptps())
		{
			// the node-sample-count correction (see
			// CertainStates::sampleCount()) is applied inside proportions()
			// itself, not here - sampleWeight is the separate, independent
			// subdivision-merge correction (Clique::sampleWeight()).
			float sum = 0; // sum of all energy contributions, populated next
			std::map<int, float> occs = states.proportions(ptp, sum, probs);
			float sampleWeight = (float)clique.sampleWeight();

			occupancies[ptp].push_back({occs, sum, states.state_count(),
			                            sampleWeight});
		}
	};

	for (Clique &clique : _clique->subdivisions())
	{
		process_clique(clique);
	}

	for (const auto &occs : occupancies)
	{
		const ProbeTypePair &ptp = occs.first;
		if (ptp.second != hnet::Types::ExistenceType)
		{
			continue; // skip for now
		}

		const std::vector<OccupancyEstimate> &estimates = occs.second;
		std::map<int, float> sums;
		float grand_sum = 0;
		size_t sample_count = 0;

		for (const int &state : {1, 2})
		{
			float sum = 0; float weights = 0;
			for (const OccupancyEstimate &est : estimates) // one per state
			{
				if (est.results.count(state) == 0)
				{
					continue;
				}

				float weight = est.sum * est.sampleWeight;
				float quantity = est.results.at(state);
				sample_count += est.samples;
				sum += quantity * weight;
				weights += weight;
			}
			if (sum != sum)
			{
				sum = 0;
			}
			sum /= weights;
			sums[state] = sum;
			grand_sum += sum;
		}

		for (auto &s : sums)
		{
			s.second /= grand_sum;
		}

		if (!ptp.first->atom() || ptp.first->atom()->symmetryCopyOf()
		    || ptp.first->is_bulk())
		{
			continue;
		}

		Atom *atom = ptp.first->atom();

		if (atom->bondLengthCount() && atom->occupancy_sum() < 0.9)
		{
			continue;
		}

		if (!ptp.first->isActiveAtom())
		{
			continue;
		}

		float calculated = sums[2] / (sums[1] + sums[2]);
		if (calculated != calculated)
		{
			calculated = 0;
			ret.erase(ptp);
			continue;
		}
		float observed = ptp.first->atomConf().occupancy();

		ret[ptp] = {calculated, observed, grand_sum, sample_count};
	}

	return ret;
}
