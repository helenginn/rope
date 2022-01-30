#include <iostream>
#include <gemmi/numb.hpp>
#include "commit.h"

#include "Cif2Geometry.h"
#include "Knotter.h"
#include "FileReader.h"
#include "GeometryTable.h"
#include "Atom.h"
#include "AtomGroup.h"
#include "glm_import.h"

using namespace gemmi::cif;

Cif2Geometry::Cif2Geometry(std::string filename)
{
	_filename = filename;
#ifndef __EMSCRIPTEN__
	_filename = std::string(DATA_DIRECTORY) + "/" + filename;
#endif

	_table = new GeometryTable();
	_atoms = new AtomGroup();
	_accessedAtoms = false;
	_accessedTable = false;
	_knot = true;
}

Cif2Geometry::~Cif2Geometry()
{
	if (!_accessedAtoms)
	{
		delete _atoms;
	}

	else if (!_accessedTable)
	{
		delete _table;
	}
}

void Cif2Geometry::parse()
{
	Document doc;
	try
	{
		doc = read_file(_filename);
	}
	catch (std::runtime_error err)
	{
		std::cout << "Could not load file." << std::endl;
		std::cout << err.what() << std::endl;
		return;
	}

	for (size_t j = 0; j < doc.blocks.size(); j++)
	{
		Block &contents = doc.blocks[j];
		for (size_t i = 0; i < contents.items.size(); i++)
		{
			if (contents.items[i].type == ItemType::Loop)
			{
				Loop &loop = contents.items[i].loop;

				processLoop(loop);
			}
		}
	}
	
	if (_knot)
	{
		Knotter knotter(_atoms, _table);
		knotter.knot();

	}
}

void Cif2Geometry::processLoop(Loop &loop)
{
	if (processLoopAsAtoms(loop))
	{
		return;
	}

	if (processLoopAsLengths(loop))
	{
		return;
	}

	if (processLoopAsAngles(loop))
	{
		return;
	}

	if (processLoopAsTorsions(loop))
	{
		return;
	}

	if (processLoopAsChirals(loop))
	{
		return;
	}
}

bool Cif2Geometry::processLoopAsChirals(Loop &loop)
{
	int code_idx = -1;
	int ctr_idx = -1;
	int id1_idx = -1;
	int id2_idx = -1;
	int id3_idx = -1;
	int sign_idx = -1;
	
	for (size_t j = 0; j < loop.tags.size(); j++)
	{
		if (loop.tags[j] == "_chem_comp_chir.comp_id")
		{
			code_idx = j;
		}
		if (loop.tags[j] == "_chem_comp_chir.atom_id_centre")
		{
			ctr_idx = j;
		}
		if (loop.tags[j] == "_chem_comp_chir.atom_id_1")
		{
			id1_idx = j;
		}
		else if (loop.tags[j] == "_chem_comp_chir.atom_id_2")
		{
			id2_idx = j;
		}
		else if (loop.tags[j] == "_chem_comp_chir.atom_id_3")
		{
			id3_idx = j;
		}
		else if (loop.tags[j] == "_chem_comp_chir.volume_sign")
		{
			sign_idx = j;
		}
	}

	if (code_idx < 0 || ctr_idx < 0 || id1_idx < 0 || id2_idx < 0
	    || id3_idx < 0 || sign_idx < 0)
	{
		return false;
	}
	
	for (size_t i = 0; i < loop.values.size(); i += loop.tags.size())
	{
		if (_code.length() && loop.values[i + code_idx] != _code)
		{
			continue;
		}
		
		std::string code = loop.values[i + code_idx];
		std::string centre = loop.values[i + ctr_idx];
		std::string p = loop.values[i + id1_idx];
		std::string q = loop.values[i + id2_idx];
		std::string r = loop.values[i + id3_idx];
		std::string sign_str = (loop.values[i + sign_idx]);
		to_lower(sign_str);

		int sign = 0;
		if (sign_str == "positive")
		{
			sign = 1;
		}
		else if (sign_str == "negative")
		{
			sign = -1;
		}

		_table->addGeometryChiral(code, centre, p, q, r, sign);
	}
	
	return true;
}

