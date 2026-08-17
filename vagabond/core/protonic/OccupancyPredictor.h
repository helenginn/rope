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

#ifndef __vagabond__OccupancyPredictor__
#define __vagabond__OccupancyPredictor__

#include <map>
#include <cstddef>

class Clique;
struct ProbeTypePair;

// predicts atom occupancies from a Clique's own proton-network
// subdivisions (CertainStates::proportions() per subdivision, pooled and
// weighted by Clique::sampleWeight()), for comparison against each atom's
// currently modelled occupancy. Pure science - no GUI/rendering
// dependency. One instance is meant to be kept alive across repeated
// estimates() calls against the same Clique (e.g. OccupanciesView's own
// "Check occupancies" button and its pH-scan sweep), rather than
// reconstructed per call.
class OccupancyPredictor
{
public:
	OccupancyPredictor(Clique *clique);

	struct OccData
	{
		float calculated{};
		float observed{};
		float probs{};
		size_t samples{};
	};

	typedef std::map<ProbeTypePair, OccData> EstimateMap;
	EstimateMap estimates();

	// off by default - not currently judged to help (see its own toggle
	// on OccupanciesView). When on, de-weights subnetworks that overlap
	// heavily with one another (see estimates()' own comment on
	// redundancy_weights()) rather than letting a cluster of
	// near-duplicate subnetworks outweigh a single independent one.
	void setRedundancyWeightingEnabled(bool on)
	{
		_redundancyWeightingEnabled = on;
	}

private:
	Clique *_clique{};
	bool _redundancyWeightingEnabled = false;
};

#endif
