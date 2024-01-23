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

#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/core/loopy/ListConformers.h>
#include "LoopyDisplay.h"
#include "ConformerClusters.h"
#include "Polymer.h"
#include "RopeSpaceItem.h"
#include "ConfSpaceView.h"
#include <vagabond/core/RopeCluster.h>

LoopyDisplay::LoopyDisplay(Scene *prev, Polymer *const &pol) 
: Scene(prev), Display(prev), _loopy(pol)
{
	_polymer = pol;
	_polymer->load();
	setOwnsAtoms(false);
	_loopy.setResponder(this);
}

LoopyDisplay::~LoopyDisplay()
{
	_polymer->unload();

}

void LoopyDisplay::setup()
{
	AtomGroup *grp = _polymer->currentAtoms();
	loadAtoms(grp);
	mechanicsButton();
	Display::setup();

	TextButton *button = new TextButton("Begin build loop", 
	                                    this);
	button->setReturnTag("looping");
	button->resize(0.8);
	button->setCentre(0.5, 0.85);
	addObject(button);


	TextButton *clusters = new TextButton("See clusters", 
	                                    this);
	clusters->setReturnTag("clusters");
	clusters->resize(0.8);
	clusters->setCentre(0.9, 0.05);
	addObject(clusters);
}

void LoopyDisplay::buttonPressed(std::string tag, Button *button)
{
	if (tag == "looping")
	{
		if (_loopy.size() >= 1)
		{
			_polymer->load();
			_worker = new std::thread(&Loopy::operator(), &_loopy, 0);
			_polymer->unload();
		}
	}
	else if (tag == "clusters")
	{
		std::unique_lock<std::mutex> lock(_spaceMut);
		ConfSpaceView *csv = new ConfSpaceView(this, _polymer->entity(), _space);
		csv->show();
	}

	Display::buttonPressed(tag, button);
}

void LoopyDisplay::sendObject(std::string tag, void *object)
{
	if (tag == "non_clash")
	{
		ListConformers *confs = static_cast<ListConformers *>(object);
		if (confs->size() == 0)
		{
			return;
		}

		if (_clusterer)
		{
			_clusterer->detach();
			delete _clusterer;
			_clusterer = nullptr;
		}
		
		_clusterer = new std::thread(&LoopyDisplay::prepareConformerCluster,
		                             this, *confs);
	}
	else if (tag == "non_contact")
	{
		ListConformers *confs = static_cast<ListConformers *>(object);
		confs->setMetadata(&_metadata, "clash", "false");
	}
}

void LoopyDisplay::prepareConformerCluster(ListConformers confs)
{
	if (confs.size() == 0)
	{
		return;
	}
	
	Conformer *const &model = confs.front();
	size_t n_angles = model->headers().size();

	MetadataGroup *group = new MetadataGroup(n_angles);
	group->addHeaders(model->headers());
	
	for (Conformer *const &conf : confs)
	{
		std::string id = conf->id();
		Metadata::KeyValues kv = 
		{{"rmsd", Value(conf->rmsd())}, 
		{"correlation", Value(conf->correlationWithDensity())}, 
		{"instance", id}};

		std::vector<Angular> angles = conf->angles();
		group->addMetadataArray(conf, angles);
		_metadata.addKeyValues(kv, true);
	}

	group->write(_polymer->id() + "_loop_torsions.csv");

	RopeSpaceItem *item = new RopeSpaceItem(_polymer->entity());
	item->setMetadata(&_metadata);
	item->setMode(rope::ConfTorsions);
	item->setObjectGroup(group);
	item->torsionCluster();
	
	std::unique_lock<std::mutex> lock(_spaceMut);
	_space.setAssociatedMetadata(&_metadata);
	_space.save(item, _polymer->entity(), rope::ConfTorsions);
}