bool Cif2Geometry::processLoopAsAtoms(Loop &loop)
{
	int code_idx = -1;
	int x_idx = -1;
	int y_idx = -1;
	int z_idx = -1;
	int ele_idx = -1;
	int atom_name_idx = -1;
	
	for (size_t j = 0; j < loop.tags.size(); j++)
	{
		if (loop.tags[j] == "_chem_comp_atom.comp_id")
		{
			code_idx = j;
		}
		if (loop.tags[j] == "_chem_comp_atom.x")
		{
			x_idx = j;
		}
		else if (loop.tags[j] == "_chem_comp_atom.y")
		{
			y_idx = j;
		}
		else if (loop.tags[j] == "_chem_comp_atom.z")
		{
			z_idx = j;
		}
		else if (loop.tags[j] == "_chem_comp_atom.atom_id")
		{
			atom_name_idx = j;
		}
		else if (loop.tags[j] == "_chem_comp_atom.type_symbol")
		{
			ele_idx = j;
		}
	}
	
	if (code_idx < 0 || x_idx < 0 || y_idx < 0 || 
	    z_idx < 0 || ele_idx < 0 || atom_name_idx < 0)
	{
		return false;
	}
	
	for (size_t i = 0; i < loop.values.size(); i += loop.tags.size())
	{
		if (_code.length() && loop.values[i + code_idx] != _code)
		{
			continue;
		}
		
		std::string code = loop.values[i + code_idx];
		glm::vec3 pos = glm::vec3(0.);
		pos.x = as_number(loop.values[i + x_idx]);
		pos.y = as_number(loop.values[i + y_idx]);
		pos.z = as_number(loop.values[i + z_idx]);

		std::string ele = loop.values[i + ele_idx];
		std::string name = loop.values[i + atom_name_idx];

		Atom *a = new Atom();
		a->setElementSymbol(ele);
		a->setAtomName(name);
		a->setInitialPosition(pos, 30);
		a->setCode(code);
		
		_atoms->add(a);
	}
	
	return true;
}

bool Cif2Geometry::processLoopAsAngles(Loop &loop)
{
	int code_idx = -1;
	int id1_idx = -1;
	int id2_idx = -1;
	int id3_idx = -1;
	int angle_idx = -1;
	int dev_idx = -1;
	
	for (size_t j = 0; j < loop.tags.size(); j++)
	{
		if (loop.tags[j] == "_chem_comp_angle.comp_id")
		{
			code_idx = j;
		}
		else if (loop.tags[j] == "_chem_comp_angle.atom_id_1")
		{
			id1_idx = j;
		}
		else if (loop.tags[j] == "_chem_comp_angle.atom_id_2")
		{
			id2_idx = j;
		}
		else if (loop.tags[j] == "_chem_comp_angle.atom_id_3")
		{
			id3_idx = j;
		}
		else if (loop.tags[j] == "_chem_comp_angle.value_angle")
		{
			angle_idx = j;
		}
		else if (loop.tags[j] == "_chem_comp_angle.value_angle_esd")
		{
			dev_idx = j;
		}
	}
	
	if (code_idx < 0 || id1_idx < 0 || id2_idx < 0 || id3_idx < 0 || 
	    angle_idx < 0 || dev_idx < 0)
	{
		return false;
	}
	
	for (size_t i = 0; i < loop.values.size(); i += loop.tags.size())
	{
		if (_code.length() && loop.values[i + code_idx] != _code)
		{
			continue;
		}
		
		std::string code = loop.values[i + code_idx];
		std::string p = loop.values[i + id1_idx];
		std::string q = loop.values[i + id2_idx];
		std::string r = loop.values[i + id3_idx];
		float angle = as_number(loop.values[i + angle_idx]);
		float dev = as_number(loop.values[i + dev_idx]);

		_table->addGeometryAngle(code, p, q, r, angle, dev);
	}
	
	return true;
}

