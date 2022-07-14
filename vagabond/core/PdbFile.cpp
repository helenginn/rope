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

#define GEMMI_WRITE_IMPLEMENTATION

#include <gemmi/mmread.hpp>
#include <gemmi/to_pdb.hpp>
#include <fstream>
#include "Environment.h"
#include "FileManager.h"
#include "PdbFile.h"
#include "CifFile.h"
#include "AtomGroup.h"
#include "Atom.h"
#include "Knotter.h"

PdbFile::PdbFile(std::string filename) : File(filename)
{

}

void PdbFile::getAllGeometry()
{
	getStandardGeometry();

	FileManager *fm = Environment::fileManager();
	fm->setFilterType(File::Geometry);

	for (size_t i = 0; i < fm->filteredCount(); i++)
	{
		std::string file = fm->filtered(i);
		std::cout << "Using " << file << std::endl;
		CifFile cf(file);
		cf.setGeometryTable(_table);
		cf.parse();
	}
}

void PdbFile::getStandardGeometry()
{
	CifFile cf("assets/geometry/standard_geometry.cif");
	cf.parse();

	if (_table != nullptr)
	{
		delete _table;
		_table = nullptr;
	}

	_table = cf.geometryTable();
}

void PdbFile::processAtom(gemmi::Atom &a, AtomInfo &ai, char conf)
{
	if (a.altloc != conf && a.altloc != '\0')
	{
		return;
	}

	Atom *vagatom = new Atom();
	vagatom->setElementSymbol(a.element.name());
	vagatom->setAtomName(a.name);
	vagatom->setResidueId(ai.resid);
	vagatom->setChain(ai.chain);
	vagatom->setHetatm(ai.isHetatm);
	vagatom->setCode(ai.res);
	glm::vec3 pos = glm::vec3(a.pos.x, a.pos.y, a.pos.z);
	vagatom->setInitialPosition(pos, a.b_iso);
	
	*_macroAtoms += vagatom;
}

std::set<char> get_alt_confs(std::vector<gemmi::Atom> &atoms)
{
	std::set<char> chs;

	for (gemmi::Atom &atom : atoms)
	{
		if (atom.altloc != '\0')
		{
			chs.insert(atom.altloc);
		}
	}
	
	return chs;
}

void PdbFile::processResidue(gemmi::Residue &r, AtomInfo &ai)
{
	ai.res = r.name;
	ai.resid = r.seqid.num.str() + r.seqid.icode;
	ai.seqstr = r.seqid.str();
	ai.isHetatm = (r.het_flag == 'H');
	
	std::set<char> confs = get_alt_confs(r.atoms);
	
	char chosen = '\0';
	if (confs.size())
	{
		chosen = *confs.begin();
	}
	
	for (size_t i = 0; i < r.atoms.size(); i++)
	{
		gemmi::Atom &a = r.atoms[i];
		processAtom(a, ai, chosen);
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

	gemmi::Structure st;
	try
	{
		st = gemmi::read_structure_file(path);
	}
	catch (const std::runtime_error &err)
	{
		std::cout << "Reading " << path << " failed: " << std::endl;
		std::cout << err.what() << std::endl;
		return;
	}

	setup_entities(st);
	
	for (size_t i = 0; i < st.entities.size(); i++)
	{
		std::vector<std::string> seq = st.entities[i].full_sequence;

//		std::cout << st.entities[i].name << " = ";
//		std::cout << gemmi::one_letter_code(seq) << std::endl;
	}

	for (size_t i = 0; i < st.models.size(); i++)
	{
		processModel(st.models[i]);
	}
	
	std::cout << "This file " << _filename << " has " << atomCount() 
	<< " atoms.\n";
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
	
	if (_knot != KnotNone && _macroAtoms->size() > 0)
	{
		getAllGeometry();
	}
	
	if (_knot != KnotNone)
	{
		AtomGroup *ptr = (_macroAtoms->size() == 0 ? _compAtoms : _macroAtoms);
		Knotter knotter(ptr, _table);
		
		if (_knot == KnotAngles)
		{
			knotter.setDoTorsions(false);
		}
		else if (_knot == KnotLengths)
		{
			knotter.setDoTorsions(false);
			knotter.setDoAngles(false);
		}

		knotter.knot();
	}
}

void PdbFile::write(std::string filename)
{
	std::string path = toFilename(_filename);
	gemmi::Structure st;
	try
	{
		st = gemmi::read_structure_file(path);
	}
	catch (const std::runtime_error &err)
	{
		std::cout << "Reading " << path << " failed: " << std::endl;
		std::cout << err.what() << std::endl;
		return;
	}

	for (gemmi::Model &m : st.models)
	{
		for (gemmi::Chain &c : m.chains)
		for (gemmi::Residue &r : c.residues)
		{
			ResidueId id(r.seqid.num.str() + r.seqid.icode);
			for (gemmi::Atom &a : r.atoms)
			{
				Atom *atom = _macroAtoms->atomByIdName(id, a.name);
				if (atom != nullptr)
				{
					glm::vec3 pos = atom->derivedPosition();
					a.pos.x = pos.x;
					a.pos.y = pos.y;
					a.pos.z = pos.z;
				}
			}
		}
	}
	
	std::ofstream file;
	file.open(filename);
	
	gemmi::write_pdb(st, file);
	
	file.close();
}

File::Type PdbFile::cursoryLook()
{
	std::string path = toFilename(_filename);
	gemmi::Structure st;
	try
	{
		st = gemmi::read_structure_file(path);
	}
	catch (const std::runtime_error &err)
	{
		std::cout << "Reading " << path << " failed: " << std::endl;
		std::cout << err.what() << std::endl;
		return Nothing;
	}
	Type hasAtoms = (st.models.size() > 0 ? MacroAtoms : Nothing);
	
	std::string spgname = st.spacegroup_hm;
	Type hasSymmetry = (spgname.length() ? Symmetry : Nothing);
	
	Type hasCell = (st.cell.a > 0 ? UnitCell : Nothing);

	File::Type type = File::Type(hasAtoms | hasSymmetry | hasCell);
	
	return type;
}

