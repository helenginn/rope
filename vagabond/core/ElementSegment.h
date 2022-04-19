// vagabond
// Copyright (C) 2019 Helen Ginn
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

#ifndef __vagabond__ElementSegment__
#define __vagabond__ElementSegment__

#include "FFTCubicGrid.h"

class AtomGroup;
struct Job;

struct VoxelElement
{
	fftwf_complex value;
	float scatter = 0;
};

class ElementSegment : public FFTCubicGrid<VoxelElement>
{
public:
	ElementSegment();
	
	/** set the periodic table element symbol
	 * @param element upper case element symbol e.g. CA for calcium. */
	void setElement(std::string element);
	
	const std::string &elementSymbol() const
	{
		return _elementSymbol;
	}
	
	void setJob(Job *job)
	{
		_job = job;
	}
	
	Job *const job() const
	{
		return _job;
	}

	virtual void populatePlan(FFT<VoxelElement>::PlanDims &dims);
	void addDensity(glm::vec3 real, float density);
	float density(int i, int j);
	void printMap();
	
	void calculateMap();

	virtual float sum();
	virtual void multiply(float scale);

	static void findDimensions(int &nx, int &ny, int &nz, glm::vec3 min,
	                           glm::vec3 max, float cubeDim);
protected:

private:
	Job *_job;
	std::string _elementSymbol;

};

#endif
