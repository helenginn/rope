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

#ifndef __vagabond__AbstractForce__
#define __vagabond__AbstractForce__

#include <vagabond/utils/glm_import.h>
#include <iostream>

typedef std::function<void(int, float)> InsertIntoRow;

class AbstractForce
{
public:
	enum Status
	{
		StatusKnown,
		StatusKnownDirOnly,
		StatusUnknown,
		StatusIgnored,
		StatusCalculated,
		StatusCalculatedDirOnly,
	};

	enum Reason
	{
		ReasonBondLength,
		ReasonBondAngle,
		ReasonBondTorsion,
		ReasonVdwContact,
		ReasonElectrostaticContact,
		ReasonReactionForce,
		ReasonUnknown,
	};

	AbstractForce(Status status, Reason reason)
	{
		_status = status;
		_reason = reason;
	}
	
	glm::vec3 get_vector()
	{
		if (!_getUnit || !_getMag)
		{
			return glm::vec3{};
		}

		glm::vec3 vec = _getUnit() * _getMag();
		return vec;
	}
	
	glm::vec3 unit_vec()
	{
		return _getUnit();
	}

	void setUnitGetter(const std::function<glm::vec3()> &func)
	{
		_getUnit = func;
	}

	void setMagGetter(const std::function<float()> &func)
	{
		_getMag = func;
	}
	
	float magnitude()
	{
		return _getMag();
	}
	
	void setStatus(Status status)
	{
		_status = status;
	}
	
	const Status &status() const
	{
		return _status;
	}
	
	const Reason &reason() const
	{
		return _reason;
	}
	
	void setReport(const std::function<std::string()> &report)
	{
		_report = report;
	}
	
	std::string report()
	{
		return _report ? _report() : "(empty report)";
	}
	
	friend std::ostream &operator<<(std::ostream &ss, const AbstractForce *f);
private:
	Status _status{};
	Reason _reason{};

	std::function<glm::vec3()> _getUnit;
	std::function<float()> _getMag;
	std::function<std::string()> _report;
};

inline std::ostream &operator<<(std::ostream &ss, const AbstractForce *f)
{
	ss << "Force (";
	switch (f->status())
	{
		case AbstractForce::StatusKnown:
		ss << "Known";
		break;
		case AbstractForce::StatusUnknown:
		ss << "Unknown";
		break;
		case AbstractForce::StatusKnownDirOnly:
		ss << "Direction known";
		break;
		case AbstractForce::StatusCalculated:
		ss << "Calculated";
		break;
		case AbstractForce::StatusCalculatedDirOnly:
		ss << "Calculated magnitude";
		break;
		case AbstractForce::StatusIgnored:
		ss << "Ignored";
		break;
	};
	
	ss << ", ";

	switch (f->reason())
	{
		case AbstractForce::ReasonBondLength:
		ss << "bond length";
		break;
		case AbstractForce::ReasonBondAngle:
		ss << "bond angle";
		break;
		case AbstractForce::ReasonBondTorsion:
		ss << "bond torsion";
		break;
		case AbstractForce::ReasonVdwContact:
		ss << "VdW Contact";
		break;
		case AbstractForce::ReasonElectrostaticContact:
		ss << "electrostatic Contact";
		break;
		case AbstractForce::ReasonReactionForce:
		ss << "reaction force";
		break;
		case AbstractForce::ReasonUnknown:
		ss << "unknown";
		break;
	};

	ss << ")";

	return ss;
}

typedef std::pair<AbstractForce *, int> ForceCoordinate;

#endif
