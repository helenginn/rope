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

#ifndef __vagabond__ElementLibrary__
#define __vagabond__ElementLibrary__

#include <map>
#include <string>

class ElementLibrary
{
public:
	ElementLibrary();

	static ElementLibrary &library()
	{
		return _library;
	}
	
	const int &numPoints() const;
	const float *getDSpacing() const;
	
	float valueForResolution(float res, const float *scatter) const;

	const float *getElementFactors(std::string element) const;
private:
	void addElement(std::string element, const float *scatter);

	std::map<std::string, const float *> _elementMap;
	static ElementLibrary _library;

};

#endif
