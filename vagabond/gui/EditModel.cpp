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

#include "EditModel.h"
#include "AdjustPhPkas.h"
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/Image.h>
#include <vagabond/core/protonic/Network.h>
#include <vagabond/core/Model.h>

EditModel::EditModel(Scene *prev, Network &network,
                     const std::function<void()> &rebuild)
: Scene(prev), _network(network), _rebuild(rebuild)
{
	if (_network.model())
	{
		_lastRecalculated = _network.model()->protonSettings();
	}
}

void EditModel::setup()
{
	addTitle("Edit model");

	TextButton *pka = new TextButton("Adjust pH and pKas", this);
	pka->setCentre(0.5, 0.3);
	pka->setReturnJob([this]()
	{
		AdjustPhPkas *apk = new AdjustPhPkas(this, _network);
		apk->show();
	});
	addObject(pka);

	TextButton *recalc = new TextButton("Recalculate proton network", this);
	recalc->setCentre(0.5, 0.85);
	recalc->setReturnJob([this]()
	{
		if (_network.model())
		{
			_lastRecalculated = _network.model()->protonSettings();
		}

		if (_rebuild)
		{
			_rebuild();
			// back to ProtonNetworkView (this screen's own _previous) so
			// the progress bar buildNetwork() just kicked off is actually
			// visible, rather than left running behind this screen.
			back();
		}
	});
	addObject(recalc);

	_refreshIcon = new Image("assets/images/refresh.png");
	_refreshIcon->resize(0.04);
	_refreshIcon->setLeft(0.72, 0.85);
	_refreshIcon->addAltTag("pH/pKa values have changed since the proton "\
	                        "network was last calculated");
	_refreshIcon->setDisabled(true);
	addObject(_refreshIcon);
}

void EditModel::doThings()
{
	if (_refreshIcon && _network.model())
	{
		bool outdated = (_network.model()->protonSettings() !=
		                 _lastRecalculated);
		_refreshIcon->setDisabled(!outdated);
	}

	Scene::doThings();
}
