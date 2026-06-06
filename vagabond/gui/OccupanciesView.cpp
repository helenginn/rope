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

#include "OccupanciesView.h"
#include <vagabond/utils/maths.h>
#include <vagabond/utils/FileReader.h>
#include <vagabond/gui/GraphView.h>
#include <vagabond/gui/Graph.h>
#include <vagabond/core/protonic/Clique.h>
#include <vagabond/core/protonic/Correlative.h>
#include <vagabond/core/protonic/CertainStates.h>
#include <vagabond/gui/elements/TextButton.h>

OccupanciesView::OccupanciesView(Scene *prev, Clique *clique)
: Scene(prev), _clique(clique)
{

}


void OccupanciesView::setup()
{
	addTitle("Occupancy prediction");

	float all_ave = 0;
	OpSet<ProbeTypePair> all = 
	Correlative::probeTypePairs(_clique->subdivisions(), all_ave);

	_correlative = new Correlative(all, all_ave, false);

	auto process_clique = [this](const Clique &clique)
	{
		if (!clique.states()) return;
		const CertainStates &states = *clique.states();
		_correlative->addStates(states);
	};
	
	for (Clique &clique : _clique->subdivisions())
	{
		process_clique(clique);
	}

	Eigen::MatrixXf overall = _correlative->acquireMatrix();

	TextButton *tb = new TextButton("Check occupancies");
	tb->setReturnJob([this]() { occupancies(); });
	tb->setCentre(0.55, 0.84);
	addObject(tb);
}

void OccupanciesView::occupancies()
{
	struct OccupancyEstimate
	{
		std::map<int, float> results{};
		float sum = 0;
	};
	
	std::map<ProbeTypePair, std::vector<OccupancyEstimate>> occupancies;
	
	auto process_clique = [&occupancies](const Clique &clique)
	{
		if (!clique.states()) return;
		const CertainStates &states = *clique.states();
		float ave = states.average_score();

		for (const ProbeTypePair &ptp : states.ptps())
		{
			float sum = 0;
			std::map<int, float> occs = states.proportions(ptp, sum, ave);
			occupancies[ptp].push_back({occs, sum});
		}
	};
	
	for (Clique &clique : _clique->subdivisions())
	{
		process_clique(clique);
	}
	
	CorrelData cd = empty_CD();
	
	Graph *graph = new Graph();
	graph->style = Graph::StyleScatter;
	graph->setRange('x', 0, 1);
	graph->setRange('y', 0, 1);
	graph->setAxisLabel('x', "Calculated occupancy");
	graph->setAxisLabel('y', "Observed occupancy");
	
	std::cout << "observed, calculated, total_prob, atom" <<  std::endl;

	for (const auto &occs : occupancies)
	{
		const ProbeTypePair &ptp = occs.first;
		if (ptp.second != hnet::Types::ExistenceType)
		{
			continue; // skip for now
		}

		std::string desc = ptp.first->desc();
		desc += (ptp.second == hnet::Types::BondType ?  " bonding" : " exists");

		const std::vector<OccupancyEstimate> &estimates = occs.second;
		std::map<int, float> sums;
		float grand_sum = 0;

		for (const int &state : {1, 2})
		{
			float sum = 0; float weights = 0;
			for (const OccupancyEstimate &est : estimates)
			{
				if (est.results.count(state) == 0)
				{
					continue;
				}

				float weight = est.sum;
				float quantity = est.results.at(state);
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

		if (!ptp.first->atom())
		{
			continue;
		}

		Atom *atom = ptp.first->atom();
		if (atom->elementSymbol() == "C" || atom->elementSymbol() == "H")
		{
			continue;
		}
		

		float sum = sums[2];
		float calculated = sums[2] / (sums[1] + sums[2]);
		float actual = ptp.first->atomConf().occupancy();
		graph->addPoint(0, calculated, actual);
		add_to_CD(&cd, calculated, actual);

		std::cout << actual << " " << sum << " " << grand_sum << " " <<
		ptp.first->atomConf() << std::endl;
	}
	
	float cc = evaluate_CD(cd);
	
	GraphView *gv = new GraphView(this, graph);
	gv->show();
	gv->setInformation("Correlation: " + f_to_str(cc, 3));
}

