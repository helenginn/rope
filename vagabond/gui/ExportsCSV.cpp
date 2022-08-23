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

#include "ExportsCSV.h"
#include <vagabond/gui/elements/AskForText.h>
#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/gui/elements/Scene.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <fstream>

void ExportsCSV::askForFilename()
{
#ifdef __EMSCRIPTEN__
		EM_ASM_({ window.download = download; window.download($0, $1, $2) }, 
		        "metadata.csv", _csv.c_str(), _csv.length());
#else
		AskForText *aft = new AskForText(_me, "File name to save to:",
		                                 "export_csv", _me);
		_me->setModal(aft);
#endif
}

void ExportsCSV::exportCSVToFile(std::string filename)
{
	std::ofstream file;
	file.open(filename);

	if (file.is_open())
	{
		file << _csv;
		file.close();
		BadChoice *bc = new BadChoice(_me, "Exported file");
		_me->setModal(bc);
	}
	else
	{
		BadChoice *bc = new BadChoice(_me, "Failure writing file");
		_me->setModal(bc);
	}
}

void ExportsCSV::buttonPressed(std::string tag, Button *button)
{
	if (tag == "export")
	{
		supplyCSV();
		askForFilename();
	}
	else if (tag == "export_csv")
	{
		TextEntry *te = static_cast<TextEntry *>(button);
		std::string filename = te->scratch();
		exportCSVToFile(filename);
	}
}

