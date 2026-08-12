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
	tix->addOption("Van der Waals repulsion",
	              toggle_type("Van der Waals repulsion",
	               hnet::Energy::Repulsion),
	               e.source_on(hnet::Energy::Repulsion));
	tix->setVertical(true);
	tix->setOneOnly(false);
	tix->arrange(0.15, 0.52, 0.32, 0.83);
	addObject(tix);

	// top-left display filters - purely cosmetic (see rebuildGraph()'s
	// own comment): neither changes what estimates() itself computes,
	// only which of its already-computed points get plotted.
	TickBoxes *displayTix = new TickBoxes(this, this);

	displayTix->addOption("Show waters", [this, displayTix]()
	{
		_showWaters = displayTix->isTicked("Show waters");
		rebuildGraph();
	}, _showWaters);

	displayTix->addOption("Clique members only", [this, displayTix]()
	{
		_cliqueOnly = displayTix->isTicked("Clique members only");
		rebuildGraph();
	}, _cliqueOnly);

	displayTix->setVertical(true);
	displayTix->setOneOnly(false);

	// same x-range as the energy-source tickboxes below (tix), just
	// higher up - was top-left (0.02, 0.06) but that overlapped the back
	// button.
	displayTix->arrange(0.15, 0.25, 0.32, 0.35);
	addObject(displayTix);

	slider("", hnet::Energy::Torsion, 0.52);
	slider("", hnet::Energy::Acceptor, 0.57);
	slider("", hnet::Energy::Distance, 0.62);
	slider("", hnet::Energy::Angle, 0.67);
	slider("", hnet::Energy::Bulk, 0.72);
	slider("", hnet::Energy::Repulsion, 0.77);

	IndexResponseView::setup();
}

void OccupanciesView::slider(std::string msg, const hnet::Energy::Source &src,
                             float y)
{
	auto drag_me = [src, this](double x, double y)
	{
		_network.energy().alter_amplification(src, x);
	};

	// captured before setup() below - setup() seeds its dot at
	// proportion (0,0) via its own internal finishedDragging("dot",0,0),
	// which (since no _responder is set) fires drag_me(-2, 0) as a side
	// effect, overwriting whatever amplification this source already
	// had with the slider's minimum before we ever get to read it.
	float current = _network.energy().amplification(src);

	Slider *s = new Slider();
	s->setDragFunction(drag_me);
	s->resize(0.15);
	s->setup("", -2, +2, 0.1, false);

	// setStart() takes a fraction of the [-2,+2] range, not the raw
	// value - was always 0.5 (dead centre = 0) regardless of whatever
	// amplification this source already had from a previous session/run,
	// making the slider look reset even when it was not.
	float fraction = (current + 2.f) / 4.f;
	s->setStart(fraction, 0.);
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
		float sampleWeight = (float)clique.sampleWeight();

		for (const ProbeTypePair &ptp : states.ptps())
		{
			float sum = 0; // sum of all energy contributions, populated next
			std::map<int, float> occs = states.proportions(ptp, sum, probs);
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

void OccupanciesView::occupancies()
{
	_lastEstimates = estimates();
	rebuildGraph();
}

void OccupanciesView::rebuildGraph()
{
	if (_lastEstimates.empty())
	{
		// see this method's own header comment - nothing to (re)plot yet.
		return;
	}

	deleteTemps();
	CorrelData cd = empty_CD();

	Graph *graph = new Graph();
	graph->style = Graph::StyleScatter;
	graph->setRange('x', 0, 1);
	graph->setRange('y', 0, 1);
	graph->setAxisLabel('x', "Calculated occupancy");
	graph->setAxisLabel('y', "Observed occupancy");

	// find the watched signal's own residue (chain + residue id) once,
	// independently of pass - the watched probe itself may not survive
	// estimates()'s own filtering (occupancy_sum()/isActiveAtom() etc.)
	// even though other atoms in the same residue do, and we still want
	// those highlighted.
	std::string watched = _clique->watchedSignal();
	std::string watchedChain;
	ResidueId watchedResidue;
	bool haveWatchedResidue = false;

	if (watched.length())
	{
		for (Probe *const &probe : _clique->probes())
		{
			if (probe->is_atom() && probe->desc() == watched)
			{
				watchedChain = probe->atom()->chain();
				watchedResidue = probe->atom()->residueId();
				haveWatchedResidue = true;
				break;
			}
		}
	}

	std::cout << "observed, calculated, samples, atom\n";
	for (auto &pair : _lastEstimates)
	{
		const ProbeTypePair &ptp = pair.first;
		float &observed = pair.second.observed;
		float &calculated = pair.second.calculated;
		size_t &samples = pair.second.samples;

		Atom *atom = ptp.first->atom();

		// "waters" here means the same thing Subdivide::has_non_water()
		// checks - the residue code, not is_bulk() (a separate, narrower
		// "liberated into bulk solvent" existence state already filtered
		// out unconditionally by estimates() itself).
		if (!_showWaters && atom && atom->code() == "HOH")
		{
			continue;
		}

		// _clique is this screen's own top-level clique - its own
		// probes() are the originally-selected/core members, same "bold"
		// distinction ViewCorrelations::showCorrelationMatrix() draws
		// between a subnetwork's own core set and whatever peripheral
		// nodes ExhaustiveSearch's wider resolution pulled in (ptps()
		// includes both, clique.probes() only the former).
		if (_cliqueOnly && _clique->probes().count(ptp.first) == 0)
		{
			continue;
		}

		// filled star (see Graph::addPoint's own comment on pointType)
		// for every atom sharing the watched signal's residue - same
		// star-for-watched visual language ClusterPlot already uses for
		// subnetworks touching the watched signal.
		int pointType = 4;
		if (haveWatchedResidue && atom &&
		    atom->chain() == watchedChain &&
		    atom->residueId() == watchedResidue)
		{
			pointType = 1;
		}

		graph->addPoint(0, calculated, observed, ptp.first->desc(), 1.f,
		                pointType);
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
