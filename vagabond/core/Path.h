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
class PathData;
class TorsionData;

class Path : public HasMetadata, Responder<HasMetadata>
{
public:
	Path() {};
	~Path();
	Path(PlausibleRoute *pr);
	void cleanupRoute();
	
	void signalDeletion();

	const Metadata::KeyValues metadata(Metadata *source) const;

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
	
	const RTPeptideTwist &twists() const
	{
		return _twists;
	}
	
	const RTMotion &motions() const
	{
		return _motions;
	}
	
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
		std::string str = desc();
		if (_hash.length())
		{
			str += " " + _hash;
		}
		return str;
	}

	virtual bool displayable() const /* so these aren't points in the map */
	{
		return true;
	}
	
	const bool &visible() const
	{
		return _visible;
	}
	
	void setStepCount(int steps)
	{
		_steps = steps;
	}
	
	float clashScore() 
	{
		return _clash;
	}
	
	float momentumScore() 
	{
		return _momentum;
	}
	
	float activationEnergy() 
	{
		return _activationEnergy;
	}


	float torsionEnergy() 
	{
		return _torsionEnergy;
	}
	
	bool operator==(const Path &other) const;
	
	void filterAngles(TorsionData *group);
	
	float angleForFraction(float frac, int idx);
	
	virtual void sendObject(std::string tag, void *object);

	virtual void addToData(PathData *pd);
	
	bool hasScores()
	{
		return _momentum != FLT_MAX && _clash != FLT_MAX;
	}
private:
	void acquireSingleProperties(Route *route);

	std::string _startInstance;
	std::string _model_id;
	std::string _endInstance;
	Instance *_instance = nullptr;
	Model *_model = nullptr;
	Instance *_end = nullptr;
	RTMotion _motions;
	RTPeptideTwist _twists;
	
	bool _contributeSVD = false;
	bool _visible = true;
	
	float _activationEnergy = FLT_MAX;
	float _torsionEnergy = FLT_MAX;
	float _momentum = FLT_MAX;
	float _clash = FLT_MAX;
	
	float _maxMomentumDistance = 8.f;
	float _maxClashDistance = 15.f;
	
	std::string _hash;

	int _steps = 12;
	
	PlausibleRoute *_route = nullptr;
};

/* path */
inline void to_json(json &j, const Path &value)
{
	j["model"] = value._model_id;
	j["start"] = value._startInstance;
	j["end"] = value._endInstance;
	j["motions"] = value._motions;
	j["twists"] = value._twists;

	j["hash"] = value._hash;
	j["clash_score"] = value._clash;
	j["momentum_score"] = value._momentum;
	j["activation_energy"] = value._activationEnergy;
	j["torsion_energy"] = value._torsionEnergy;
	j["maximum_momentum_distance"] = value._maxMomentumDistance;
	j["maximum_clash_distance"] = value._maxClashDistance;
}

/* path */
inline void from_json(const json &j, Path &value)
{
	value._startInstance = j.at("start");
	value._endInstance = j.at("end");
	value._model_id = j.at("model");
	value._motions = j.at("motions");
	
	if (j.count("twists"))
	{
		value._twists = j.at("twists");
	}
	
	if (j.count("hash"))
	{
		value._clash = j.at("clash_score");
		value._momentum = j.at("momentum_score");
		value._hash = j.at("hash");
	}
	
	if (j.count("maximum_momentum_distance"))
	{
		value._maxMomentumDistance = j.at("maximum_momentum_distance");
	}
	
	if (j.count("maximum_clash_distance"))
	{
		value._maxClashDistance = j.at("maximum_clash_distance");
	}
	
	if (j.count("activation_energy"))
	{
		value._activationEnergy = j.at("activation_energy");
	}
	
	if (j.count("torsion_energy"))
	{
		value._torsionEnergy = j.at("torsion_energy");
	}
}

#endif
