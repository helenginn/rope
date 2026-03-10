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

#include <iostream>
#include "PositionShifter.h"

PositionShifter::PositionShifter(const glm::mat4x4 &model)
{
	_model = (model);
	_inv = glm::inverse(_model);
}

PositionShifter::~PositionShifter()
{
	_stop = true;

	if (_worker)
	{
		_worker->join();
		delete _worker;
	}
}

void PositionShifter::addPosition(void *ptr, const Getter &getter,
                                  const Setter &setter)
{
	Element ele{};
	ele.reference = ptr;
	ele.raw_getter = getter;
	ele.raw_setter = setter;
	ele.init = getter();
	_objects.push_back(ele);
}

void PositionShifter::setupGetterSetters()
{
	// set getters/setters accounting for model view
	for (Element &ele : _objects)
	{
		ele.getter = [&ele, this]()
		{
			glm::vec3 raw = ele.raw_getter();
			return glm::vec3(_model * glm::vec4(raw, 1.f));
		};

		ele.setter = [&ele, this](const glm::vec3 &vec)
		{
			glm::vec3 result = glm::vec3(_inv * glm::vec4(vec, 1.f));
			ele.raw_setter(result);
		};
		
		if (ele.reference)
		{
			_map[ele.reference] = &ele;
		}
	}

	// reset the z axis

	float sum_z = 0;
	for (Element &ele : _objects)
	{
		glm::vec3 with_z = ele.getter();
		sum_z += with_z.z;
	}
	_z = sum_z / (float)_objects.size();

	for (Element &ele : _objects)
	{
		glm::vec3 vec = ele.getter();
		vec.z = _z;
		ele.setter(vec);
	}
}

float PositionShifter::score()
{
	float score = {};

	for (Element &left : _objects)
	{
		for (Element &right : _objects)
		{
			if (&left == &right)
			{
				continue;
			}

			float target = glm::length(left.init - right.init);
			float actual = glm::length(left.raw_getter() - right.raw_getter());
			
			float r = target / actual;
			float contrib = (r - 1) * (r - 1) / (1 + r * r);
			score += contrib;
		}
	}
	
	return score;
}

glm::vec3 PositionShifter::gradient(Element *ele)
{
	glm::vec3 dir = {};
	Element &left = *ele;
	int count = 0;

	for (Element &right : _objects)
	{
		if (&left == &right)
		{
			continue;
		}
		
		if (_sensitivities.count(left.reference) && 
		    !_sensitivities[left.reference].count(right.reference))
		{
			continue;
		}

		float target = glm::length(left.init - right.init);
		glm::vec3 motion = left.getter() - right.getter();
		float actual = glm::length(motion);

		float c = 2 * (target - actual);
		glm::vec3 contrib = motion * c / actual;

		dir += contrib;
		count++;
	}

	return dir;
}

//(2(x-1)(1+x^2)-2x(x-1)^2)/(1+x^2)^2
//

void PositionShifter::setup()
{
	setupGetterSetters();

	_tidy();
}

void PositionShifter::run()
{
	auto repeat_moves = [this]()
	{
		while (!_stop)
		{
			move();
		}
	};

	_worker = new std::thread(repeat_moves);
}

void PositionShifter::move()
{
	std::vector<std::function<void()>> adjustments;
	
	void *skip = nullptr;
	{
		std::unique_lock<std::mutex> lock(_mutex);
		skip = _skip;
	}

	float alpha = 0.5;
	float learning_rate = 0.0001;
	for (Element &ele : _objects)
	{
		if (ele.reference == skip)
		{
			continue;
		}

		glm::vec3 move = gradient(&ele);
		glm::vec3 motion = (1 - alpha) * ele.momentum + alpha * move;
		if (_num == 0)
		{
			motion = move;
		}
		
		auto adjust = [&ele, motion, learning_rate]()
		{
			glm::vec3 current = ele.getter();
			glm::vec3 little = motion * learning_rate;
			if (glm::length(motion) > 1e-5)
			{
				current += motion * learning_rate;
			}
			ele.setter(current);
			ele.momentum = motion;
		};
		
		adjustments.push_back(adjust);
	}

	{
		std::unique_lock<std::mutex> lock(_mutex);
		for (auto adjust : adjustments)
		{
			adjust();
		}
	}
	
	_tidy();
	
	_num++;
}

void PositionShifter::setPosition(void *ptr, glm::vec3 pos)
{
	Element *e = _map[ptr];
	if (e)
	{
		e->setter(pos);
	}
}

glm::vec3 PositionShifter::getPosition(void *ptr)
{
	Element *e = _map[ptr];
	if (e)
	{
		return e->getter();
	}
	else return {};
}
