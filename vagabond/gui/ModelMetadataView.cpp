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

#include <vagabond/gui/elements/Text.h>
#include "ModelMetadataView.h"
#include "Model.h"

ModelMetadataView::ModelMetadataView(Scene *prev, Model &model) :
ListView(prev), _model(model)
{
	_kv = _model.metadata();

}

void ModelMetadataView::setup()
{
	addTitle("Metadata for " + _model.name());
	
	ListView::setup();
}


Renderable *ModelMetadataView::getLine(int i)
{
	Metadata::KeyValues::const_iterator it;
	int count = 0;
	std::string first, last;
	bool number = false;
	
	for (it = _kv.cbegin(); it != _kv.cend(); it++)
	{
		if (count == i)
		{
			first = it->first;
			last = it->second.text();
			number = it->second.hasNumber();
		}

		count++;
	}

	Box *b = new Box();
	
	std::string header = first + (number ? " (#)" : "");

	Text *h = new Text(header);
	h->setLeft(0.0, 0.);
	b->addObject(h);
	
	Text *v = new Text(last);
	v->setRight(0.6, 0.);
	b->addObject(v);
	
	return b;
}

size_t ModelMetadataView::lineCount()
{
	return _kv.size();
}
