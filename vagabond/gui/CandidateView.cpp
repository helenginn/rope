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


#include "CandidateView.h"
#include "VagWindow.h"
#include <json/json.hpp>
#include <vagabond/gui/elements/Choice.h>
#include <vagabond/gui/elements/AskYesNo.h>
#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/utils/FileReader.h>
#include <vagabond/core/FileManager.h>
#include <vagabond/core/Environment.h>

using nlohmann::json;

std::vector<std::string> CandidateView::_links;

CandidateView::CandidateView(Scene *prev, std::string json) : ListView(prev)
{
	_json = json;
	parse();
}

void CandidateView::parse()
{
	if (_json.length() == 0)
	{
		return;
	}

	std::cout << _json << std::endl;
	json result = json::parse(_json);

	json result_set = result["result_set"];

	for (json &res : result_set)
	{
		std::string code = res["identifier"];
		size_t pos = code.rfind("_");
		code = code.substr(0, pos);
		to_lower(code);
		_ids.push_back(code);
	}
}

void CandidateView::setup()
{
	addTitle(i_to_str(lineCount()) + " candidate PDBs");
	
	if (lineCount() > 0)
	{
		TextButton *tb = new TextButton("Download", this);
		tb->setRight(0.9, 0.1);
		tb->setReturnTag("download");
		addObject(tb);
	}

	ListView::setup();
}

Renderable *CandidateView::getLine(int i)
{
	std::string pdb = _ids[i];
	std::string candidate = _ids[i] + ".pdb";
	if (Environment::fileManager()->hasFile(candidate))
	{
		pdb += " - downloaded";
	}

	Text *t = new Text(pdb);
	t->setLeft(-0.05, 0);
	return t;
}

void CandidateView::buttonPressed(std::string tag, Button *button)
{
	if (tag == "download")
	{
		AskYesNo *ayn = new AskYesNo(this, "Download " + i_to_str(lineCount()) 
		                            +" PDB files\nwhere available from pdb-redo?",
		                            "download", this);
		setModal(ayn);
	}
	if (tag == "yes_download")
	{
		download();
	}

	ListView::buttonPressed(tag, button);
}

void CandidateView::download()
{
	BadChoice *bc = new BadChoice(this, "Download runs in the background.");
	setModal(bc);

	_links.clear();

	for (const std::string &id : _ids)
	{
		std::string candidate = id + ".pdb";
		if (Environment::fileManager()->hasFile(candidate))
		{
			continue;
		}

		std::string link = "https://pdb-redo.eu/db/" + id + "/"
		+ id + "_final.pdb";
		_links.push_back(link);
	}
	
//	VagWindow::window()->prepareProgressBar(_ids.size());

#ifndef __EMSCRIPTEN__
	ThreadStuff *ts = new ThreadStuff("", &FileManager::acceptDownload, nullptr);
	ts->links = _links;
	pthread_create(&FileManager::thread(), NULL, pull_one_url, ts);
#else
	for (const std::string &link : _links)
	{
		emscripten_async_wget_data(link.c_str(), (void *)&link,
		                           FileManager::prepareDownload, handleError);
	}
#endif
}

void CandidateView::handleError(void *me)
{
	// nothing, may have been deleted by this point.
}

