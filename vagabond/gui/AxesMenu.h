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

#ifndef __vagabond__AxesMenu__
#define __vagabond__AxesMenu__

#include <vagabond/gui/elements/ListView.h>
#include <vagabond/c4x/Cluster.h>
#include <vagabond/core/MetadataGroup.h>
#include "ExportsCSV.h"

class AxesMenu : public ListView, public ExportsCSV
{
public:
	AxesMenu(Scene *prev);
	
	void setEntityId(std::string entity_id)
	{
		_entity_id = entity_id;
	}

	void setCluster(Cluster<MetadataGroup> *cluster)
	{
		_cluster = cluster;
	}

	virtual size_t lineCount();
	virtual Renderable *getLine(int i);

	virtual void buttonPressed(std::string tag, Button *button = nullptr);

	virtual void setup();
private:
	Cluster<MetadataGroup> *_cluster = nullptr;
	std::string _entity_id;

	virtual void supplyCSV();
	virtual void supplyModal(Modal *m)
	{
		setModal(m);
	}
	
};

#endif
