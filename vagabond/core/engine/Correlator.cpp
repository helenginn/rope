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

#include "engine/Correlator.h"
#include "AtomSegment.h"
#include "engine/MapSumHandler.h"
#include <vagabond/utils/maths.h>

Correlator::Correlator(OriginGrid<fftwf_complex> *data,
                       MapSumHandler *sumHandler)
{
	_density = data;
	_sumHandler = sumHandler;
}

void Correlator::prepareList()
{
	_template = _sumHandler->templateMap();
	
	/* find minimum and maximum real-space limits of the template */
	glm::vec3 min = _template->minBound();
	glm::vec3 max = _template->maxBound();
	int all = 0;
	int rejected = 0;

	/* loop through each point in the reference map */
	for (size_t k = 0; k < _density->nz(); k++)
	{
		for (size_t j = 0; j < _density->ny(); j++)
		{
			for (size_t i = 0; i < _density->nx(); i++)
			{
				glm::vec3 real_pos = _density->real(i, j, k);

				/* find point relative to the minimum of template */
				glm::vec3 relative_pos = real_pos;
				relative_pos -= min;
				
//				std::cout << "start: " << relative_pos << std::endl;
				/* convert to fractional in the space of the big map */
				_density->real2Voxel(relative_pos);
				_density->index_to_fractional(relative_pos);
				
				Grid<fftwf_complex>::collapseFrac(relative_pos);

				/* new position in the space closest to the template box */
				_density->fractional_to_index(relative_pos);
				_density->voxel2Real(relative_pos);

				relative_pos += min;
				all++;

				/* * store the reference density value of the voxel */
				float value = _density->realValue(real_pos);
				float check = _density->realValue(relative_pos);

				/* establish if each point falls within the box 
				 * and throw out if not */
				if (relative_pos.x > max.x || relative_pos.y > max.y
				    || relative_pos.z > max.z)
				{
					rejected++;
					continue;
				}
				
				Comparison comp{relative_pos, value};
				_comparisons.push_back(comp);
			}
		}
	}
}

double Correlator::correlation(AtomSegment *seg)
{
	CorrelData cd = empty_CD();
	int count = 0;

	for (Comparison &comp : _comparisons)
	{
		glm::vec3 &real = comp.template_real_voxel;
		float test = seg->interpolate(real);
		float &val = comp.comparison_value;
		count++;

		add_to_CD(&cd, val, test, test);
	}

	return evaluate_CD(cd);
}
