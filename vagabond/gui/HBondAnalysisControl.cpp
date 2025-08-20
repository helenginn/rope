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

#include "HBondAnalysisControl.h"
#include <vagabond/gui/elements/TextButton.h>

HBondAnalysisControl::HBondAnalysisControl(Scene *prev, Clique *clique)
: Scene(prev), _clique(clique)
{

}

void HBondAnalysisControl::setup()
{
	addTitle("Analysis overview");

	{
		TextButton *tb = new TextButton("Choose communication points", this);
		tb->setLeft(0.2, 0.3);
		addObject(tb);
	}

	{
		TextButton *tb = new TextButton("Subdivide network", this);
		tb->setLeft(0.2, 0.4);
		tb->setInert(true, true);
		addObject(tb);
	}

	{
		TextButton *tb = new TextButton("Exhaustive search", this);
		tb->setLeft(0.2, 0.5);
		tb->setInert(true, true);
		addObject(tb);
	}

	{
		TextButton *tb = new TextButton("Correlations", this);
		tb->setLeft(0.2, 0.6);
		tb->setInert(true, true);
		addObject(tb);
	}

	{
		TextButton *tb = new TextButton("Prepare groups for path", this);
		tb->setLeft(0.2, 0.7);
		tb->setInert(true, true);
		addObject(tb);
	}

	{
		TextButton *tb = new TextButton("Best path search", this);
		tb->setLeft(0.2, 0.8);
		tb->setInert(true, true);
		addObject(tb);
	}
}
