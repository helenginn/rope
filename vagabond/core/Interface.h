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
#include "Comparable.h"

class Interface
{
public:
	Interface(Model *left, Comparable *right);
	Interface() {};

	Interface *expandedInterface(Interface *face);

	void addInteraction(Interaction &ia);
	std::string desc();
	
	void loadModel();
private:
	std::list<Interaction> _interactions;

	Model *_left = nullptr;
	Comparable *_right = nullptr;
};

#endif
