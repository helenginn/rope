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
#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/gui/MetadataView.h>
#include <vagabond/utils/FileReader.h>
#include <vagabond/core/Entity.h>
#include <vagabond/core/Metadata.h>
#include <vagabond/core/EntityManager.h>
#include <vagabond/core/Environment.h>

#include <json/json.hpp>
using nlohmann::json;

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

	{
		TextButton *t = new TextButton("Protein data bank", this);
		t->setLeft(0.25, 0.4);
		t->setReturnTag("protein_data_bank");
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

//		std::vector<Entity *> entities = em->entities();
		for (const Entity *e : em->entities())
		{
			if (e == _entity)
			{
				continue;
			}

			if (!m->hasEntity(e->name()))
			{
				continue;
			}
			
			std::string str = i_to_str(m->instanceCountForEntity(e->name()));
			std::string header = "has_" + e->name();
			
			kv[header] = str;
			kv["has_other_entity"] = Value("true");

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

std::string CalculateMetadata::prepareQuery()
{
	std::string entry_ids = "entries(entry_ids: [";
	for (Model const *m : _entity->models())
	{
		entry_ids += "\"" + m->name() + "\", ";
	}
	
	if (_entity->models().size())
	{
		entry_ids.pop_back();
		entry_ids.pop_back();
	}
	
	entry_ids += "]) ";
	
	std::string requests = "{ rcsb_id diffrn { ambient_temp } "\
	"exptl_crystal_grow { pH } reflns { d_resolution_high } "\
	"symmetry { space_group_name_H_M } cell{length_a, length_b, length_c} }";
	
	std::string query = "{ " + entry_ids + requests + "}";
	return query;
}

void CalculateMetadata::fetchFromPDB()
{
	std::string query = prepareQuery();
	load(query);

}

void CalculateMetadata::doThings()
{
	if (_process)
	{
		processResult(_result);
		_process = false;
		_result = "";
	}
}

void CalculateMetadata::handleError()
{
	BadChoice *bc = new BadChoice(this, "Error getting PDB metadata");
	this->setModal(bc);
}

std::string CalculateMetadata::toURL(std::string query)
{
	std::string url = "https://data.rcsb.org/graphql?query=";
	urlencode(query);
	url += query;

	return url;
}

void CalculateMetadata::processResult(std::string result)
{
//	std::cout << result << std::endl;
	
	json all = json::parse(result);
	json &entries = all["data"]["entries"];
	
	Metadata *m = new Metadata();

	for (json &entry : entries)
	{
		Metadata::KeyValues kv;
		std::string model, temp, ph, res, spg;
		if (entry.count("rcsb_id") == 0)
		{
			continue;
		}

		model = entry["rcsb_id"];
		to_lower(model);
		kv["model"] = model;

		json &diffrn = entry["diffrn"];
		if (!diffrn[0]["ambient_temp"].is_null())
		{
			temp = i_to_str(diffrn[0]["ambient_temp"]);
			std::cout << "Temp: " << temp << std::endl;
			kv["Collection Temperature (K)"] = temp;
		}

		json &exptl = entry["exptl_crystal_grow"];
		if (!exptl[0]["pH"].is_null())
		{
			ph = f_to_str(exptl[0]["pH"], 1);
			kv["pH"] = ph;
		}

		json &reflns = entry["reflns"];
		if (!reflns[0]["d_resolution_high"].is_null())
		{
			res = f_to_str(reflns[0]["d_resolution_high"], 2);
			kv["Resolution (A)"] = res;
		}

		json &symmetry = entry["symmetry"];
		if (!symmetry["space_group_name_H_M"].is_null())
		{
			spg = symmetry["space_group_name_H_M"];
			kv["Space group"] = spg;
		}

		for (size_t i = 0; i < 3; i++)
		{
			std::string d = (i == 0 ? "a" : (i == 1 ? "b" : "c"));

			json &cell = entry["cell"];
			if (!cell["length_" + d].is_null())
			{
				std::string a = f_to_str(cell["length_" + d], 2);
				kv["Unit cell length " + d + " (A)"] = a;
			}
		}
		
		std::cout << "model: " << model << std::endl;
		std::cout << "temp: " << temp << std::endl;
		std::cout << "res: " << res << std::endl;
		
		m->addKeyValues(kv, true);
	}
	
	MetadataView *mv = new MetadataView(this, m);
	mv->show();
}

void CalculateMetadata::buttonPressed(std::string tag, Button *button)
{
	if (tag == "bound_entities")
	{
		populateBoundEntities();
	}
	
	if (tag == "protein_data_bank")
	{
		fetchFromPDB();
	}

	Scene::buttonPressed(tag, button);
}

/*
void CalculateMetadata::load(std::string query)
{
	std::string url = "https://data.rcsb.org/graphql?query=";
	urlencode(query);
	url += query;

#ifndef __EMSCRIPTEN__
	ThreadStuff *ts = new ThreadStuff(url, &CalculateMetadata::prepareResult, this);
	pthread_create(&_thread, NULL, pull_one_url, ts);
#else
	emscripten_async_wget_data(url.c_str(), this,
	                      handleResult, handleError);

#endif
}
*/
