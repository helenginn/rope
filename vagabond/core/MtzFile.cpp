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

#include "MtzFile.h"
#include <gemmi/mtz.hpp>

//using namespace gemmi::Mtz;

MtzFile::MtzFile(std::string filename) : File(filename)
{

}

File::Type MtzFile::cursoryLook()
{
	std::string tmp = toFilename(_filename);
	Type type = Nothing;

	gemmi::Mtz mtz = gemmi::read_mtz_file(tmp);
	
	if (mtz.columns.size())
	{
		type = (Type)(type | Reflections);
	}
	
	if (mtz.datasets.size())
	{
		type = (Type)(type | UnitCell);
	}
	
	if (mtz.spacegroup && strlen(mtz.spacegroup->hm))
	{
		type = (Type)(type | Symmetry);
	}

	return type;
}

void MtzFile::parse()
{
	std::string tmp = toFilename(_filename);
	gemmi::Mtz mtz = gemmi::read_mtz_file(tmp);

	if (mtz.datasets.size() == 0)
	{
		return;
	}

	gemmi::UnitCell uc = mtz.datasets[0].cell;

	_values["_cell.length_a"] = f_to_str(uc.a, 4);
	_values["_cell.length_b"] = f_to_str(uc.b, 4);
	_values["_cell.length_c"] = f_to_str(uc.c, 4);
	_values["_cell.angle_alpha"] = f_to_str(uc.alpha, 4);
	_values["_cell.angle_beta"]  = f_to_str(uc.beta, 4);
	_values["_cell.angle_gamma"] = f_to_str(uc.gamma, 4);
	
	_values["_symmetry.space_group_name_H-M"] = mtz.spacegroup->hm;

	gemmi::Mtz::Column *ch = mtz.column_with_label("H");
	gemmi::Mtz::Column *ck = mtz.column_with_label("K");
	gemmi::Mtz::Column *cl = mtz.column_with_label("L");
	gemmi::Mtz::Column *cf = mtz.column_with_label("F");
	if (cf == nullptr)
	{
		std::cout << "checking for FP" << std::endl;
		cf = mtz.column_with_label("FP");
	}

	gemmi::Mtz::Column *csf = mtz.column_with_label("SIGF");
	if (csf == nullptr)
	{
		csf = mtz.column_with_label("SIGFP");
	}

	gemmi::Mtz::Column *cfree = mtz.rfree_column();

	int num = mtz.columns.size();
	
	for (size_t i = 0; i < mtz.data.size(); i += num)
	{
		int h = mtz.data[i + ch->idx];
		int k = mtz.data[i + ck->idx];
		int l = mtz.data[i + cl->idx];
		bool free = cfree ? fabs(mtz.data[i + cfree->idx]) < 1e-6 : 0; 
		
		std::cout << cf << " " << csf << " ";
		
		float f = cf ? mtz.data[i + cf->idx] : 0;
		float sigf = csf ? mtz.data[i + csf->idx] : 0;
		
		std::cout << f << " " << sigf << std::endl;

		Reflection refl{};
		refl.hkl.h = h;
		refl.hkl.k = k;
		refl.hkl.l = l;
		refl.free = free;
		refl.f = f;
		refl.sigf = sigf;
		
		_reflections.push_back(refl);
	}
}
