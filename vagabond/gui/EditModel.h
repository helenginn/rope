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

#ifndef __vagabond__EditModel__
#define __vagabond__EditModel__

#include <vagabond/gui/elements/Scene.h>
#include <vagabond/core/CustomProtonSettings.h>
#include <functional>

class Network;
class Image;

// developer-only entry point (ProtonNetworkView's "Edit model...", gated
// behind ROPE_DEV) for model-editing tools that don't fit the regular
// H-bond analysis flow - currently just a menu of one, "Adjust pH and
// pKas" (AdjustPhPkas.h), with more expected to land here later.
class EditModel : public Scene
{
public:
	// rebuild, if given, is called when "Recalculate proton network" is
	// pressed - ProtonNetworkView passes its own buildNetwork() in, since
	// a pH/pKa change requires a full rebuild (see Network::
	// chargeStatesForPKa()/applyPKaEnergy(), both baked into the
	// constructor's own sweeps, not something a live update could patch
	// in afterwards). Left null only in contexts with no such view to
	// rebuild - the button simply stays a snapshot-only "dismiss the
	// outdated icon" action there, same as before this existed.
	EditModel(Scene *prev, Network &network,
	         const std::function<void()> &rebuild = nullptr);

	virtual void setup();
	virtual void doThings();
private:
	Network &_network;
	std::function<void()> _rebuild;

	// snapshot of Model::protonSettings() as of the last time
	// "Recalculate proton network" was pressed (or this screen was
	// opened, if never) - doThings() compares the live settings against
	// this every frame to decide whether _refreshIcon should show.
	CustomProtonSettings _lastRecalculated;
	Image *_refreshIcon = nullptr;
};

#endif