bool Cif2Geometry::processLoopAsLengths(Loop &loop)
{
	int code_idx = -1;
	int id1_idx = -1;
	int id2_idx = -1;
	int dist_idx = -1;
	int dev_idx = -1;
	
	for (size_t j = 0; j < loop.tags.size(); j++)
	{
		if (loop.tags[j] == "_chem_comp_bond.comp_id")
		{
			code_idx = j;
		}
		else if (loop.tags[j] == "_chem_comp_bond.atom_id_1")
		{
			id1_idx = j;
		}
		else if (loop.tags[j] == "_chem_comp_bond.atom_id_2")
		{
			id2_idx = j;
		}
		else if (loop.tags[j] == "_chem_comp_bond.value_dist")
		{
			dist_idx = j;
		}
		else if (loop.tags[j] == "_chem_comp_bond.value_dist_esd")
		{
			dev_idx = j;
		}
	}
	
	if (code_idx < 0 || id1_idx < 0 || id2_idx < 0 || 
	    dist_idx < 0 || dev_idx < 0)
	{
		return false;
	}
	
	for (size_t i = 0; i < loop.values.size(); i += loop.tags.size())
	{
		if (_code.length() && loop.values[i + code_idx] != _code)
		{
			continue;
		}
		
		std::string code = loop.values[i + code_idx];
		std::string p = loop.values[i + id1_idx];
		std::string q = loop.values[i + id2_idx];
		float length = as_number(loop.values[i + dist_idx]);
		float dev = as_number(loop.values[i + dev_idx]);

		_table->addGeometryLength(code, p, q, length, dev);
	}
	
	return true;
}

bool Cif2Geometry::processLoopAsTorsions(Loop &loop)
{
	int code_idx = -1;
	int id1_idx = -1;
	int id2_idx = -1;
	int id3_idx = -1;
	int id4_idx = -1;
	int angle_idx = -1;
	int dev_idx = -1;
	int period_idx = -1;
	
	for (size_t j = 0; j < loop.tags.size(); j++)
	{
		if (loop.tags[j] == "_chem_comp_tor.comp_id")
		{
			code_idx = j;
		}
		else if (loop.tags[j] == "_chem_comp_tor.atom_id_1")
		{
			id1_idx = j;
		}
		else if (loop.tags[j] == "_chem_comp_tor.atom_id_2")
		{
			id2_idx = j;
		}
		else if (loop.tags[j] == "_chem_comp_tor.atom_id_3")
		{
			id3_idx = j;
		}
		else if (loop.tags[j] == "_chem_comp_tor.atom_id_4")
		{
			id4_idx = j;
		}
		else if (loop.tags[j] == "_chem_comp_tor.value_angle")
		{
			angle_idx = j;
		}
		else if (loop.tags[j] == "_chem_comp_tor.value_angle_esd")
		{
			dev_idx = j;
		}
		else if (loop.tags[j] == "_chem_comp_tor.period")
		{
			period_idx = j;
		}
	}
	
	if (code_idx < 0 || id1_idx < 0 || id2_idx < 0 || id3_idx < 0 || 
	    id4_idx < 0 || angle_idx < 0 || dev_idx < 0 || period_idx < 0)
	{
		return false;
	}
	
	for (size_t i = 0; i < loop.values.size(); i += loop.tags.size())
	{
		if (_code.length() && loop.values[i + code_idx] != _code)
		{
			continue;
		}
		
		std::string code = loop.values[i + code_idx];
		std::string p = loop.values[i + id1_idx];
		std::string q = loop.values[i + id2_idx];
		std::string r = loop.values[i + id3_idx];
		std::string s = loop.values[i + id4_idx];
		float angle = as_number(loop.values[i + angle_idx]);
		float dev = as_number(loop.values[i + dev_idx]);
		int period = as_number(loop.values[i + period_idx]);

		_table->addGeometryTorsion(code, p, q, r, s, angle, dev, period);
	}
	
	return true;
}

const size_t Cif2Geometry::atomCount() const
{
	return _atoms->size();
}
