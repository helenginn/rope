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
#include "TorsionData.h"
#include "PlausibleRoute.h"
#include "Polymer.h"

class Model;
class TorsionData;

class Path : public HasMetadata, Responder<HasMetadata>
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
	
	void setStepCount(int steps)
	{
		_steps = steps;
	}
	
	bool operator==(const Path &other) const;
	
	void filterAngles(TorsionData *group);
	
	float angleForFraction(float frac, int idx);
	
	virtual void sendObject(std::string tag, void *object);
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
}

/* path */
inline void from_json(const json &j, Path &value)
{
	value._startInstance = j.at("start");
	value._endInstance = j.at("end");
	value._model_id = j.at("model");
	value._motions = j.at("motions");
}

#endif
