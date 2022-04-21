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

#include <gemmi/mmread.hpp>
#include "PdbFile.h"
#include "CifFile.h"
#include "AtomGroup.h"
#include "Atom.h"
#include "Knotter.h"

PdbFile::PdbFile(std::string filename) : File(filename)
{

}

void PdbFile::getStandardGeometry()
{
	CifFile cf("assets/geometry/standard_geometry.cif");
	cf.parse();
	_table = cf.geometryTable();
}

void PdbFile::processAtom(gemmi::Atom &a, AtomInfo &ai)
{
	if (a.altloc != '\0' && a.altloc != 'A')
	{
		return;
	}
	Atom *vagatom = new Atom();
	vagatom->setElementSymbol(a.element.name());
	vagatom->setAtomName(a.name);
	vagatom->setResidueId(ai.resid);
	vagatom->setChain(ai.chain);
	vagatom->setCode(ai.res);
	glm::vec3 pos = glm::vec3(a.pos.x, a.pos.y, a.pos.z);
	vagatom->setInitialPosition(pos, a.b_iso);
	
	*_macroAtoms += vagatom;
}

void PdbFile::processResidue(gemmi::Residue &r, AtomInfo &ai)
{
	ai.res = r.name;
	ai.resid = r.seqid.num.str() + r.seqid.icode;
	ai.seqstr = r.seqid.str();
	ai.isHetatm = (r.het_flag == 'H');
	
	for (size_t i = 0; i < r.atoms.size(); i++)
	{
		gemmi::Atom &a = r.atoms[i];
		processAtom(a, ai);
	}
}

void PdbFile::processModel(gemmi::Model &m)
{
	for (size_t i = 0; i < m.chains.size(); i++)
	{
		gemmi::Chain &c = m.chains[i];
		std::string ch_id = c.name;

		for (size_t j = 0; j < c.residues.size(); j++)
		{
			AtomInfo ai{};
			ai.chain = ch_id;
			processResidue(c.residues[j], ai);
		}
	}
}

void PdbFile::parseFileContents()
{
	std::string path = toFilename(_filename);

	gemmi::Structure st = gemmi::read_structure_file(path);
	setup_entities(st);
	
	for (size_t i = 0; i < st.entities.size(); i++)
	{
		std::vector<std::string> seq = st.entities[i].full_sequence;

		std::cout << st.entities[i].name << std::endl;
		
		for (size_t j = 0; j < seq.size(); j++)
		{
			std::cout << seq[j] << " ";
		}
		std::cout << std::endl;
	}

	for (size_t i = 0; i < st.models.size(); i++)
	{
		processModel(st.models[i]);
	}
	
	std::cout << "This file has " << atomCount() << " atoms.\n";
	std::string spgname = st.spacegroup_hm;
	_values["_symmetry.space_group_name_H-M"] = spgname;

	_values["cell.length_a"] = st.cell.a;
	_values["cell.length_b"] = st.cell.b;
	_values["cell.length_c"] = st.cell.c;
	_values["cell.angle_alpha"] = st.cell.alpha;
	_values["cell.angle_beta"] = st.cell.beta;
	_values["cell.angle_gamma"] = st.cell.gamma;
}

void PdbFile::parse()
{
	parseFileContents();
	
	if (_knot && _macroAtoms->size() > 0)
	{
		getStandardGeometry();
	}
	
	if (_knot)
	{
		AtomGroup *ptr = (_macroAtoms->size() == 0 ? _compAtoms : _macroAtoms);
		Knotter knotter(ptr, _table);
		knotter.knot();
	}
}

File::Type PdbFile::cursoryLook()
{
	std::string path = toFilename(_filename);
	gemmi::Structure st = gemmi::read_structure_file(path);
	Type hasAtoms = (st.models.size() > 0 ? MacroAtoms : Nothing);
	
	std::string spgname = st.spacegroup_hm;
	Type hasSymmetry = (spgname.length() ? Symmetry : Nothing);
	
	Type hasCell = (st.cell.a > 0 ? UnitCell : Nothing);

	File::Type type = File::Type(hasAtoms | hasSymmetry | hasCell);
	
	return type;
}

