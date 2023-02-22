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

#ifndef __vagabond__Cyclic__
#define __vagabond__Cyclic__

#include <string>
#include <vector>
#include <vagabond/utils/glm_json.h>
#include <vagabond/core/ChemotaxisEngine.h>
#include <vagabond/core/Responder.h>

#include <json/json.hpp>
using nlohmann::json;

class CyclicView;

class Cyclic : public RunsEngine, public HasResponder<Responder<Cyclic>>
{
public:
	Cyclic();
	
	void addNextAtomName(std::string name, float length_to_next,
	                     float angle_at_centre);

	void setup();
	
	void increment();
	
	void addCurve();
	void addAtoms();

	void updateCurve();
	
	std::string angles();
	std::string lengths();
	
	glm::vec3 point(int i) const;
	
	const glm::vec3 atomPos(int i) const;
	
	const std::vector<std::string> &atomNames() const
	{
		return _names;
	}
	
	int indexOfName(std::string name) const;
	
	size_t atomCount()
	{
		return _num;
	}
	
	size_t pointCount()
	{
		return _curve.size();
	}

	float score();
	float lengthScore();
	float angleScore();

	void refine();
	
	void setOffset(float off)
	{
		_offset = off;
	}
	
	void setMagnitude(float mag)
	{
		_magnitude = mag;
	}

	virtual size_t parameterCount();
	virtual int sendJob(std::vector<float> &all);
	virtual float getResult(int *job_id);

	friend void to_json(json &j, const Cyclic &value);
	friend void from_json(const json &j, Cyclic &value);
private:
	std::vector<float> currentLengths();
	std::vector<float> currentAngles();
	
	struct Parameters
	{
		float radius = 1.2;
		float z_amplitude = 0.2;
		float offset = 0.0;
		
		std::vector<float> nudges;
		
		float mult = 100.f;
		
		static size_t parameterCount()
		{
			return 3;
		}
		
		void addFromVector(std::vector<float> &all)
		{
			radius += all[0] / mult;
			z_amplitude += all[1] / mult;
			offset += all[2] / mult;
			
			const int tot = parameterCount();
			
			if (nudges.size() < all.size() - tot)
			{
				nudges.resize(all.size() - tot);
			}
			
			for (size_t i = 0; i < nudges.size(); i++)
			{
				nudges[i] += all[i + tot] / mult;
			}
		}

	};

	friend void to_json(json &j, const Parameters &value);
	friend void from_json(const json &j, Parameters &value);
	
	Parameters _pams{};
	Parameters _start{};
	ChemotaxisEngine *_engine = nullptr;

	int _num = 0;
	float _offset = 0;
	int _sinMult = 2;
	std::vector<int> _idxs;
	std::vector<glm::vec3> _curve;

	std::vector<std::string> _names;
	std::vector<float> _lengths;
	std::vector<float> _angles;

	std::map<int, double> _scores;
	int _issue = 0;
	float _step = 0.5;
	float _magnitude = 1.;
};

inline void to_json(json &j, const Cyclic::Parameters &value)
{
	j["radius"] = value.radius;
	j["z_amplitude"] = value.z_amplitude;
	j["offset"] = value.offset;
	j["nudges"] = value.nudges;
}

inline void from_json(const json &j, Cyclic::Parameters &value)
{
	value.radius = j.at("radius");
	value.z_amplitude = j.at("z_amplitude");
	value.offset = j.at("offset");
	std::vector<float> nudges = j.at("nudges");
	value.nudges = nudges;
}

inline void to_json(json &j, const Cyclic &value)
{
	j["names"] = value._names;
	j["num"] = value._num;
	j["sinmult"] = value._sinMult;
	j["lengths"] = value._lengths;
	j["angles"] = value._angles;
	j["step"] = value._step;
	j["params"] = value._pams;
}

inline void from_json(const json &j, Cyclic &value)
{
	value._names = j.at("names");
	value._num = j.at("num");
	value._sinMult = j.at("sinmult");
	std::vector<float> lengths = j.at("lengths");
	value._lengths = lengths;
	std::vector<float> angles = j.at("angles");
	value._angles = angles;
	value._step = j.at("step");
	value._pams = j.at("params");
	
	value.setup();
}

#endif
