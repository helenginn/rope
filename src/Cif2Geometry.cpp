#include <iostream>
#include <gemmi/numb.hpp>

#include "Cif2Geometry.h"
#include "FileReader.h"
#include "GeometryTable.h"
#include "Atom.h"
#include "AtomGroup.h"
#include "glm_import.h"

using namespace gemmi::cif;

Cif2Geometry::Cif2Geometry(std::string filename)
{
	_filename = filename;
	_table = new GeometryTable();
	_atoms = new AtomGroup();
	_accessedAtoms = false;
	_accessedTable = false;
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
	Document doc = read_file(_filename);

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
}

void Cif2Geometry::processLoop(Loop &loop)
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
		return;
	}
	
	for (size_t i = 0; i < loop.values.size(); i += loop.tags.size())
	{
		if (_code.length() && loop.values[i + code_idx] != _code)
		{
			continue;
		}
		
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
		
		_atoms->add(a);
	}

}
