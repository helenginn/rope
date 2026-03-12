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
#include "Mesh.h"
#include <vagabond/core/EntityManager.h>
#include <vagabond/core/Metadata.h>
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

std::string Antigen::validate() const
{
	if (title == "")
	{
		return "Model title/filename missing for antigen";
	}
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
	auto validate_antigen = [](const Antigen &antigen)
	{
		return antigen.validate();
	};

	return MabUtils::gather_validations<Antigen>
	(validate_antigen, *this);
}

OpSet<std::string> Antigens::entities() const
{
	OpSet<std::string> entities;
	for (const Antigen &antigen : *this)
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

std::string Competition::validate(const Antigens &antigens) const
{
	if (filename.length() == 0)
	{
		return "Competition data table missing filename";
	}
	if (!metadata)
	{
		return "Competition data table " + filename + " could not be loaded";
	}
	if (left_header.length() == 0)
	{
		return "No first antibody header for " + filename;
	}
	if (right_header.length() == 0)
	{
		return "No second antibody header for " + filename;
	}
	if (value_header.length() == 0)
	{
		return "No data value header for " + filename;
	}
	if (!metadata->areAllNumbers(value_header))
	{
		return "CSV values for header " + value_header + 
		" in \n" + filename + " are mixed text/numbers and "\
		"cannot be used as experimental data";
	}
	if (!antigen.length())
	{
		return "No antigen model listed as subject for " + filename;
	}
	bool found = false;
	for (const Antigen &atgn : antigens)
	{
		if (atgn.title == antigen)
		{
			found = true; break;
		}
	}
	if (!found)
	{
		return "Antigen model \"" + antigen + "\" for " + filename
		+ " does not exist anymore";
	}

	return "";
}

std::string Fiducials::validate(const Antigens &antigens)
{
	auto validate_fiducial = [antigens](Fiducial &fid)
	{
		return fid.validate(antigens);
	};

	return MabUtils::gather_validations<Fiducial>
	(validate_fiducial, *this);
}

std::string Competitions::validate(const Antigens &antigens)
{
	auto validate_comp = [antigens](const Competition &comp)
	{
		return comp.validate(antigens);
	};

	return MabUtils::gather_validations<Competition>
	(validate_comp, *this);
}

OpSet<std::string> Competitions::all_antibodies()
{
	OpSet<std::string> all;
	for (Competition &comp : *this)
	{
		TabulatedData *data = comp.metadata->asData();
		all += data->all_options(comp.left_header);
		all += data->all_options(comp.right_header);
	}
	return all;
}

OpSet<std::string> Fiducials::all_fiducials()
{
	OpSet<std::string> all;
	for (Fiducial &fid : *this)
	{
		all += fid.name;
	}
	return all;
}

OpSet<std::string> Fiducial::non_antigen_entities
(const Antigens &antigens)
{
	OpSet<std::string> all = entities;
	OpSet<std::string> excluded = antigens.entities();
	all -= excluded;
	return all;
}

std::string Fiducial::validate(const Antigens &antigens)
{
	if (name == "")
	{
		return "Antibody is not chosen";
	}

	if (model.filename() == "")
	{
		return "Antibody " + name + " model PDB not chosen";
	}
	
	if (antigen == "")
	{
		return "Antigen model not assigned for " + name;
	}

	const Antigen *found = antigens.antigen(antigen);
	if (!found)
	{
		return "Antigen \"" + antigen + "\" assigned to " + name
		+ " does not exist anymore";
	}
	
	OpSet<std::string> model_names = model.entity_names();
	OpSet<std::string> antigen_names = found->entities;

	if (model_names.size() == 0)
	{
		return "No chain assignment in " + name;
	}
	
	for (const std::string &antigen_name : antigen_names)
	{
		if (model_names.count(antigen_name) == 0)
		{
			return "Antibody " + name + " is missing chain "\
			"assignment of "\
			"antigen component \"" + antigen_name + "\"";
		}
	}
	
	model_names -= antigen_names;
	if (model_names.size() == 0)
	{
		return "Antibody " + name + " has no non-antigen entities"
		+ " assigned";
	}
	
	if (non_antigen_entities(antigens).size() == 0)
	{
		return "Antibody " + name + " has non-antigen entities"
		+ " assigned, but\nall removed from definition of antibody";
	}

	return "";
}

const Antigen *Antigens::antigen(const std::string &id) const
{
	for (const Antigen &antigen : *this)
	{
		if (antigen.title == id)
		{
			return &antigen;
		}
	}
	return nullptr;
}

std::vector<Instance *> Antigen::instances()
{
	model.load();
	std::vector<Instance *> instances = model.instances();
	std::vector<Instance *> antigen_only;
	
	for (Instance *const &inst : instances)
	{
		if (entities.count(inst->entity_id()))
		{
			antigen_only.push_back(inst);
		}
	}
	
	model.unload();
	return antigen_only;

}

Mesh *Antigen::mesh()
{
	if (!_mesh)
	{
		model.load();
		_mesh = new Mesh(*this);
		model.unload();
	}
	return _mesh;
}
