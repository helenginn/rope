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

#include "Cyclic.h"
#include <vagabond/utils/glm_import.h>
#include <vagabond/utils/FileReader.h>
#include <sstream>
#include <iostream>

Cyclic::Cyclic()
{

}

void Cyclic::addNextAtomName(std::string name, float length_to_next,
                             float angle_at_centre)
{
	_num++;
	_names.push_back(name);
	_lengths.push_back(length_to_next);
	_angles.push_back(angle_at_centre);
}

void Cyclic::setup()
{
	addCurve();
	addAtoms();
	updateCurve();
}

void Cyclic::calculateSteps()
{
	_steps.clear();

	if (!_quick)
	{
		for (float d = 0; d < 360; d += _step)
		{
			_steps.push_back(d);
		}
		return;
	}

	for (size_t i = 0; i < _num; i++)
	{
		float step = 1 / (float)_num;
		float prop = step * i;
		if (_pams.nudges.size() > i)
		{
			prop += _pams.nudges[i];
		}
		prop += _pams.offset;
		prop += _offset;
		while (prop < 0) { prop += 1; }
		prop = fmod(prop, 1.f);

		_steps.push_back(prop * 360.);
	}
}

void Cyclic::updateCurve()
{
	if (_quick)
	{
		calculateSteps();
	}

	int n = 0;
	for (size_t i = 0; i < _steps.size(); i++)
	{
		float d = _steps[i];
		glm::vec3 &v = _curve[n];

		float r = deg2rad(d);

		float factor = _sinMult * (r);
		float rise = sin(factor);

		rise *= _pams.z_amplitude * _magnitude;

		v.x = sin(r);
		v.y = cos(r);

		v.z = rise;
		v = glm::normalize(v);
		v *= _pams.radius;

		n++;
	}
	
	if (!_quick)
	{
		addAtoms();
	}
}

void Cyclic::addAtoms()
{
	_idxs.clear();
	
	for (size_t i = 0; i < _num && _quick; i++)
	{
		_idxs.push_back(i);
	}

	for (size_t i = 0; i < _num && !_quick; i++)
	{
		float step = 1 / (float)_num;
		float prop = step * i;
		if (_pams.nudges.size() > i)
		{
			prop += _pams.nudges[i];
		}
		prop += _pams.offset;
		prop += _offset;
		
		while (prop < 0) { prop += 1; }
		
		prop = fmod(prop, 1.f);
		int idx = (int)lrint(prop * (float)pointCount());
		
		while (idx < 0) idx += pointCount();
		while (idx >= pointCount()) idx -= pointCount();
		_idxs.push_back(idx);
	}
}

void Cyclic::addCurve()
{
	_curve.clear();
	
	if (_steps.size() == 0)
	{
		calculateSteps();
	}
	
	for (size_t i = 0; i < _steps.size(); i++)
	{
		_curve.push_back(glm::vec3{});
	}
}

void Cyclic::increment()
{
	return;
	float inc = 1 / (float)pointCount();
	_offset += inc;
	_offset = fmod(_offset, 1.);
	updateCurve();
}

std::string Cyclic::lengths()
{
	std::vector<float> curr = currentLengths();
	std::ostringstream ss;
	int i = 0;

	for (const float &l : curr)
	{
		ss << f_to_str(l - _lengths[i], 3);
		
		if (i < _num - 1)
		{
			ss << std::endl;
		}
		i++;
	}
	
	return ss.str();
}

std::string Cyclic::angles()
{
	std::ostringstream ss;

	for (int i = 0; i < _num; i++)
	{
		glm::vec3 vex[3];

		for (int j = -1; j <= 1; j++)
		{
			int n = (i + j) % _num;
			if (n < 0) n += _num;
			vex[j + 1] = atomPos(n);
		}
		
		vex[0] -= vex[1];
		vex[2] -= vex[1];
		vex[0] = normalize(vex[0]);
		vex[2] = normalize(vex[2]);
		
		float a = glm::angle(vex[0], vex[2]);
		float d = rad2deg(a);
		
		ss << f_to_str(d - _angles[i], 3);
		
		if (i < _num - 1)
		{
			ss << std::endl;
		}
	}
	
	return ss.str();
}

std::vector<float> Cyclic::currentAngles()
{
	std::vector<float> results;

	for (int i = 0; i < _num; i++)
	{
		glm::vec3 vex[3];

		for (int j = -1; j <= 1; j++)
		{
			int n = (i + j) % _num;
			if (n < 0) n += _num;
			vex[j + 1] = atomPos(n);
		}
		
		vex[0] -= vex[1];
		vex[2] -= vex[1];
		vex[0] = normalize(vex[0]);
		vex[2] = normalize(vex[2]);
		
		float a = glm::angle(vex[0], vex[2]);
		float d = rad2deg(a);
		results.push_back(d);
	}
	
	return results;
}

std::vector<float> Cyclic::currentLengths()
{
	std::vector<float> results;
	for (int i = 0; i < _num; i++)
	{
		glm::vec3 vex[2];

		for (int j = 0; j <= 1; j++)
		{
			int n = (i + j) % atomCount();
			vex[j] = atomPos(n);
		}
		
		float l = glm::length(vex[1] - vex[0]);
		results.push_back(l);
	}
	
	return results;
}

float Cyclic::angleScore()
{
	float score = 0;
	std::vector<float> curr = currentAngles();

	for (size_t i = 0; i < _angles.size(); i++)
	{
		float add = (_angles[i] - curr[i]);
		add = deg2rad(add);
		score += add * add;
	}
	
	float diff = sqrt(score);
	return diff;
}

float Cyclic::lengthScore()
{
	float score = 0;
	std::vector<float> curr = currentLengths();

	for (size_t i = 0; i < _lengths.size(); i++)
	{
		float add = (_lengths[i] - curr[i]);
		score += add * add;
	}
	float diff = sqrt(score);
	
	return diff;
}

float Cyclic::score()
{
	float total = 0;
	float len = lengthScore();
	float ang = angleScore();
	total += len + ang;
	
	return total;
}

void Cyclic::refine()
{
	_start = _pams;
	_engine = new ChemotaxisEngine(this);
	_engine->start();
}

size_t Cyclic::parameterCount()
{
	size_t count = Parameters::parameterCount() + atomCount();
	return count;
}

int Cyclic::indexOfName(std::string name) const
{
	for (size_t i = 0; i < atomNames().size(); i++)
	{
		if (name == atomNames()[i])
		{
			return i;
		}
	}

	return -1;
}

int Cyclic::sendJob(const std::vector<float> &all)
{
	Parameters defs(_start);
	defs.addFromVector(all);
	_pams = defs;

	updateCurve();
	
	_issue++;
	float sc = score();
	_scores[_issue] = sc;
	triggerResponse();
	return _issue;
}

float Cyclic::getResult(int *job_id)
{
	if (_scores.size() == 0)
	{
		*job_id = -1;
		return FLT_MAX;
	}

	*job_id = _scores.begin()->first;
	float result = _scores.begin()->second;
	_scores.erase(_scores.begin());

	return result;
}

const glm::vec3 Cyclic::atomPos(int i) const
{
	glm::vec3 v = point(_idxs[i]);
	return v;
}

glm::vec3 Cyclic::point(int i) const
{
	glm::vec4 tmp = _transform * glm::vec4(_curve[i], 1.f);
	return glm::vec3(tmp);
}
