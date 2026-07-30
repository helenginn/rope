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

#include "ExhaustiveSearch.h"
#include "CertainStates.h"
#include "CliqueFinder.h"
#include "SearchAll.h"
#include "Clique.h"
#include <time.h>

SearchAll::SearchAll(Clique *parent, Network &network) : 
_clique(parent), _network(network)
{

}

void SearchAll::run()
{
	std::list<Clique> &subs = _clique->subdivisions();
	_clique->removeChildren();
	float total = 0;
	int seconds = 0;
	time_t start = ::time(nullptr);
	
	OpSet<Probe *> expanded = 
	CliqueFinder::expandSelectionToNeighbours(_clique->probes(), 15, true);

	for (Clique &clique : subs)
	{
		if (_cancel && _cancel->load())
		{
			break;
		}

		if (clique.states() && clique.states()->state_count())
		{
			_clique->addItem(&clique);
			continue;
		}

		ExhaustiveSearch search(clique.probes(), expanded);
		search.setCancelFlag(_cancel);
		search.search();
		clickTicker();
		clique.setStates(search.states());
		int num_nodes = search.probe_count();
		int num_results = search.states()->state_count();
		std::string name = "Subnetwork (" + std::to_string(num_nodes) +
		" nodes, " + std::to_string(num_results) + " arrangements)";
		clique.setName(name);
		_clique->addItem(&clique);
		total += search.memory_use();
		time_t end = ::time(nullptr);
		time_t diff = end - start;
		std::cout << "DEBUG: " << diff << " " << total << " MB " << std::endl;
	}
	
	finishTicker();
}
