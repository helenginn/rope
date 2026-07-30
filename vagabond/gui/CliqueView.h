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

#ifndef __vagabond__CliqueView__
#define __vagabond__CliqueView__

#include <vagabond/gui/elements/Image.h>
#include <vagabond/core/Item.h>

class Network;
class LineGroup;
class ProtonNetworkView;

class CliqueView : public Image
{
public:
	CliqueView(ProtonNetworkView *scene, 
	Network &network, const OpSet<Probe *> &probes);

	void setupConstants();
	
	void setKillAndClean(const std::function<void()> &do_it)
	{
		_kill = do_it;
	}
	
	void kill()
	{
		if (_kill) _kill();
	}
	
	void highlightCliquesWith(const OpSet<Probe *> &probes);
	void insertClique(Clique *clique);

	// re-applies wireDescendants() to every top-level clique already in
	// this view. Subdivisions can be created (via SearchAll/Exhaustive
	// search) or have their select job temporarily borrowed (see
	// ViewCorrelations) after this view was first built and cached -
	// call this whenever the view is shown again to pick up either case.
	void rewireSubdivisions();
private:
	void setupCloseButton();

	// subdivisions created by SearchAll (see SearchAll::run()) are wired
	// into a clique's Item children purely for display - unlike top-level
	// Network cliques, they never go through insertClique(), so they have
	// no select job. Recurse through a clique's descendants after it is
	// inserted and give each one a minimal, view-only click behaviour.
	void wireDescendants(Item *item);
	void wireSubdivision(Clique *sub);

	Network &_network;
	std::function<void()> _kill{};
	
	ProtonNetworkView *_scene{};
	Item _parent{};
	Item _ambiguous{};
	Item _wet{};
	Item _dry{};
	Item _certain{};
	LineGroup *_lg{};
	
	std::vector<Clique *> _cliques;
};

#endif
