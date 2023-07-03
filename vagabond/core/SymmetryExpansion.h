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

#ifndef __vagabond__SymmetryExpansion__
#define __vagabond__SymmetryExpansion__

#include <ccp4/csymlib.h>
#include "Diffraction.h"

namespace SymmetryExpansion
{
	static void apply(Diffraction *grid, CCP4SPG *spg, float dMax)
	{
		fftwf_complex *dest = (fftwf_complex *)calloc(grid->nn(),
		                                              sizeof(fftwf_complex));

		if (spg->spg_num == 1)
		{
			return;
		}

		auto accepted_res = [grid, dMax](int i, int j, int k) -> bool
		{
			if (dMax < 0) return true;
			return (grid->resolution(i, j, k) > dMax);
		};

		auto is_sysabs = [spg](int i, int j, int k) -> bool
		{
			int abs = ccp4spg_is_sysabs(spg, i, j, k);
			return abs;
		};
		
		auto rotate_by_symop_rot = [](int &i, int &j, int &k, float *rot)
		{
			int _i = (int) rint(i*rot[0] + j*rot[3] + k*rot[6]);
			int _j = (int) rint(i*rot[1] + j*rot[4] + k*rot[7]);
			int _k = (int) rint(i*rot[2] + j*rot[5] + k*rot[8]);

			i = _i; j = _j; k = _k;
		};

		auto added_phase = [](int i, int j, int k, float *trn)
		-> float
		{
			float shift = (float)i * trn[0];
			shift += (float)j * trn[1];
			shift += (float)k * trn[2];
			shift = shift - floor(shift);

			float add = shift * 360.f;
			return add;
		};
		
		auto phase_for_index = [grid](int i, int j, int k) -> float
		{
			return grid->element(i, j, k).phase();
		};
		
		auto amplitude_for_index = [grid](int i, int j, int k) -> float
		{
			return grid->element(i, j, k).amplitude();
		};
		
		auto add_real_imag_for_index = [grid, dest](int i, int j, int k,
		                                            float amp, float phase)
		{
			long ele = grid->index(i, j, k);
			dest[ele][0] += amp * cos(deg2rad(phase));
			dest[ele][1] += amp * sin(deg2rad(phase));
		};
		
		auto add_amp_phase_to_mates =
		[spg, rotate_by_symop_rot, added_phase, phase_for_index,
		amplitude_for_index, add_real_imag_for_index]
		(int i, int j, int k)
		{
			float amp = amplitude_for_index(i, j, k);
			amp /= spg->nsymop;

			float original_phase = phase_for_index(i, j, k);

			for (int l = 0; l < spg->nsymop; l++)
			{
				float *rot = &spg->invsymop[l].rot[0][0];
				float *trn = spg->symop[l].trn;

				int _i = i; int _j = j; int _k = k;
				rotate_by_symop_rot(_i, _j, _k, rot);
				float phase = original_phase + added_phase(_i, _j, _k, trn);

				add_real_imag_for_index(_i, _j, _k, amp, phase);
			}
		};
		
		auto filtered_averaging = 
		[add_amp_phase_to_mates, is_sysabs, accepted_res](int i, int j, int k)
		{
			if (!is_sysabs(i, j, k) && accepted_res(i, j, k))
			{
				add_amp_phase_to_mates(i, j, k);
			}
		};

		grid->do_op_on_centred_index(filtered_averaging);
		grid->do_op_on_each_1d_index([dest, grid](int i)
		{
			grid->element(i).value[0] = dest[i][0];
			grid->element(i).value[1] = dest[i][1];
		});
	}

};

#endif
