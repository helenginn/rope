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

#ifndef __vagabond__IconLegend__
#define __vagabond__IconLegend__

#include "ClusterPointDemo.h"
#include <vagabond/gui/elements/ButtonResponder.h>

class Rule;
class Scene;

class IconLegend : public ClusterPointDemo, public ButtonResponder
{
public:
	IconLegend(Scene *responder);
	~IconLegend();
	
	void addRule(const Rule *r);

	virtual void makePoints();
	virtual void buttonPressed(std::string tag, Button *button);
private:
	std::vector<const Rule *> _rules;
	Scene *_responder = nullptr;
};

#endif
