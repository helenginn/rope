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
#include "MatrixPlot.h"
#include <vagabond/utils/maths.h>
#include <vagabond/utils/FileReader.h>
#include <vagabond/utils/FloydWarshall.h>
#include <vagabond/gui/GraphView.h>
#include <vagabond/gui/Graph.h>
#include <vagabond/core/protonic/Energy.h>
#include <vagabond/core/protonic/Clique.h>
#include <vagabond/core/protonic/Network.h>
#include <vagabond/core/protonic/ProbeResult.h>
#include <vagabond/core/protonic/CertainStates.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/TickBoxes.h>
#include <vagabond/gui/elements/Slider.h>

OccupanciesView::OccupanciesView(Scene *prev, Clique *clique, Network &network)
: Scene(prev), IndexResponseView(prev), _clique(clique), _network(network)
{

}


void OccupanciesView::setup()
{
	addTitle("Occupancy prediction");

	TextButton *tb = new TextButton("Check occupancies");
	tb->setReturnJob([this]() { occupancies(); });
	tb->setCentre(0.50, 0.92);
	addObject(tb);
	
	TickBoxes *tix = new TickBoxes(this, this);

	auto toggle_type = [this, tix]
	(std::string tag, hnet::Energy::Source source)
	{
		return [this, tix, tag, source]()
		{
			bool ticked = tix->isTicked(tag);
			_network.energy().alter_source(source, ticked);
		};
	};

	hnet::Energy &e = _network.energy();
	tix->addOption("Torsion energies", 
	              toggle_type("Torsion energies", 
	              hnet::Energy::Torsion),
	              e.source_on(hnet::Energy::Torsion));
	tix->addOption("Hydrogen bond acceptance", 
	              toggle_type("Hydrogen bond acceptance", 
	               hnet::Energy::Acceptor), 
	               e.source_on(hnet::Energy::Acceptor));
	tix->addOption("Hydrogen bond distance", 
	              toggle_type("Hydrogen bond distance", 
	               hnet::Energy::Distance), 
	               e.source_on(hnet::Energy::Distance));
	tix->addOption("Hydrogen bond angles", 
	              toggle_type("Hydrogen bond angles", 
	               hnet::Energy::Angle), 
	               e.source_on(hnet::Energy::Angle));
	tix->addOption("Liberation into bulk solvent", 
	              toggle_type("Liberation into bulk solvent", 
	               hnet::Energy::Bulk), 
	               e.source_on(hnet::Energy::Bulk));
	tix->setVertical(true);
	tix->setOneOnly(false);
	tix->arrange(0.15, 0.52, 0.32, 0.78);
	addObject(tix);
	
	slider("", hnet::Energy::Torsion, 0.52);
	slider("", hnet::Energy::Acceptor, 0.57);
	slider("", hnet::Energy::Distance, 0.62);
	slider("", hnet::Energy::Angle, 0.67);
	slider("", hnet::Energy::Bulk, 0.72);
	
	IndexResponseView::setup();
}

void OccupanciesView::slider(std::string msg, const hnet::Energy::Source &src,
                             float y)
{
	auto drag_me = [src, this](double x, double y)
	{
		_network.energy().alter_amplification(src, x);
	};

	Slider *s = new Slider();
	s->setDragFunction(drag_me);
	s->resize(0.15);
	s->setup("", -2, +2, 0.1, false);
	s->setStart(0.5, 0.);
	s->setCentre(0.1, y);
	addObject(s);

}

OccupanciesView::EstimateMap OccupanciesView::estimates()
{
	struct OccupancyEstimate // one per clique
	{
		std::map<int, float> results{}; // one per state
		float sum = 0;
		size_t samples = 0;
	};
	
	std::map<ProbeTypePair, std::vector<OccupancyEstimate>> occupancies;
	EstimateMap ret;
	
	auto process_clique = [&occupancies](const Clique &clique)
	{
		if (!clique.states()) return;
		const CertainStates &states = *clique.states();
		float ave = states.average_score();

		for (const ProbeTypePair &ptp : states.ptps())
		{
			float sum = 0; // sum of all energy contributions, populated next
			std::map<int, float> occs = states.proportions(ptp, sum, ave);
			occupancies[ptp].push_back({occs, sum, states.state_count()});
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

				float weight = est.sum;
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

void OccupanciesView::occupancies()
{
	EstimateMap pass = estimates();

	deleteTemps();
	CorrelData cd = empty_CD();
	
	Graph *graph = new Graph();
	graph->style = Graph::StyleScatter;
	graph->setRange('x', 0, 1);
	graph->setRange('y', 0, 1);
	graph->setAxisLabel('x', "Calculated occupancy");
	graph->setAxisLabel('y', "Observed occupancy");

	std::cout << "observed, calculated, samples, atom\n";
	for (auto &pair : pass)
	{
		const ProbeTypePair &ptp = pair.first;
		float &observed = pair.second.observed;
		float &calculated = pair.second.calculated;
		size_t &samples = pair.second.samples;

		graph->addPoint(0, calculated, observed, ptp.first->desc());
		add_to_CD(&cd, calculated, observed);

		std::cout << observed << " " << calculated << " " << samples << " " <<
		ptp.first->atomConf() << "\n";
	}
	std::cout << std::flush;
	
	float cc = evaluate_CD(cd);
	
	setInformation("Correlation: " + f_to_str(cc, 3));
	graph->setup(0.4, 0.5);
	graph->addToGraphPosition(0.75, 0.5);
	graph->setIndexResponder(this);
	_graph = graph;
	addTempObject(graph);

}

void OccupanciesView::interactedWithNothing(bool left, bool hover)
{
	if (_graph)
	{
		_graph->clearLabels();
	}
}
