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

#include <gemmi/polyheur.hpp>
#include <gemmi/mmread.hpp>
#include <gemmi/to_pdb.hpp>
#include <fstream>
#include "files/PdbFile.h"
#include "files/CifFile.h"
#include "AtomGroup.h"
#include "Atom.h"
#include "Knotter.h"

PdbFile::PdbFile(std::string filename) : File(filename)
{

}

void PdbFile::getAllGeometry()
{
	_table = new GeometryTable(GeometryTable::getAllGeometry());
}

void PdbFile::processAtomSet(std::vector<gemmi::Atom *> &atoms, AtomInfo &ai)
{
	Atom *vagatom = new Atom();
	vagatom->setAtomNum(_num);
	vagatom->setResidueId(ai.resid);
	vagatom->setChain(ai.chain);
	vagatom->setHetatm(ai.isHetatm);
	vagatom->setCode(ai.res);

	bool first = true;
	
	for (gemmi::Atom *const &a : atoms)
	{
		vagatom->setAtomName(a->name);
		vagatom->setElementSymbol(a->element.name());

		if (first)
		{
			first = false;
		}

		std::string str;
		str += a->altloc;
		if (str[0] == 0)
		{
			str = "";
		}

		glm::vec3 pos = glm::vec3(a->pos.x, a->pos.y, a->pos.z);
		vagatom->conformerPositions()[str].pos.ave = pos;
		vagatom->conformerPositions()[str].b = a->b_iso;
		vagatom->conformerPositions()[str].occ = a->occ;
		
		glm::mat3x3 tensor = {};
		tensor[0][0] = a->aniso.u11;
		tensor[0][1] = a->aniso.u12;
		tensor[0][2] = a->aniso.u13;
		tensor[1][0] = a->aniso.u12;
		tensor[1][1] = a->aniso.u22;
		tensor[1][2] = a->aniso.u23;
		tensor[2][0] = a->aniso.u13;
		tensor[2][1] = a->aniso.u23;
		tensor[2][2] = a->aniso.u33;

		vagatom->conformerPositions()[str].tensor = tensor;
	}
	
	std::string chosen = vagatom->conformerPositions().begin()->first;
	
	auto better_than_chosen = [&chosen](const std::string &alternative)
	{
		if (alternative.length() < chosen.length())
		{
			return true;
		}
		
		return (alternative < chosen);
	};
	
	for (auto it = vagatom->conformerPositions().begin();
	     it != vagatom->conformerPositions().end(); it++)
	{
		if (better_than_chosen(it->first))
		{
			chosen = it->first;
		}
	}
	
	Atom::AtomPlacement &pl = vagatom->conformerPositions()[chosen];
	vagatom->setInitialPosition(pl.pos.ave, pl.b, pl.tensor, pl.occ);
	
	_num++;
	*_macroAtoms += vagatom;
}

std::set<char> get_alt_confs(const std::vector<gemmi::Atom> &atoms)
{
	std::set<char> chs;

	for (const gemmi::Atom &atom : atoms)
	{
		if (atom.altloc != '\0')
		{
			chs.insert(atom.altloc);
		}
	}
	
	return chs;
}

std::vector<std::string> get_atom_names(const std::vector<gemmi::Atom> &atoms)
{
	std::vector<std::string> names;

	for (const gemmi::Atom &atom : atoms)
	{
		if (std::find(names.begin(), names.end(), atom.name) == names.end())
		{
			names.push_back(atom.name);
		}
	}
	
	return names;
}

std::map<std::string, std::vector<gemmi::Atom *>> 
group_alt_confs(std::vector<gemmi::Atom> &atoms)
{
	std::map<std::string, std::vector<gemmi::Atom *>> map;

	for (gemmi::Atom &atom : atoms)
	{
		std::string name = atom.name;
		map[name].push_back(&atom);
	}
	
	return map;
}

void PdbFile::processResidue(gemmi::Residue &r, AtomInfo &ai)
{
	ai.res = r.name;
	ai.resid = r.seqid.num.str() + r.seqid.icode;
	ai.seqstr = r.seqid.str();
	ai.isHetatm = (r.het_flag == 'H');
	
	typedef std::map<std::string, std::vector<gemmi::Atom *>> GroupMap;
	GroupMap grps = group_alt_confs(r.atoms);
	
	std::vector<std::string> names = get_atom_names(r.atoms);
	for (const std::string &name : names)
	{
		processAtomSet(grps[name], ai);
	}
}

