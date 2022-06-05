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

#include "CalculateMetadata.h"
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/MetadataView.h>
#include <vagabond/utils/FileReader.h>
#include <vagabond/core/Entity.h>
#include <vagabond/core/EntityManager.h>
#include <vagabond/core/Environment.h>

CalculateMetadata::CalculateMetadata(Scene *prev, Entity *ent) : Scene(prev)
{
	_entity = Environment::entityManager()->entity(ent->name());
}

void CalculateMetadata::setup()
{
	addTitle("Add to metadata...");

	{
		Text *t = new Text("Calculate metadata from what source...");
		t->setLeft(0.2, 0.2);
		addObject(t);
	}

	{
		TextButton *t = new TextButton("Other bound entities", this);
		t->setLeft(0.25, 0.3);
		t->setReturnTag("bound_entities");
		addObject(t);
	}
}

void CalculateMetadata::populateBoundEntities()
{
	Metadata *md = new Metadata();

	for (Model const *m : _entity->models())
	{
		EntityManager *em = Environment::entityManager();
		int count = 0;

		Metadata::KeyValues kv;
		kv["model"] = m->id();

		for (const Entity &e : em->objects())
		{
			if (&e == _entity)
			{
				continue;
			}

			if (!m->hasEntity(e.name()))
			{
				continue;
			}
			
			std::string str = i_to_str(m->moleculeCountForEntity(e.name()));
			std::string header = "has_" + e.name();
			
			kv[header] = str;

			count++;
		}
		
		if (count > 0)
		{
			md->addKeyValues(kv, true);
		}
	}

	MetadataView *mv = new MetadataView(this, md);
	mv->show();
}

void CalculateMetadata::buttonPressed(std::string tag, Button *button)
{
	if (tag == "bound_entities")
	{
		populateBoundEntities();
	}

	Scene::buttonPressed(tag, button);
}
