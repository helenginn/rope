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

#include <gemmi/symmetry.hpp>
#include "grids/Diffraction.h"

namespace SymmetryExpansion
{
	static void apply(Diffraction *grid, const gemmi::SpaceGroup *spg, float dMax)
	{
		gemmi::GroupOps grp = spg->operations();
		fftwf_complex *dest = (fftwf_complex *)calloc(grid->nn(),
		                                              sizeof(fftwf_complex));

		if (spg->number == 1)
		{
			return;
		}

		auto accepted_res = [grid, dMax](int i, int j, int k) -> bool
		{
			if (dMax < 0) return true;
			return (grid->resolution(i, j, k) > dMax);
		};

		auto is_sysabs = [grp](int i, int j, int k) -> bool
		{
			int abs = grp.is_systematically_absent({i, j, k});
			return abs;
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
		[grp, phase_for_index, amplitude_for_index, add_real_imag_for_index]
		(int i, int j, int k)
		{
			float amp = amplitude_for_index(i, j, k);
			amp /= grp.sym_ops.size();

			float original_phase = phase_for_index(i, j, k);

			for (int l = 0; l < grp.sym_ops.size(); l++)
			{
				gemmi::Op op = grp.get_op(l);
				gemmi::Op::Miller m = {i, j, k};
				gemmi::Op::Miller n = op.apply_to_hkl(m);
				float ph_shift = rad2deg(op.phase_shift(m));
				float phase = original_phase + ph_shift;

				add_real_imag_for_index(n[0], n[1], n[2], amp, phase);
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

		free(dest);
	}

};

#endif
