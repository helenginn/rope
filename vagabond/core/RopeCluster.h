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

#ifndef __vagabond__RopeCluster__
#define __vagabond__RopeCluster__

#include <sstream>
#include "vagabond/c4x/ClusterSVD.h"
#include "vagabond/core/MetadataGroup.h"
#include "vagabond/core/PositionalGroup.h"

class RopeCluster : public ViewableCluster
{
public:
	virtual std::string csvFirstLine() = 0;
	virtual std::string csvLine(int i, int j) = 0;
	virtual std::string value_desc(int i, int j) const = 0;

	virtual void changeLastAxis(int axis) = 0;
	virtual void calculateInverse() = 0;
	virtual int version() = 0;
private:

};

class PositionalCluster : public ClusterSVD<PositionalGroup>, public RopeCluster
{
public:
	PositionalCluster(PositionalGroup &group) : ClusterSVD<PositionalGroup>(group)
	{};
	
	virtual std::string csvFirstLine()
	{
		return "atom_id,position_change(xyz),magnitude";
	}
	
	virtual std::string csvLine(int i, int j)
	{
		std::vector<Posular> poz = rawVector(i);
		return (dataGroup()->headers()[j].desc() + ", " 
		        + poz[j].str() + ", "
		        + std::to_string((float)poz[j]));
	}

	virtual int version()
	{
		return ClusterSVD<PositionalGroup>::version();
	}
	
	virtual void cluster()
	{
		ClusterSVD<PositionalGroup>::cluster();
	}

	virtual void calculateInverse()
	{
		ClusterSVD<PositionalGroup>::calculateInverse();
	}

	virtual size_t displayableDimensions()
	{
		return ClusterSVD<PositionalGroup>::displayableDimensions();
	}
	
	virtual size_t rows() const
	{
		return ClusterSVD<PositionalGroup>::rows();
	}

	virtual size_t columns() const
	{
		return ClusterSVD<PositionalGroup>::columns();
	}

	virtual const size_t pointCount() const
	{
		return ClusterSVD<PositionalGroup>::pointCount();
	}

	virtual std::string value_desc(int i, int j) const
	{
		std::ostringstream ss;
		ss << this->value(i, j);
		return ss.str();
	}

	virtual const size_t &axis(int i) const
	{
		return ClusterSVD<PositionalGroup>::axis(i);
	}

	virtual float weight(int i) const
	{
		return ClusterSVD<PositionalGroup>::weight(i);
	}

	virtual void reweight(glm::vec3 &point) const
	{
		ClusterSVD<PositionalGroup>::reweight(point);
	}

	virtual glm::vec3 pointForDisplay(int i) const
	{
		return ClusterSVD<PositionalGroup>::pointForDisplay(i);
	}

	virtual glm::vec3 pointForDisplay(std::vector<float> &mapped) const
	{
		return ClusterSVD<PositionalGroup>::pointForDisplay(mapped);
	}

	virtual glm::vec3 point(int i) const
	{
		return ClusterSVD<PositionalGroup>::point(i);
	}

	virtual glm::vec3 point(std::vector<float> &mapped) const
	{
		return ClusterSVD<PositionalGroup>::point(mapped);
	}

	virtual void changeLastAxis(int axis)
	{
		return ClusterSVD<PositionalGroup>::changeLastAxis(axis);
	}
	
	virtual bool givesTorsionAngles()
	{
		return false;
	}

	virtual bool canMapVectors()
	{
		return true;
	}

	virtual ObjectGroup *objectGroup()
	{
		return ClusterSVD<PositionalGroup>::objectGroup();
	}
	
	virtual void chooseBestAxes(std::vector<float> &vals)
	{
		return ClusterSVD<PositionalGroup>::chooseBestAxes(vals);
	}

};

class TorsionCluster : public ClusterSVD<MetadataGroup>, public RopeCluster
{
public:
	TorsionCluster(MetadataGroup &group) : ClusterSVD<MetadataGroup>(group)
	{};
	
	virtual std::string csvFirstLine()
	{
		return "torsion_id,torsion(degrees)";
	}
	
	virtual std::string csvLine(int i, int j)
	{
		std::vector<Angular> poz = rawVector(i);
		return (dataGroup()->headers()[j].desc() + ", " 
		        + std::to_string(poz[j]));
	}
	
	virtual void cluster()
	{
		ClusterSVD<MetadataGroup>::cluster();
	}

	virtual int version()
	{
		return ClusterSVD<MetadataGroup>::version();
	}
	
	virtual void setSubtractAverage(bool subtract)
	{
		ClusterSVD<MetadataGroup>::objectGroup()->setSubtractAverage(subtract);
	}

	virtual void calculateInverse()
	{
		ClusterSVD<MetadataGroup>::calculateInverse();
	}

	virtual size_t displayableDimensions()
	{
		return ClusterSVD<MetadataGroup>::displayableDimensions();
	}
	
	virtual size_t rows() const
	{
		return ClusterSVD<MetadataGroup>::rows();
	}

	virtual size_t columns() const
	{
		return ClusterSVD<MetadataGroup>::columns();
	}

	virtual const size_t pointCount() const
	{
		return ClusterSVD<MetadataGroup>::pointCount();
	}

	virtual std::string value_desc(int i, int j) const
	{
		std::ostringstream ss;
		ss << this->value(i, j);
		return ss.str();
	}

	virtual const size_t &axis(int i) const
	{
		return ClusterSVD<MetadataGroup>::axis(i);
	}

	virtual float weight(int i) const
	{
		return ClusterSVD<MetadataGroup>::weight(i);
	}

	virtual void reweight(glm::vec3 &point) const
	{
		ClusterSVD<MetadataGroup>::reweight(point);
	}

	virtual glm::vec3 pointForDisplay(int i) const
	{
		return ClusterSVD<MetadataGroup>::pointForDisplay(i);
	}

	virtual glm::vec3 pointForDisplay(std::vector<float> &mapped) const
	{
		return ClusterSVD<MetadataGroup>::pointForDisplay(mapped);
	}

	virtual glm::vec3 point(int i) const
	{
		return ClusterSVD<MetadataGroup>::point(i);
	}

	virtual glm::vec3 point(std::vector<float> &mapped) const
	{
		return ClusterSVD<MetadataGroup>::point(mapped);
	}

	virtual void changeLastAxis(int axis)
	{
		return ClusterSVD<MetadataGroup>::changeLastAxis(axis);
	}
	
	virtual bool givesTorsionAngles()
	{
		return true;
	}

	virtual bool canMapVectors()
	{
		return true;
	}

	virtual ObjectGroup *objectGroup()
	{
		return ClusterSVD<MetadataGroup>::objectGroup();
	}
	
	virtual void chooseBestAxes(std::vector<float> &vals)
	{
		return ClusterSVD<MetadataGroup>::chooseBestAxes(vals);
	}

};

#endif
