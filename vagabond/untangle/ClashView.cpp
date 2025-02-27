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

#include "ClashView.h"
#include "NonBonds.h"
#include <vagabond/core/AtomGroup.h>
#include <vagabond/gui/MatrixPlot.h>

ClashView::ClashView(Scene *prev, AtomGroup *group, NonBonds *nonBonds) :
Scene(prev), _group(group), _nonBonds(nonBonds)
{

}

void ClashView::setup()
{
	addTitle("Clash view");

	std::set<ChainResi> ids = _nonBonds->residueIds();

	setupMatrix(&_mat, ids.size(), ids.size());
	
	int i = 0; int j = 0;
	std::map<int, ChainResi> convert;

	for (const ChainResi &id_a : ids)
	{
		for (const ChainResi &id_b : ids)
		{
			if (abs(i - j) <= 1)
			{
				i++;
				continue;
			}

			float score = _nonBonds->evaluateResiduePair(id_a, id_b);
			_mat[i][j] = 1 - exp(-score*score / 2.f);
			i++;
		}
		convert[j] = id_a;
		j++; i = 0;
	}
	
	MatrixPlot *plot = new MatrixPlot(_mat, _mutex);
	plot->update();
	plot->setCentre(0.5, 0.5);
	
	auto report_residue = [this, convert](float x, float y)
	{
		x *= (float)convert.size();
		y *= (float)convert.size();
		
		int i = x;
		int j = y;
		
		try
		{
			std::string str = convert.at(i).ch + convert.at(i).id.str();
			str += " to " + convert.at(j).ch + convert.at(j).id.str();
			setInformation(str);
		}
		catch (...)
		{
			return;
		}
	};

	plot->setHoverJob(report_residue);
	addObject(plot);
}


