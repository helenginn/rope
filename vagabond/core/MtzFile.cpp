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
#define GEMMI_WRITE_IMPLEMENTATION
#include <gemmi/mtz.hpp>
#include <vagabond/core/matrix_functions.h>
#include <vagabond/core/Diffraction.h>

//using namespace gemmi::Mtz;

MtzFile::MtzFile(std::string filename) : File(filename)
{

}

File::Type MtzFile::cursoryLook()
{
	std::string tmp = toFilename(_filename);
	Type type = Nothing;
	gemmi::Mtz mtz;

	try
	{
		mtz = gemmi::read_mtz_file(tmp);
	}
	catch (std::system_error &err)
	{
		return type;
	}
	
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

gemmi::Mtz::Column *find_column(gemmi::Mtz &mtz, 
                                const std::vector<std::string> &list)
{
	gemmi::Mtz::Column *ret = nullptr;
	for (const std::string &trial : list)
	{
		ret = mtz.column_with_label(trial);
		if (ret)
		{
			return ret;
		}
	}
	
	return nullptr;
}

gemmi::Mtz::Column *find_phi_column(gemmi::Mtz &mtz)
{
	const std::vector<std::string> list = {"PHWT", "PHIC", "PHIF", "P"};
	return find_column(mtz, list);
}

gemmi::Mtz::Column *find_f_column(gemmi::Mtz &mtz)
{
	const std::vector<std::string> list = {"FWT", "F", "FP", "Fobs"};
	return find_column(mtz, list);
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
	gemmi::Mtz::Column *cf = find_f_column(mtz);

	gemmi::Mtz::Column *csf = mtz.column_with_label("SIGF");
	if (csf == nullptr)
	{
		csf = mtz.column_with_label("SIGFP");
	}

	gemmi::Mtz::Column *cph = find_phi_column(mtz);
	gemmi::Mtz::Column *cfree = mtz.rfree_column();

	int num = mtz.columns.size();
	
	for (size_t i = 0; i < mtz.data.size(); i += num)
	{
		int h = mtz.data[i + ch->idx];
		int k = mtz.data[i + ck->idx];
		int l = mtz.data[i + cl->idx];
		bool free = cfree ? fabs(mtz.data[i + cfree->idx]) < 1e-6 : 0; 
		
		float f = cf ? mtz.data[i + cf->idx] : 0;
		float sigf = csf ? mtz.data[i + csf->idx] : 0;
		float phi = cph ? mtz.data[i + cph->idx] : 0;

		Reflection refl{};
		refl.hkl.h = h;
		refl.hkl.k = k;
		refl.hkl.l = l;
		refl.free = free;
		refl.f = f;
		refl.sigf = sigf;
		refl.phi = phi;
		
		_reflections.push_back(refl);
	}
}

void MtzFile::setMap(ArbitraryMap *map)
{
	_map = new Diffraction(map);
}

std::string MtzFile::write_to_string(float max_res)
{
	gemmi::Mtz mtz = gemmi::Mtz(true);
	mtz.spacegroup = gemmi::find_spacegroup_by_name("P 1");

	glm::mat3x3 frac2Real = _map->frac2Real();
	std::array<double, 6> uc_dims;
	unit_cell_from_mat3x3(frac2Real, &uc_dims[0]);

	gemmi::UnitCell uc(uc_dims);
	mtz.set_cell_for_all(uc);
	mtz.add_dataset("_filename");

	mtz.add_column("FWT", 'F', 0, 3, false);
	mtz.add_column("PHWT", 'P', 0, 4, false);

	std::vector<float> data;
	std::vector<float> line(5);

	//	CCP4SPG *spg = ccp4spg_load_by_ccp4_num(1);

	for (int k = 0; k < _map->nz() / 2; k++)
	{
		for (int j = -_map->ny() / 2; j < _map->ny() / 2; j++)
		{
			for (int i = -_map->nx() / 2; i < _map->nx() / 2; i++)
			{
				bool f000 = (i == 0 && j == 0 && k == 0);

				if (_map->resolution(i, j, k) < max_res)
				{
					continue;
				}

				float f = _map->element(i, j, k).amplitude();
				float p = _map->element(i, j, k).phase();

				line = {(float)i, (float)j, (float)k, f, p};

				data.reserve(data.size() + line.size());
				data.insert(data.end(), line.begin(), line.end());
			}
		}
	}

	mtz.set_data(&data[0], data.size());
	std::string contents;
	mtz.write_to_string(contents);
	return contents;
}
