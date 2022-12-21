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

#ifndef __vagabond__ViewableCluster__
#define __vagabond__ViewableCluster__

#include <vagabond/c4x/Angular.h>

class MetadataGroup;
class ObjectGroup;

class ViewableCluster
{
public:
	ViewableCluster() {};
	virtual ~ViewableCluster() {};

	/** perform appropriate calculations to generate clusters */
	virtual void cluster() = 0;

	/** how many dimensions can be displayed simultaneously in a GUI */
	virtual size_t displayableDimensions() = 0;
	
	virtual size_t rows() const = 0;
	virtual size_t columns() const = 0;
	virtual const size_t pointCount() const = 0;
	virtual glm::vec3 point(int i) const = 0;
	virtual glm::vec3 point(std::vector<float> &mapped) const = 0;
	virtual float weight(int i) const = 0;
	virtual const size_t &axis(int i) const = 0;

	virtual bool givesTorsionAngles() = 0;
	virtual bool canMapVectors() = 0;
	
	virtual ObjectGroup *objectGroup() = 0;
	
	virtual void chooseBestAxes(std::vector<float> &vals) = 0;
private:

};

#endif
