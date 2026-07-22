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
#include <vagabond/utils/Eigen/Dense>
#include <vagabond/core/protonic/Energy.h>
#include <map>

class Graph;
class Clique;
class Network;
class Correlative;
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
	void updateEstimates(std::map<ProbeTypePair, OccData> &ests);
	std::map<ProbeTypePair, OccData> estimates();
	Clique *_clique{};
	Correlative *_correlative{};

	std::map<ProbeTypePair, OccData> _estimates;
	std::map<ProbeTypePair, OccData> _first;
	Graph *_graph{};
	Network &_network;
	Eigen::MatrixXf _overall;
};

#endif
