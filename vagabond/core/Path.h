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
#include "SplitRoute.h"
#include "Polymer.h"

class MetadataGroup;

class Path : public HasMetadata
{
public:
	Path() {};
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

	friend void to_json(json &j, const Path &value);
	friend void from_json(const json &j, Path &value);
	
	PlausibleRoute *toRoute();
	void housekeeping();
	
	void setContributeToSVD(bool contrib)
	{
		_contributeSVD = contrib;
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
	
	void calculateArrays(MetadataGroup *group);
	
	size_t angleArraySize()
	{
		return _angleArrays.size();
	}
	
	void setStepCount(int steps)
	{
		_steps = steps;
	}
	
	MetadataGroup::Array angleArray(int i)
	{
		return _angleArrays[i];
	}
	
	bool operator==(const Path &other) const;
private:
	void calculateDeviations(MetadataGroup *group);

	std::string _startInstance;
	std::string _model_id;
	std::string _endInstance;
	Instance *_instance = nullptr;
	Model *_model = nullptr;
	Instance *_end = nullptr;
	
	bool _contributeSVD = false;
	bool _visible = true;

	std::vector<ResidueTorsion> _rts;
	std::map<int, WayPoints> _wayPoints;
	std::vector<bool> _flips;
	std::vector<MetadataGroup::Array> _angleArrays;
	std::vector<MetadataGroup::Array> _deviationArrays;
	int _steps = 12;

	Route::Point _destination;
	PlausibleRoute::InterpolationType _type = PlausibleRoute::Polynomial;
	
	PlausibleRoute *_route = nullptr;
};

/* path */
inline void to_json(json &j, const Path &value)
{
	j["model"] = value._model_id;
	j["start"] = value._startInstance;
	j["end"] = value._endInstance;
	j["parameters"] = value._rts;
	j["destination"] = value._destination;
	j["flips"] = value._flips;
	j["waypoints"] = value._wayPoints;
}

/* path */
inline void from_json(const json &j, Path &value)
{
	value._startInstance = j.at("start");
	value._endInstance = j.at("end");
	value._model_id = j.at("model");
	std::vector<float> dest = j.at("destination");
	value._destination = dest;
	
	std::vector<ResidueTorsion> rts = j.at("parameters");
	value._rts = rts;

    std::vector<bool> flips = j.at("flips");
    value._flips = flips;

    std::map<int, WayPoints> wps = j.at("waypoints");
    value._wayPoints = wps;
}

#endif
