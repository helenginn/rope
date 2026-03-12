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

#ifndef __vagabond__ListDeletable__
#define __vagabond__ListDeletable__

#include <vagabond/utils/OpSet.h>
#include <vagabond/gui/elements/Text.h>
#include <vagabond/gui/elements/Scene.h>
#include <vagabond/gui/elements/ImageButton.h>

inline
void make_list_deletable(Scene *scene, OpSet<std::string> *where,
                         const OpSet<std::string> &start,
                         float &top)
{
	for (const std::string &str : start)
	{
		Text *text = new Text(str);
		text->setRight(0.8, top);
		scene->addTempObject(text);

		ImageButton *ib = new ImageButton("assets/images/cross.png", 
		                                  scene);
		ib->resize(0.06);
		ib->setLeft(0.8, top);
		ib->setReturnJob
		([scene, where, str]()
		 {
			*where -= str;
			scene->refresh();
		 });
		scene->addTempObject(ib);

		top += 0.06;
	}
	
}

#endif
