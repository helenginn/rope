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

#include <gemmi/numb.hpp>
#include "commit.h"
#include "CifFile.h"
#include "File.h"
#include "Atom.h"
#include "Diffraction.h"
#include "RefList.h"

using namespace gemmi::cif;

File::File(std::string filename)
{
	changeFilename(filename);
}

File::~File()
{
	if (!_accessedCompAtoms)
	{
		delete _compAtoms;
	}

	if (!_accessedMacroAtoms)
	{
		delete _macroAtoms;
	}

	else if (!_accessedTable)
	{
		delete _table;
	}
}

std::string File::toFilename(std::string filename)
{
	std::string tmp = filename;
#ifndef __EMSCRIPTEN__
	tmp = std::string(DATA_DIRECTORY) + "/" + filename;
#endif

	return tmp;
}

void File::changeFilename(std::string filename)
{
	_filename = filename;
}

const size_t File::compAtomCount() const
{
	return _compAtoms->size();
}

const size_t File::atomCount() const
{
	if (_macroAtoms && _macroAtoms->size() > 0)
	{
		return _macroAtoms->size();
	}
	if (_compAtoms && _compAtoms->size() > 0)
	{
		return _compAtoms->size();
	}
	
	return 0;
}

AtomGroup *File::atoms()
{
	if (_macroAtoms && _macroAtoms->size() > 0)
	{
		_accessedMacroAtoms = true;
		return _macroAtoms;
	}
	else
	{
		_accessedCompAtoms = true;
		return _compAtoms;
	}
}

bool File::hasUnitCell() const
{
	bool found = true;
	found &= _values.count("_cell.length_a");
	found &= _values.count("_cell.length_b");
	found &= _values.count("_cell.length_c");
	found &= _values.count("_cell.angle_alpha");
	found &= _values.count("_cell.angle_beta");
	found &= _values.count("_cell.angle_gamma");
	
	return found;
}

std::array<double, 6> File::unitCell() const
{
	if (!hasUnitCell())
	{
		throw std::runtime_error("Requested non-existent unit cell");
	}

	double a = as_number(_values.at("_cell.length_a"));
	double b = as_number(_values.at("_cell.length_b"));
	double c = as_number(_values.at("_cell.length_c"));
	double alpha = as_number(_values.at("_cell.angle_alpha"));
	double beta = as_number(_values.at("_cell.angle_beta"));
	double gamma = as_number(_values.at("_cell.angle_gamma"));
	
	std::array<double, 6> cell = {a, b, c, alpha, beta, gamma};
	
	return cell;
}

void cleanup(std::string &str)
{
	for (size_t i = 0; i < str.length(); i++)
	{
		if ((str[i] == '\'' || str[i] == '"'))
	    {
		     str.erase(str.begin() + i);
		     i--;
	    }
    }
}

int File::spaceGroupNum() const
{
	int spg = -1;
	if (_values.count("_symmetry.Int_Tables_number") > 0)
	{
		spg = as_number(_values.at("_symmetry.Int_Tables_number"));
	}
	else if (_values.count("_symmetry.space_group_name_H-M") > 0)
	{
		std::string str = _values.at("_symmetry.space_group_name_H-M");
		cleanup(str);
		CCP4SPG *group = ccp4spg_load_by_ccp4_spgname(str.c_str());
		spg = group->spg_ccp4_num;
		ccp4spg_free(&group);
	}

	return spg;
}

RefList *File::reflectionList() const
{
	RefList *list = new RefList(_reflections);
	list->setSpaceGroup(spaceGroupNum());
	
	if (hasUnitCell())
	{
		std::array<double, 6> cell = unitCell();
		list->setUnitCell(cell);
	}
	
	return list;
}

Diffraction *File::diffractionData() const
{
	RefList *list = reflectionList();
	Diffraction *diffraction = new Diffraction(*list);
	return diffraction;
}

File *File::loadUnknown(std::string filename)
{
	File *f = nullptr;
	
	const std::string mtz = "mtz";
	const std::string pdb = "pdb";
	const std::string cif = "cif";
	int l = filename.length();
	if (filename.substr(l - mtz.length(), mtz.length()) == mtz)
	{

	}
	else if (filename.substr(l - pdb.length(), pdb.length()) == pdb)
	{

	}
	else if (filename.substr(l - cif.length(), cif.length()) == cif)
	{
		f = new CifFile(filename);
	}
	
	if (f)
	{
		f->parse();
	}
	
	return f;
}
