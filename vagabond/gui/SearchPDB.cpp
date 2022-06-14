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


#include "SearchPDB.h"
#include "CandidateView.h"
#include <vagabond/core/Entity.h>
#include <json/json.hpp>
#include <vagabond/utils/FileReader.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/ChooseRange.h>
#include <vagabond/gui/elements/BadChoice.h>

SearchPDB::SearchPDB(Scene *prev, Entity *ent) : Scene(prev)
{
	_entity = ent;
	_cutoff = 98;
}

void SearchPDB::setup()
{
	addTitle("Search PDB by " + _entity->name() + " sequence");

	{
		TextButton *tb = new TextButton("Sequence identity cutoff", this);
		tb->setLeft(0.2, 0.3);
		tb->setReturnTag("get_identity_cutoff");
		addObject(tb);
	}

	{
		Text *t = new Text(i_to_str(_cutoff));
		t->setRight(0.8, 0.3);
		_cutoffText = t;
		addObject(t);
	}

	{
		TextButton *tb = new TextButton("Run", this);
		tb->setLeft(0.8, 0.8);
		tb->setReturnTag("run");
		addObject(tb);
	}
}

void SearchPDB::buttonPressed(std::string tag, Button *button)
{
	if (tag == "get_identity_cutoff")
	{
		std::string str = "Choose sequence identify cutoff";
		ChooseRange *cr = new ChooseRange(this, str, "set_identity_cutoff", this);
		cr->setRange(0, 99, 99);
		setModal(cr);
	}

	if (tag == "set_identity_cutoff")
	{
		ChooseRange *cr = static_cast<ChooseRange *>(button->returnObject());
		float max = cr->max();
		_cutoff = lrint(max);
		refresh();
	}
	
	if (tag == "run")
	{
		std::string json = prepareQuery();
		load(json);
	}
	
	Scene::buttonPressed(tag, button);
}

void SearchPDB::refresh()
{
	_cutoffText->setText(i_to_str(_cutoff));
}

void SearchPDB::processResult(std::string result)
{
	CandidateView *cv = new CandidateView(this, result);
	cv->show();
}

std::string SearchPDB::toURL(std::string query)
{
	std::string url = "https://search.rcsb.org/rcsbsearch/v2/query?json=";
	urlencode(query);
	url += query;

	return url;
}

void SearchPDB::handleError()
{
	BadChoice *bc = new BadChoice(this, "Error getting PDB codes");
	this->setModal(bc);
}

void SearchPDB::render()
{
	if (_process)
	{
		processResult(_result);
		_process = false;
		_result = "";
	}

	Scene::render();
}

std::string SearchPDB::prepareQuery()
{
	nlohmann::ordered_json query_params;
	query_params["target"] = "pdb_protein_sequence";
	query_params["value"] = _entity->sequence()->str();
	query_params["identity_cutoff"] = (float)_cutoff / 100.;
	query_params["evalue_cutoff"] = 0.1;

	nlohmann::ordered_json query;
	query["type"] = "terminal";
	query["service"] = "sequence";
	query["parameters"] = query_params;

	nlohmann::ordered_json ranking;
	ranking["sort_by"] = "score";
	ranking["direction"] = "asc";

	nlohmann::ordered_json group_by;
	group_by["aggregation_method"] = "sequence_identity";
	group_by["similarity_cutoff"] = 50;
	group_by["ranking_criteria_type"] = ranking;
	
	nlohmann::ordered_json request_options;
	request_options["results_verbosity"] = "minimal";
	request_options["group_by"] = group_by;
	request_options["group_by_return_type"] = "groups";

	nlohmann::ordered_json full;
	full["query"] = query;
	full["request_options"] = request_options;
	full["return_type"] = "polymer_entity";

	return to_string(full);
}
