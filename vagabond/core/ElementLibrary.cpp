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

#include "ElementLibrary.h"
#include "ScatterFactors.h"
#include "../utils/FileReader.h"

ElementLibrary ElementLibrary::_library;

ElementLibrary::ElementLibrary()
{
	addElement("h", ScatterFactors::hScatter);
	addElement("c", ScatterFactors::cScatter);
	addElement("n", ScatterFactors::nScatter);
	addElement("o", ScatterFactors::oScatter);
	addElement("s", ScatterFactors::sScatter);
	addElement("p", ScatterFactors::pScatter);
	addElement("f", ScatterFactors::fScatter);
}

void ElementLibrary::addElement(std::string element, const float *scatter)
{
	to_lower(element);
	_elementMap[element] = scatter;
}

float ElementLibrary::valueForResolution(float res, const float *scatter) const
{
	const float *ds = getDSpacing();

	for (int i = 0; i < numPoints() - 1; i++)
	{
		if (res >= ds[i] && res < ds[i + 1])
		{
			float interval = (res - ds[i]);
			interval /= ds[i + 1] - ds[i];
			float val = scatter[i] + interval * (scatter[i + 1] - scatter[i]);
			return val;
		}
	}

	return 0;
}

const int &ElementLibrary::numPoints() const
{
	return ScatterFactors::numScatter;
}

const float *ElementLibrary::getDSpacing() const
{
	return ScatterFactors::dScatter;
}

const float *ElementLibrary::getElementFactors(std::string element) const
{
	to_lower(element);
	if (_elementMap.count(element) == 0)
	{
		throw std::runtime_error("Element " + element + " not in "
		                         "scattering factor dictionary");
	}

	return _elementMap.at(element);
}
