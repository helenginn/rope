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

#define _USE_MATH_DEFINES
#include <math.h>
#include <gemmi/numb.hpp>
#include "../utils/FileReader.h"
#include "files/CifFile.h"
#include "files/CsvFile.h"
#include "files/MtzFile.h"
#include "files/PdbFile.h"
#include "files/FastaFile.h"
#include "Atom.h"
#include "grids/Diffraction.h"
#include "AtomContent.h"
#include "Metadata.h"
#include "RefList.h"
#include "FileManager.h"

using namespace gemmi::cif;

File::File(std::string filename)
{
	changeFilename(filename);
	_compAtoms->setOwns(true);
	_macroAtoms->setOwns(true);
	_metadata = new Metadata();
}

File::~File()
{
	delete _compAtoms;
	delete _macroAtoms;
	
	if (!_accessedMetadata)
	{
		delete _metadata;
	}
	
	if (!_accessedTable)
	{
		delete _table;
	}
}

std::string File::toFilename(std::string filename)
{
	std::string tmp = filename;
#ifndef __EMSCRIPTEN__
	if (!file_exists(tmp))
	{
		FileManager::correctFilename(filename);
		tmp = filename;
	}
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

AtomContent *File::compAtoms()
{
	return new AtomContent(*_compAtoms);
}

AtomContent *File::atoms()
{
	if (_macroAtoms->size() > 0)
	{
		return new AtomContent(*_macroAtoms);
	}
	else
	{
		return new AtomContent(*_compAtoms);
	}
}

Metadata *File::metadata()
{
	_accessedMetadata = true;
	return new Metadata(*_metadata);
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
	}

	return spg;
}

std::string File::spaceGroupName() const
{
	std::string name = _values.at("_symmetry.space_group_name_H-M");
	cleanup(name);
	return name;
}

RefList *File::reflectionList() const
{
	RefList *list = new RefList(_reflections);
	list->setSpaceGroupName(spaceGroupName());
	
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

bool File::compare_file_ending(const std::string &filename, 
                         const std::string &comp, File::Flavour result)
{
	int l = filename.length();
	if (l < comp.length())
	{
		return None;
	}
	if (filename.substr(l - comp.length(), comp.length()) == comp)
	{
		return result;
	}

	return None;
}

File::Flavour File::flavour(std::string filename)
{
	const std::string json = "json";
	const std::string mtz = "mtz";
	const std::string pdb = "pdb";
	const std::string cif = "cif";
	const std::string csv = "csv";
	const std::string fasta = "fasta";
	to_lower(filename);

	if (compare_file_ending(filename, json, Jsn))
	{
		return Jsn;
	}
	if (compare_file_ending(filename, mtz, Mtz))
	{
		return Mtz;
	}
	if (compare_file_ending(filename, pdb, Pdb))
	{
		return Pdb;
	}
	if (compare_file_ending(filename, cif, Cif))
	{
		return Cif;
	}
	if (compare_file_ending(filename, csv, Csv))
	{
		return Csv;
	}
	if (compare_file_ending(filename, fasta, Fasta))
	{
		return Fasta;
	}

	return None;
}

File *File::openUnknown(std::string filename)
{
	File *f = nullptr;
	
	Flavour flav = flavour(filename);
	if (flav == Mtz)
	{
		f = new MtzFile(filename);
	}
	else if (flav == Pdb)
	{
		f = new PdbFile(filename);
	}
	else if (flav == Cif)
	{
		f = new CifFile(filename);
	}
	else if (flav == Jsn)
	{
		throw std::runtime_error("Don't load json this way");
//		f = new JsonFile(filename);
	}
	else if (flav == Csv)
	{
		f = new CsvFile(filename);
	}
	else if (flav == Fasta)
	{
		f = new FastaFile(filename);
	}
	
	return f;
}

File *File::loadUnknown(std::string filename)
{
	File *f = openUnknown(filename);

	if (f)
	{
		f->parse();
	}
	
	return f;
}

File::Type File::checkUnknownType(std::string filename, Flavour needed)
{
	Flavour flav = flavour(filename);
	
	if (needed != None && needed != flav)
	{
		return Nothing;
	}

	Type type = Nothing;

	if (flav == Mtz)
	{
		MtzFile f = MtzFile(filename);
		type = f.cursoryLook();
	}
	else if (flav == Pdb)
	{
		PdbFile f = PdbFile(filename);
		type = f.cursoryLook();
	}
	else if (flav == Cif)
	{
		CifFile f = CifFile(filename);
		type = f.cursoryLook();
	}
	else if (flav == Jsn)
	{
		return File::Json;
	}
	else if (flav == Csv)
	{
		CsvFile f = CsvFile(filename);
		type = f.cursoryLook();
	}
	else if (flav == Fasta)
	{
		FastaFile f = FastaFile(filename);
		type = f.cursoryLook();
	}
	
	return type;
}

File::Type File::typeUnknown(std::string filename, Flavour needed)
{
	try
	{
		return checkUnknownType(filename, needed);
	}
	catch (...)
	{
		return Nothing;
	}
}
