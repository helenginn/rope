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

OpSet<std::string> Competition::antibody_names()
{
	OpSet<std::string> all;
	TabulatedData *data = metadata->asData();
	all += data->all_options(left_header);
	all += data->all_options(right_header);
	return all;
}

OpSet<std::string> Competitions::all_antibodies()
{
	OpSet<std::string> all;
	for (Competition &comp : *this)
	{
		all += comp.antibody_names();
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

Antigen *Antigens::antigen(const std::string &id) 
{
	for (Antigen &antigen : *this)
	{
		if (antigen.title == id)
		{
			return &antigen;
		}
	}
	return nullptr;
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

void Mab::save()
{
	json data;
	data["entity_manager"] = *Environment::entityManager();
	data["mabscape"] = *this;

	std::ofstream file;
	file.open("mabscape.json");
	file << data.dump(1);
	file << std::endl;
	file.close();
}

void Mab::load()
{
	if (!file_exists("mabscape.json"))
	{
		std::cout << "Could not find json environment; "\
		"starting new one." << std::endl;
		return;
	}

	json data;
	std::ifstream f;
	f.open("mabscape.json");
	f >> data;
	f.close();

	*Environment::entityManager() = data["entity_manager"];
	Environment::entityManager()->housekeeping();
	*this = data["mabscape"];

	housekeeping();
}

void Mab::housekeeping()
{
	colours.recalculate();

	for (Antigen &a : antigens)
	{
		a.housekeeping();
	}

	for (Fiducial &f : fiducials)
	{
		f.housekeeping();
	}
}

void Antigen::housekeeping()
{
	model.housekeeping();
}

void Fiducial::housekeeping()
{
	model.housekeeping();
}

std::vector<std::string> 
Competition::clean_order(const std::vector<std::string> &order)
{
	OpSet<std::string> all = antibody_names();
	std::vector<std::string> finalised;

	for (const std::string &ab : order)
	{
		if (all.count(ab))
		{
			finalised.push_back(ab);
			all -= ab;
		}
	}
	
	for (const std::string &ab : all)
	{
		finalised.push_back(ab);
	}
	return finalised;
}

Eigen::MatrixXf Competition::make_plot(std::vector<std::string> &order)
{
	std::vector<std::string> all = clean_order(order);

	Eigen::MatrixXf mat(all.size(), all.size());

	for (int i = 0; i < all.size(); i++)
	{
		const std::string &a = all[i];
		for (int j = 0; j < all.size(); j++)
		{
			const std::string &b = all[j];
			float val = value(a, b);
			mat(i, j) = val;
		}

	}
	return mat;
}

TabulatedData *Competition::asData()
{
	if (!metadata)
	{
		return nullptr;
	}

	if (!_asData)
	{
		_asData = metadata->asData();
	}

	return _asData;
}

float Competition::value(const std::string &ab1, const std::string &ab2)
{
	if (ab1 == ab2)
	{
		return NAN;
	}

	TabulatedData *data = asData();

	int l = data->indexForHeader(left_header);
	int r = data->indexForHeader(right_header);
	int v = data->indexForHeader(value_header);
	
	if (l < 0 || r < 0 || v < 0) 
	{
		return NAN;
	}
	
	float sum = 0;
	float count = 0;
	for (int i = 0; i < data->entryCount(); i++)
	{
		std::vector<std::string> vals = data->entry(i);
		if ((vals[l] == ab1 && vals[r] == ab2) ||
		    (vals[l] == ab2 && vals[r] == ab1))
		{
			sum += atof(vals[v].c_str());
			count++;
		}
	}
	
	float ave = sum / count;
	adjust_value(ave);
	return ave;
}

void Competition::adjust_value(float &val)
{
	val /= scale;
	if (!as_competition)
	{
		val = 1 - val;
	}
}
