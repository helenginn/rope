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

#ifndef __vagabond__Interface__
#define __vagabond__Interface__

#include <list>
#include "Interaction.h"
#include "Instance.h"

class Interface
{
public:
	Interface(Instance *left, Instance *right);
	Interface() {};

	Interface *expandedInterface(Interface *face);

	void addInteraction(Interaction &ia);
	std::string desc();
	
	const size_t size() const
	{
		return _interactions.size();
	}
	
	const std::list<Interaction> &interactions() const
	{
		return _interactions;
	}
	
	void loadModel();
private:
	std::list<Interaction> _interactions;

	Instance *_left = nullptr;
	Instance *_right = nullptr;
};

#endif
