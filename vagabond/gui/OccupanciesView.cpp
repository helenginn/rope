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
#include <vagabond/core/protonic/Clique.h>
#include <vagabond/core/protonic/Network.h>
#include <vagabond/core/protonic/Correlative.h>
#include <vagabond/core/protonic/ProbeResult.h>
#include <vagabond/core/protonic/CertainStates.h>
#include <vagabond/gui/elements/TextButton.h>

OccupanciesView::OccupanciesView(Scene *prev, Clique *clique, Network &network)
: Scene(prev), IndexResponseView(prev), _clique(clique), _network(network)
{

}


void OccupanciesView::setup()
{
	addTitle("Occupancy prediction");

	TextButton *tb = new TextButton("Check occupancies");
	tb->setReturnJob([this]() { occupancies(); });
	tb->setCentre(0.50, 0.84);
	addObject(tb);

	std::map<ProbeTypePair, OccData> pass = estimates();
	OpSet<ProbeTypePair> active;
	for (auto &pair : pass)
	{
		active += pair.first;
	}
	std::cout << "Active: " << active.size() << std::endl;

	float all_ave = 0;
	OpSet<ProbeTypePair> all = 
	Correlative::probeTypePairs(_clique->subdivisions(), all_ave);

	_correlative = new Correlative(active, all_ave, false);

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

	_overall = _correlative->acquireMatrix();

	auto combine = [](float x, float y)
	{
		return x * y;
	};

	FloydWarshall fw(_overall, combine, true);
	fw.run();

	PCA::Matrix tmp(_overall);
	MatrixPlot *mp = new MatrixPlot(tmp);
	mp->setCentre(0.25, 0.5);
	addObject(mp);

	auto lookup = _correlative->matrixLookup();
	auto display_lookup = [this, lookup](float x, float y)
	{
		std::string info = _correlative->matrixLookup()(x, y);
		setInformation(info);
	};
	mp->setHoverJob(display_lookup);
	
	IndexResponseView::setup();
}

std::map<ProbeTypePair, OccupanciesView::OccData> OccupanciesView::estimates()
{
	struct OccupancyEstimate
	{
		std::map<int, float> results{};
		float sum = 0;
		size_t samples = 0;
	};
	
	std::map<ProbeTypePair, std::vector<OccupancyEstimate>> occupancies;
	std::map<ProbeTypePair, OccData> ret;
	
	auto process_clique = [&occupancies](const Clique &clique)
	{
		if (!clique.states()) return;
		const CertainStates &states = *clique.states();
		float ave = states.average_score();

		for (const ProbeTypePair &ptp : states.ptps())
		{
			float sum = 0;
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

		std::string desc = ptp.first->desc();
		desc += (ptp.second == hnet::Types::BondType ?  " bonding" : " exists");

		const std::vector<OccupancyEstimate> &estimates = occs.second;
		std::map<int, float> sums;
		float grand_sum = 0; 
		size_t sample_count = 0;

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

		if (atom->elementSymbol() == "C" || atom->elementSymbol() == "H")
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
		
		if (fabs(calculated - 0.5) < 1e-6)
		{
			continue;
		}
		
		ret[ptp] = {calculated, observed, grand_sum, sample_count};
	}
	
	return ret;
}

void OccupanciesView::updateEstimates(std::map<ProbeTypePair, OccData> &ests)
{
	auto current = ests;
	
	auto fraction_for = [](std::map<ProbeTypePair, 
	                       OccData> &ests,
	                       const ProbeTypePair &ptp, int state)
	{
		float calculated = ests.at(ptp).calculated;
		if (state == 2) return calculated;
		if (state == 1) return 1 - calculated;
		return 0.f;
	};

	for (auto &pair : ests)
	{
		const ProbeTypePair &left = pair.first;
		Eigen::MatrixXf rows = _correlative->rowsFor(left);
		
		float frax[2] = {0, 0};
		for (const int &state : {1, 2})
		{
			float my_frac = fraction_for(_first, left, state);
			float curr_frac = fraction_for(ests, left, state);

			int idx = state - 1;
			Eigen::VectorXf ccs = rows(idx, Eigen::all);
			
			int n = 0;
			for (auto &next : ests)
			{
				for (const int &other_state : {1, 2})
				{
					const ProbeTypePair &right = next.first;
					float cc = ccs[n];
					float other_frac = fraction_for(_first, right, 
					                                other_state); // urgh

					float same = (1 - cc) * sqrt(my_frac * curr_frac);
					float updated = (cc) * sqrt(my_frac * other_frac);

					frax[state - 1] += (same + updated) * cc;
					n++;
				}
			}
		}
		
		ests[left].calculated = frax[1] / (frax[0] + frax[1]);
	}
}

void OccupanciesView::occupancies()
{
	std::map<ProbeTypePair, OccData> pass;
	if (_estimates.size() == 0)
	{
		pass = estimates();
		_first = pass;
	}
	else
	{
		pass = _estimates;
		updateEstimates(pass);
	}

	std::map<ProbeTypePair, OccData> copy = pass;
	_estimates = pass;

	deleteTemps();
	CorrelData cd = empty_CD();
	
	Graph *graph = new Graph();
	graph->style = Graph::StyleScatter;
	graph->setRange('x', 0, 1);
	graph->setRange('y', 0, 1);
	graph->setAxisLabel('x', "Calculated occupancy");
	graph->setAxisLabel('y', "Observed occupancy");
	
	std::cout << "observed, calculated, samples, atom" <<  std::endl;
	for (auto &pair : pass)
	{
		const ProbeTypePair &ptp = pair.first;
		float &observed = pair.second.observed;
		float &calculated = pair.second.calculated;
		size_t &samples = pair.second.samples;

		graph->addPoint(0, calculated, observed, ptp.first->desc());
		add_to_CD(&cd, calculated, observed);

		std::cout << observed << " " << calculated << " " << samples << " " <<
		ptp.first->atomConf() << std::endl;
	}
	
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
