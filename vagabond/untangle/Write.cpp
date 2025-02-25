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

#include "Write.h"
#include <vagabond/core/Atom.h>
#include <vagabond/core/AtomGroup.h>
#include <vagabond/utils/FileReader.h>
#include <gemmi/mmread.hpp>
#include <gemmi/to_pdb.hpp>
#include <iostream>
#include <fstream>

Write::Write(const std::string &pattern, 
             const std::string &output, AtomGroup *group)
: _group(group), _patternFile(pattern), _outputFile(output)
{

}

void Write::processModel(gemmi::Model &m)
{
	for (size_t i = 0; i < m.chains.size(); i++)
	{
		gemmi::Chain &c = m.chains[i];
		for (size_t j = 0; j < c.residues.size(); j++)
		{
			gemmi::Residue &r = c.residues[j];

			for (size_t k = 0; k < r.atoms.size(); k++)
			{
				gemmi::Atom &a = r.atoms[k];

				processAtom(c, r, a);
			}
		}
	}
}

void Write::processAtom(gemmi::Chain &c, gemmi::Residue &r, gemmi::Atom &a)
{
	std::string resid = r.name; trim(resid);
	std::string res = r.seqid.num.str() + r.seqid.icode; trim(res);
	std::string name = a.name; trim(name);

	std::string desc = c.name + "-" + resid + res + ":" + name;
	
	Atom *found = _group->atomByDesc(desc);
	
	if (!found)
	{
		std::cout << "Could not find " << desc << " for writing" << std::endl;
		return;
	}

	std::string search = "";
	if (a.altloc) search += a.altloc;
	
	try
	{
		Atom::AtomPlacement &results = found->conformerPositions().at(search);
		a.aniso.u11 = results.tensor[0][0];
		a.aniso.u12 = results.tensor[0][1];
		a.aniso.u13 = results.tensor[0][2];
		a.aniso.u22 = results.tensor[1][1];
		a.aniso.u23 = results.tensor[1][2];
		a.aniso.u33 = results.tensor[2][2];
		a.occ = results.occ;
		a.b_iso = results.b;
		a.pos.x = results.pos.ave.x;
		a.pos.y = results.pos.ave.y;
		a.pos.z = results.pos.ave.z;
	}
	catch (...)
	{
		std::cout << "Could not find atom conformer " << search << " for"
		<< " atom " << desc << std::endl;
		return;
	}
}

void Write::operator()()
{
	gemmi::Structure st;
	try
	{
		st = gemmi::read_structure_file(_patternFile);
	}
	catch (const std::runtime_error &err)
	{
		std::cout << "Reading original filename: " << _patternFile << " failed: " << std::endl;
		std::cout << err.what() << std::endl;
		return;
	}

	setup_entities(st);

	for (size_t i = 0; i < st.models.size(); i++)
	{
		processModel(st.models[i]);
	}

	for (size_t i = 0; i < st.models.size(); i++)
	{
		processModel(st.models[i]);
	}

	std::ofstream file;
	file.open(_outputFile);
	
	gemmi::write_pdb(st, file);
	
	file.close();
}