void PdbFile::processModel(gemmi::Model &m, int idx)
{
	for (size_t i = 0; i < m.chains.size(); i++)
	{
		gemmi::Chain &c = m.chains[i];
		std::string ch_id = c.name;
		if (idx >= 0)
		{
			ch_id += std::to_string(idx);
		}

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

	if (!file_exists(path.c_str()))
	{
		throw std::runtime_error("Cannot find file: " + path);
	}

	gemmi::Structure st;
	try
	{
		st = gemmi::read_structure_file(path);
	}
	catch (const std::runtime_error &err)
	{
		std::cout << "Reading original filename: " << _filename << " failed: " << std::endl;
		std::cout << err.what() << std::endl;
		return;
	}

	const gemmi::SpaceGroup *spg = st.find_spacegroup();
	
	if (spg)
	{
		_values["_symmetry.space_group_name_H-M"] = spg->xhm();
	}
	
	_values["_cell.length_a"] = std::to_string(st.cell.a);
	_values["_cell.length_b"] = std::to_string(st.cell.b);
	_values["_cell.length_c"] = std::to_string(st.cell.c);
	_values["_cell.angle_alpha"] = std::to_string(st.cell.alpha);
	_values["_cell.angle_beta"] = std::to_string(st.cell.beta);
	_values["_cell.angle_gamma"] = std::to_string(st.cell.gamma);

	gemmi::setup_entities(st);
	
	/*
	for (size_t i = 0; i < st.entities.size(); i++)
	{
		std::vector<std::string> seq = st.entities[i].full_sequence;

//		std::cout << st.entities[i].name << " = ";
//		std::cout << gemmi::one_letter_code(seq) << std::endl;
	}
	*/

	for (size_t i = 0; i < st.models.size(); i++)
	{
		int idx = st.models.size() == 1 ? -1 : i;
		processModel(st.models[i], idx);
	}
	
//	std::cout << "This file " << _filename << " has " << atomCount() 
//	<< " atoms.\n";
	std::string spgname = st.spacegroup_hm;
	_values["_symmetry.space_group_name_H-M"] = spgname;

	_values["cell.length_a"] = st.cell.a;
	_values["cell.length_b"] = st.cell.b;
	_values["cell.length_c"] = st.cell.c;
	_values["cell.angle_alpha"] = st.cell.alpha;
	_values["cell.angle_beta"] = st.cell.beta;
	_values["cell.angle_gamma"] = st.cell.gamma;
}

void PdbFile::writeAtomsToStructure(AtomGroup *grp, gemmi::Structure &st,
                                    const std::string &model_name, bool altConfs)
{
	st.models.push_back(gemmi::Model());
	
	gemmi::Model &m = st.models.back();
	gemmi::Chain *c = nullptr;
	
	ResidueId prev("");
	gemmi::Residue *last = nullptr;
	gemmi::Residue *first = nullptr;

	for (size_t i = 0; i < grp->size(); i++)
	{
		Atom *atom = (*grp)[i];

		if (!c || atom->chain() != c->name)
		{
			m.chains.push_back(gemmi::Chain(atom->chain()));
			c = &m.chains.back();
		}

		const ResidueId &id = atom->residueId();
		if (id != prev)
		{
			char insert = (id.insert.length() == 0 ? ' ' : id.insert[0]);
			gemmi::ResidueId gemmi_id;
			gemmi_id.seqid = gemmi::SeqId(id.num, insert);
			gemmi_id.name = atom->code();
			c->residues.push_back(gemmi::Residue(gemmi_id));
			last = &(c->residues.back());
			prev = id;
		}
		
		if (last != nullptr && !altConfs)
		{
			last->atoms.push_back(gemmi::Atom());
			gemmi::Atom &a = last->atoms.back();
			glm::vec3 pos = atom->derivedPosition();
			a.name = atom->atomName();
			a.element = gemmi::Element(atom->elementSymbol());
			a.pos.x = pos.x;
			a.pos.y = pos.y;
			a.pos.z = pos.z;
			first = nullptr;
		}
		else if (last != nullptr && altConfs)
		{
			Atom::ConformerInfo &info = atom->conformerPositions();
			std::map<std::string, std::vector<gemmi::Atom>> map;
			
			for (auto it = info.begin(); it != info.end(); it++)
			{
				map[it->first].push_back(gemmi::Atom());
				gemmi::Atom &a = map[it->first].back();
				glm::vec3 pos = it->second.pos.ave;
				a.name = atom->atomName();
				a.b_iso = it->second.b;
				a.occ = it->second.occ;
				a.altloc = it->first[0];
				a.element = gemmi::Element(atom->elementSymbol());
				a.pos.x = pos.x;
				a.pos.y = pos.y;
				a.pos.z = pos.z;
			}
			
			for (auto it = map.begin(); it != map.end(); it++)
			{
				for (gemmi::Atom &atom : it->second)
				{
					last->atoms.push_back(atom);
				}
			}
		}
	}
}

void PdbFile::writeStructure(gemmi::Structure &st, std::string name)
{
	std::ofstream file;
	file.open(name);
	
	gemmi::write_pdb(st, file);
	
	file.close();
}

void PdbFile::writeAtoms(AtomGroup *grp, std::string name, bool altConfs)
{
	gemmi::Structure st;
	writeAtomsToStructure(grp, st, name, altConfs);
	writeStructure(st, name);
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
		std::cout << "Reading original file " << _filename << " failed: " << std::endl;
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
				Atom *atom = _macroAtoms->atomByIdName(id, a.name, c.name);
				if (atom != nullptr)
				{
					glm::vec3 pos = atom->derivedPosition();
					a.pos.x = pos.x;
					a.pos.y = pos.y;
					a.pos.z = pos.z;
					double b = atom->derivedBFactor();
//					a.b_iso = b;
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
		std::cout << "Reading original file " << _filename << " failed: " << std::endl;
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

