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

#include "ViewCorrelations.h"
#include <vagabond/core/protonic/Clique.h>
#include <vagabond/gui/elements/list/LineGroup.h>
#include <vagabond/gui/elements/ScrollBox.h>

ViewCorrelations::ViewCorrelations(Scene *prev, Clique *clique)
: Scene(prev), _clique(clique)
{

}

void ViewCorrelations::setup()
{
	addTitle("Sub-network correlations");

	makeList();
}

void ViewCorrelations::makeList()
{
	LineGroup *lg = new LineGroup(_clique, this);
	lg->setLeft(0.1, 0.23);

	ScrollBox *sb = new ScrollBox();
	sb->setContent(lg);
	sb->setBounds(glm::vec4(0.23, 0.1, 0.35, 0.9));
	addObject(sb);

}
