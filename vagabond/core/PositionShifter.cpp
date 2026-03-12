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

void PositionShifter::removePointer(void *ptr)
{
	for (auto it = _objects.begin(); it != _objects.end(); it++)
	{
		if (it->reference == ptr)
		{
			_objects.erase(it);
			break;
		}
	}

	_sensitivities.erase(ptr);
	_map.erase(ptr);
	_ptrs.erase(ptr);
}

void PositionShifter::addPosition(void *ptr, const Getter &init,
                                  const Getter &getter, const Setter &setter)
{
	if (_ptrs.count(ptr))
	{
		return;
	}

	Element ele{};
	ele.reference = ptr;
	ele.raw_getter = getter;
	ele.raw_setter = setter;
	ele.init = init();
	_objects.push_back(ele);
	_ptrs.insert(ptr);
	
	setupGetterSetters(_objects.back());
	adjustZ(_objects.back());
	tidy();
}

void PositionShifter::setupGetterSetters(Element &ele)
{
	// set getters/setters accounting for model view
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

void PositionShifter::adjustZ(Element &ele)
{
	// adjust the z axis if we haven't had many pointers yet
	if (_n <= 10)
	{
		_z *= _n;
		_z += ele.getter().z;
		_n++;
		_z /= (float)_n;
	}

	// go through all objects and update the z-axis for all of them.
	for (Element &ele : _objects)
	{
		glm::vec3 vec = ele.getter();
		vec.z = _z;
		ele.setter(vec);
	}
}

glm::vec3 PositionShifter::gradient(Element *ele)
{
	Element &left = *ele;
	OpSet<Element *> process;
	OpSet<Element *> done;
	OpSet<Element *> all;

	for (Element &right : _objects)
	{
		if (&left != &right)
		{
			all.insert(&right);
		}

		if (_sensitivities.count(left.reference) && 
		    !_sensitivities[left.reference].count(right.reference))
		{
			continue;
		}

		process.insert(&right);
	}

	auto make_target = [](float target)
	{
		return [target](Element &left, Element &right) -> float
		{
			return target;
		};
	};

	auto get_target = [](Element &left, Element &right) -> float
	{
		float target = glm::length(left.init - right.init);
		if (target < 1.f) target = 1.f;
		return target;
	};
	
	auto contribution_from = []<typename GetTarget>
	(const GetTarget &getTarget, Element &left, Element &right)
	{
		float target = getTarget(left, right);
		glm::vec3 motion = left.getter() - right.getter();
		float actual = glm::length(motion);
		float c = 2 * (target - actual);
		glm::vec3 contrib = motion * c / actual;
		return contrib;
	};
	
	glm::vec3 dir = {};
	int depth = 2;

	while (depth > 0)
	{
		OpSet<Element *> tmp;
		for (Element *right_ptr : process)
		{
			Element &right = *right_ptr;

			if (&left == &right)
			{
				continue;
			}

			glm::vec3 contrib = contribution_from(get_target, left, right);

			dir += contrib;
			
			for (void *sensitive : _sensitivities[right.reference])
			{
				if (sensitive && _map[sensitive] && !done.count(_map[sensitive]))
				{
					tmp.insert(_map[sensitive]);
					done.insert(_map[sensitive]);
				}
			}
		}
		process = tmp;
		depth--;
	}
	
	OpSet<Element *> remaining = all - done;

	int count = 0;
	glm::vec3 repulsion = {};
	for (Element *right_ptr : remaining)
	{
		Element &right = *right_ptr;
		glm::vec3 motion = left.getter() - right.getter();
		float actual = glm::length(motion);
		if (actual < 2)
		{
			count++;
			glm::vec3 contrib = contribution_from(make_target(2), left, right);
			repulsion += contrib;
		}
	}
	if (count > 0)
	{
		repulsion /= (float)count;
	}

	return dir + repulsion;
}

void PositionShifter::tidy()
{
	for (const Tidy &tidy : _tidyJobs)
	{
		tidy();
	}
}

void PositionShifter::pause()
{
	std::unique_lock<std::mutex> lock(_pauseMutex);
	_pause = true;
}

void PositionShifter::unpause()
{
	std::unique_lock<std::mutex> lock(_pauseMutex);
	_pause = false;
	_waitForPause.notify_one();
}

void PositionShifter::run()
{
	auto repeat_moves = [this]()
	{
		while (!_stop)
		{
			std::unique_lock<std::mutex> lock(_pauseMutex);
			if (_pause)
			{
				_waitForPause.wait(lock);
			}
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
		std::unique_lock<std::mutex> lock(_partialMutex);
		skip = _skip;
	}

	float alpha = 0.2;
	float learning_rate = 0.0012;
	glm::vec3 average = {};
	float count = 0;
	for (Element &ele : _objects)
	{
		if (ele.reference == skip)
		{
			ele.momentum = {};
			continue;
		}

		glm::vec3 move = gradient(&ele);
		glm::vec3 motion = (1 - alpha) * ele.momentum + alpha * move;
		if (_num == 0)
		{
			motion = move;
		}
		average += motion;
		count++;
		
		float z = _z;
		auto adjust = [&ele, &average, motion, learning_rate, z]()
		{
			glm::vec3 current = ele.getter();
			glm::vec3 little = motion * learning_rate;
			glm::vec3 correction = average * learning_rate;
			current += motion * learning_rate - correction;
			current.z = z;
			ele.setter(current);
			ele.momentum = motion;
		};
		
		adjustments.push_back(adjust);
	}
	
	average /= count;

	{
		std::unique_lock<std::mutex> lock(_partialMutex);
		for (auto adjust : adjustments)
		{
			adjust();
		}
	}
	
	tidy();
	
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
