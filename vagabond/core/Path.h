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

#ifndef __vagabond__Path__
#define __vagabond__Path__

#include "HasMetadata.h"
#include "MetadataGroup.h"
#include "PlausibleRoute.h"
#include "Polymer.h"

class MetadataGroup;
class Trajectory;

class Path : public HasMetadata
{
public:
	Path() {};
	~Path();
	Path(PlausibleRoute *pr);
	void cleanupRoute();

	Instance *startInstance() const
	{
		return _instance;
	}
	
	Instance *endInstance()
	{
		return _end;
	}
	
	bool sameRouteAsPath(Path *other);

	friend void to_json(json &j, const Path &value);
	friend void from_json(const json &j, Path &value);
	
	PlausibleRoute *toRoute();
	void housekeeping();
	
	void setContributeToSVD(bool contrib)
	{
		_contributeSVD = contrib;
	}
	
	size_t motionCount() const
	{
		return _motions.size();
	}
	
	const bool &contributesToSVD() const
	{
		return _contributeSVD;
	}
	
	void addDeviationsToGroup(MetadataGroup &group);
	void addTorsionsToGroup(MetadataGroup &group);
	
	std::string desc() const;

	virtual const std::string id() const
	{
		return desc();
	}

	virtual bool displayable() const /* so these aren't points in the map */
	{
		return false;
	}
	
	const bool &visible() const
	{
		return _visible;
	}
	
	Trajectory *calculateTrajectory(int steps);
	
	void setStepCount(int steps)
	{
		_steps = steps;
	}
	
	bool operator==(const Path &other) const;
	Trajectory *calculateDeviations(int steps);
	
	void filterAngles(MetadataGroup *group);
	
	float angleForFraction(float frac, int idx);
	
private:
	std::string _startInstance;
	std::string _model_id;
	std::string _endInstance;
	Instance *_instance = nullptr;
	Model *_model = nullptr;
	Instance *_end = nullptr;
	RTMotion _motions;
	
	bool _contributeSVD = false;
	bool _visible = true;

	int _steps = 12;
	
	PlausibleRoute::InterpolationType _type = PlausibleRoute::Polynomial;
	
	PlausibleRoute *_route = nullptr;
};

/* path */
inline void to_json(json &j, const Path &value)
{
	j["model"] = value._model_id;
	j["start"] = value._startInstance;
	j["end"] = value._endInstance;
	j["motions"] = value._motions;

	/*
	j["parameters"] = value._rts;
	j["destination"] = value._destination;
	j["flips"] = value._flips;
	j["waypoints"] = value._wayPoints;
	*/
}

/* path */
inline void from_json(const json &j, Path &value)
{
	value._startInstance = j.at("start");
	value._endInstance = j.at("end");
	value._model_id = j.at("model");

	if (j.count("destination"))
	{
		std::vector<float> dest = j.at("destination");
		std::vector<ResidueTorsion> rts = j.at("parameters");
		std::vector<bool> flips = j.at("flips");
		std::map<int, WayPoints> wps = j.at("waypoints");

		value._motions.vector_from(rts);
		value._motions.motion_angles_from(dest);
		value._motions.waypoints_from(wps);
		value._motions.flips_from(flips);
	}
	else if (j.count("motions"))
	{
		value._motions = j.at("motions");
	}
}

#endif
