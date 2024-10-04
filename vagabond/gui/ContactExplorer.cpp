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

#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/TableView.h>
#include "ContactExplorer.h"
#include "MatrixPlot.h"

ContactExplorer::ContactExplorer(Scene *prev, Contacts &contacts) 
: Scene(prev), _contacts(contacts)
{

}

void ContactExplorer::mousePressEvent(double x, double y, 
                                      SDL_MouseButtonEvent button)
{
	double tx = x; double ty = y;
	convertToGLCoords(&tx, &ty);

	glm::vec3 v = glm::vec3(tx, ty, 0);
	glm::vec3 min, max;
	_plot->boundaries(&min, &max);

	v -= min;
	v /= (max - min);
	v.z = 0;

	if ((v.x < 0 || v.x > 1) || (v.y < 0 || v.y > 1))
	{
		Scene::mousePressEvent(x, y, button);
		return;
	}

	int diff = _end - _start + 1;
	int left = v.x * diff + _start;
	int right = v.y * diff + _start;
	setInformation(std::to_string(left) + " vs. " + std::to_string(right));
	Scene::mousePressEvent(x, y, button);
}

void ContactExplorer::setup()
{
	if (_contacts.totals.size() == 0)
	{
		return;
	}

	auto bt = _contacts.totals.begin();
	auto et = _contacts.totals.end();
	et--;
	int start = bt->first.as_num();
	int end = et->first.as_num();
	_start = start;
	_end = end;
	int diff = end - start + 1;
	
	_data = new TabulatedData({{"left", TabulatedData::Number},
		                         {"right", TabulatedData::Number},
		                         {"value", TabulatedData::Number}});
	PCA::setupMatrix(&_matrix, diff, diff);
	
	for (auto it = _contacts.totals.begin(); it != _contacts.totals.end(); it++)
	{
		for (auto jt = it->second.begin(); jt != it->second.end(); jt++)
		{
			int l = it->first.as_num();
			int r = jt->first.as_num();
			_matrix[l - start][r - start] += jt->second * 4;
			
			if (l < r)
			{
				_data->addEntry({{"left", std::to_string(l)},
					               {"right", std::to_string(r)},
				                {"value", std::to_string(jt->second)}});
			}
		}
	}

	_plot = new MatrixPlot(_matrix, _mutex);
	addObject(_plot);
	
	TextButton *button = new TextButton("View table", this);
	button->setReturnJob(
	[this]()
	{
		TableView *view = new TableView(this, _data, "Contact values");
		view->show();
	});
	button->setCentre(0.5, 0.8);
	addObject(button);
}
