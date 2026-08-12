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

#ifndef __vagabond__OccupanciesView__
#define __vagabond__OccupanciesView__

#include <vagabond/gui/elements/IndexResponseView.h>
#include <vagabond/core/protonic/Energy.h>
#include <unordered_map>

class Graph;
class Clique;
class Network;
struct ProbeTypePair;

class OccupanciesView : public IndexResponseView
{
public:
	OccupanciesView(Scene *prev, Clique *clique, Network &network);

	virtual void setup();
	void occupancies();
	
	struct OccData
	{
		float calculated{};
		float observed{};
		float probs{};
		size_t samples{};
	};

protected:
	virtual void interactedWithNothing(bool left, bool hover = false);
private:
	void slider(std::string msg, const hnet::Energy::Source &src, float y);

	typedef std::map<ProbeTypePair, OccData> EstimateMap;
	EstimateMap estimates();

	// (re)builds _graph from _lastEstimates, applying the current
	// _showWaters/_cliqueOnly filters - shared by occupancies() (which
	// recomputes _lastEstimates first) and the top-left tickboxes' own
	// toggle jobs (which just re-filter the same, already-computed
	// estimates, since neither filter changes what estimates() itself
	// would produce). A no-op if occupancies() has never actually been
	// run yet (_lastEstimates empty) - the tickboxes are pure display
	// filters on top of an existing plot, not their own trigger to run
	// the (potentially slow) full estimation pass.
	void rebuildGraph();

	Clique *_clique{};

	Graph *_graph{};
	Network &_network;

	EstimateMap _lastEstimates;

	// top-left display-filter tickboxes (see setup()/rebuildGraph()).
	bool _showWaters = true;
	bool _cliqueOnly = false;
};

#endif
