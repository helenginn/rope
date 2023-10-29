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

#ifndef __vagabond__Correlator__
#define __vagabond__Correlator__

#include <vagabond/utils/glm_import.h>
#include <vector>
#include <fftw3.h>

template <class T>
class OriginGrid;

class AtomSegment;
class AtomMap;

class Correlator
{
public:
	Correlator(OriginGrid<fftwf_complex> *data, const AtomMap *templateMap);

	void prepareList();
	double correlation(AtomMap *seg);
private:
	OriginGrid<fftwf_complex> *_density = nullptr;
	
	struct Comparison
	{
		glm::vec3 template_real_voxel;
		float comparison_value;
	};

	std::vector<Comparison> _comparisons;
	const AtomMap *_template = nullptr;
};

#endif
