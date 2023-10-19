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

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#include <curl/curl.h>
#include <vagabond/utils/extra_curl_utils.h>
#endif

#include <iostream>
#include <vagabond/utils/FileReader.h>

#include "EntityFromSequence.h"
#include "FastaView.h"
#include "AddEntity.h"

#include <vagabond/gui/elements/TextEntry.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/AskForText.h>
#include <vagabond/gui/elements/BadChoice.h>

EntityFromSequence::EntityFromSequence(Scene *prev) : 
Scene(prev), Fetcher()
{

}

EntityFromSequence::~EntityFromSequence()
{

}

void EntityFromSequence::setup()
{
	addTitle("Entity from sequence");
	
	{
		TextButton *tb = new TextButton("From UniProt ID", this);
		tb->setReturnTag("uniprot");
		tb->setLeft(0.2, 0.3);
		addObject(tb);
	}
	
	{
		TextButton *tb = new TextButton("From fasta file", this);
		tb->setReturnTag("fasta");
		tb->setLeft(0.2, 0.4);
		addObject(tb);
	}
	
	{
		TextButton *tb = new TextButton("Type in sequence", this);
		tb->setReturnTag("type");
		tb->setLeft(0.2, 0.5);
		addObject(tb);
	}
}

void EntityFromSequence::handleError()
{
	BadChoice *bc =  new BadChoice(this, "Connection error");
	this->setModal(bc);
}

void EntityFromSequence::processResult(std::string seq)
{

	try
	{
		json all = json::parse(seq);
		json &results = all["results"];
		
		std::string next;

		for (json &result : results)
		{
			if (result.count("sequence") == 0)
			{
				continue;
			}

			json &seq = result["sequence"];
			if (seq.count("value") == 0)
			{
				continue;
			}
			
			next = seq["value"];
			break;
		}
		
		if (next.length() == 0)
		{
			throw std::runtime_error("no sequence");
		}

		AddEntity *addEntity = new AddEntity(_previous, next);
		back();
		addEntity->show();
	}
	catch (...)
	{
		BadChoice *bc = new BadChoice(this, "Sequence could not be obtained");
		setModal(bc);
		return;
	}
}

std::string EntityFromSequence::toURL(std::string query)
{
	std::string url = "https://rest.uniprot.org/uniprotkb/search?query=accession:";
	url += query;

	return url;
}

void EntityFromSequence::buttonPressed(std::string tag, Button *button)
{
	if (tag == "uniprot")
	{
		AskForText *aft = new AskForText(this, "Enter UniProt ID:",
		                                 "uniprot_id", this, TextEntry::Id);
		setModal(aft);
		return;
	}
	
	if (tag == "fasta")
	{
		FileView *view = new FileView(this, this, true);
		view->filterForTypes(File::Sequence);
		view->show();
	}
	
	if (tag == "type")
	{
		AskForText *aft = new AskForText(this, "Enter peptide sequence:",
		                                 "peptide_sequence", this, 
		                                 TextEntry::Alphabetic);
		setModal(aft);
		return;

	}

	if (tag == "uniprot_id")
	{
		TextEntry *te = static_cast<TextEntry *>(button);
		std::string text = te->scratch();
		load(text);

	}

	if (tag == "peptide_sequence")
	{
		TextEntry *te = static_cast<TextEntry *>(button);
		std::string text = te->scratch();
		makePeptide(text);
	}

	Scene::buttonPressed(tag, button);
}

void EntityFromSequence::makePeptide(std::string text)
{
	AddEntity *addEntity = new AddEntity(_previous, text);
	back();
	addEntity->show();

}

void EntityFromSequence::doThings()
{
	if (_process)
	{
		processResult(_result);
		_result = "";
		_process = false;
	}
}

void EntityFromSequence::sendObject(std::string filename, void *object)
{
	FastaView *fv = new FastaView(this, filename, true);
	fv->show();
}
