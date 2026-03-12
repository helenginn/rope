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
#ifndef __vagabond__ChooseAntigen__
#define __vagabond__ChooseAntigen__

#include "Mab.h"
#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/gui/ChooseHeader.h>

inline
std::function<void()> make_choose_antigen(Scene *prev,
                                          std::string *where,
                                          Antigens *antigens)
{
	return [prev, where, antigens]()
	{
		auto picked_antigen = [prev, where](std::string antigen)
		{
			*where = antigen;
			prev->refresh();
		};

		OpSet<std::string> names;
		for (const Antigen &antigen : *antigens)
		{
			names += antigen.title;
		}
		
		if (names.size())
		{
			ChooseHeader *ch = new ChooseHeader(prev, true);
			ch->setHeaders(names);
			ch->setChoose(picked_antigen);
			ch->show();
		}
		else
		{
			BadChoice *bc = 
			new BadChoice(prev, "Please add at least one antigen \n"
			              "model from the previous menu in order \n"
			              "to be able to pick it from a list.");
			prev->setModal(bc);
		}
	};

}

#endif

