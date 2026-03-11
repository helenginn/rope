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

#include "Mab.h"
#include <vagabond/core/EntityManager.h>
#include <vagabond/core/Entity.h>
#include <vagabond/utils/maths.h>

void ColourMap::recalculate()
{
	mapping.clear();
	EntityManager *manager = Environment::entityManager();

	for (int i = 0; i < manager->objectCount(); i++)
	{
		std::string name = manager->object(i).name();
		float hue = (float)(i * 1.618 * 90) + 180;
		hue = fmod(hue, 360.f);
		float r = hue; float g = 30; float b = 70;
		hsv_to_rgb(r, g, b);
		mapping[name] = {r, g, b};
		mapping[name] -= glm::vec3(0.7, 0.7, 0.7);
	}
}

std::string Antigen::validate()
{
	if (model.filename() == "")
	{
		return "No model PDB chosen for " + title;
	}

	if (entities.size() == 0)
	{
		return "No entities assigned for " + title;
	}
	
	for (const std::string &ent : entities)
	{
		if (!model.hasEntity(ent))
		{
			return "Model missing assigned entity " + ent + " in "
			+ title;
		}
	}
	
	return "";
}

std::string Antigens::validate()
{
	std::string problems;
	for (Antigen &antigen : *this)
	{
		std::string contribution = antigen.validate();
		if (contribution.length())
		{
			problems += contribution;
			problems += "\n";
		}
	}
	if (problems.length() > 0)
	{
		problems.pop_back();
	}
	return problems;
}

OpSet<std::string> Antigens::entities()
{
	OpSet<std::string> entities;
	for (Antigen &antigen : *this)
	{
		entities += antigen.entities;
	}

	return entities;
}

std::string Competition::interpretation_as_desc()
{
	std::string str = (as_competition ? "competition, " : "binding, ");
	str += "0-" + f_to_str(scale, 0);
	return str;
}
