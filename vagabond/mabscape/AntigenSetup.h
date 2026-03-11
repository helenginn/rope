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

#ifndef __vagabond__AntigenSetup__
#define __vagabond__AntigenSetup__

#include <vagabond/gui/elements/Scene.h>
#include "Mab.h"

class FileManager;


class AntigenSetup : public Scene
{
public:
	AntigenSetup(Scene *scene, ColourMap &colours, Antigens &antigens);
	void prepareChoosePDB();
	void prepareChooseTitle();
	void prepareAssignChains();
	void listAntigenChains();
	void scrollButtons();
	void deleteButton();

	void setup();
	void refresh();
	
	Antigen &antigen()
	{
		return *_antigen;
	}
private:
	Antigens &_antigens;
	Antigens::iterator _antigen;
	ColourMap &_colours;

};

#endif
