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
	_modelIsIdentity = (_model == glm::mat4x4(1.f));
}

PositionShifter::~PositionShifter()
{
	_stop = true;

	if (_worker)
	{
		// if the worker is currently parked in unpause()'s wait(), it will
		// never see _stop without being woken up - join() below would
		// otherwise block forever on a paused shifter.
		_waitForPause.notify_all();
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
	// set getters/setters accounting for model view - skipping the
	// matrix-vector multiply entirely when _model is identity (common
	// for 2D-only consumers) avoids doing it on every single getter()/
	// setter() call, of which there are many per gradient() evaluation.
	ele.getter = [&ele, this]()
	{
		if (_modelIsIdentity)
		{
			return ele.raw_getter();
		}

		glm::vec3 raw = ele.raw_getter();
		return glm::vec3(_model * glm::vec4(raw, 1.f));
	};

	ele.setter = [&ele, this](const glm::vec3 &vec)
	{
		if (_modelIsIdentity)
		{
			ele.raw_setter(vec);
			return;
		}

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

glm::vec3 PositionShifter::gradient(size_t idx,
                                    const std::vector<Element *> &elePtrs,
                                    const std::vector<glm::vec3> &positions)
{
	Element &left = *elePtrs[idx];
	const glm::vec3 &leftPos = positions[idx];

	// "how far apart do these two want to be" force law, given an
	// already-computed motion/actual (shared below by both the spring
	// term and repulsion, rather than each recomputing left.getter() -
	// right.getter() separately) - floored to avoid a ~0 divisor (two
	// elements landing at, or very near, the same position). Without
	// that floor, this produces NaN/Inf, which then permanently poisons
	// that element's momentum (Renderable::addToVertices()'s own NaN
	// guard blocks the resulting write, but not the corruption of
	// momentum itself, so every future step for that element is
	// silently dropped too - frozen in place for good). A large seed/
	// target mismatch on the very first few steps (e.g. an initial
	// layout that doesn't yet reflect the real target structure at all)
	// can easily overshoot two elements onto the same spot, so this
	// isn't just a theoretical edge case.
	auto contribution_from = [](const glm::vec3 &motion, float actual,
	                            float target)
	{
		float a = (actual < 0.01f) ? 0.01f : actual;
		float c = 2 * (target - a);
		return motion * c / a;
	};

	glm::vec3 dir = {};
	glm::vec3 repulsion = {};
	int repelled = 0;

	for (size_t j = 0; j < elePtrs.size(); j++)
	{
		if (j == idx)
		{
			continue;
		}

		Element &right = *elePtrs[j];
		glm::vec3 motion = leftPos - positions[j];
		float actual = glm::length(motion);

		// weightFn (if set) is the sole "how related are these two"
		// mechanism (see its own comment in the header) - skipping the
		// spring term entirely below that threshold, rather than always
		// computing it and multiplying by ~0, keeps this loop cheap for
		// callers (e.g. ProtonNetworkView) whose weight is 0 for the
		// vast majority of pairs.
		float weight = _weightFn ? _weightFn(left.reference, right.reference)
		: 1.f;

		if (weight != 0.f)
		{
			float target = _targetFn
			? _targetFn(left.reference, right.reference)
			: glm::length(left.init - right.init);
			if (target < 1.f) target = 1.f;

			dir += contribution_from(motion, actual, target) * weight;
		}

		// never weighted, unlike the spring term above - repulsion's job
		// is purely to stop elements physically overlapping, regardless
		// of how related they are. Weighting it down for distant/
		// unrelated pairs would remove the only force keeping them apart
		// once they happen to end up close on screen, letting them drift
		// onto an identical coordinate with nothing to push them back
		// apart - a self-reinforcing collapse, since two elements sharing
		// a coordinate exactly zeroes that axis's contribution for every
		// pair forever after (motion in that axis is then always 0).
		if (actual < 2)
		{
			repelled++;
			repulsion += contribution_from(motion, actual, 2.f);
		}
	}

	if (repelled > 0)
	{
		repulsion /= (float)repelled;
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
			{
				std::unique_lock<std::mutex> lock(_pauseMutex);
				while (_pause && !_stop)
				{
					_waitForPause.wait(lock);
				}
			}
			// _pauseMutex is released here, before the expensive part -
			// holding it through move() is what starved the GUI thread's
			// pause()/unpause() calls (isPaused() no longer needs it at all).

			if (_stop)
			{
				break;
			}

			move();
		}
	};

	_worker = new std::thread(repeat_moves);
}

void PositionShifter::move()
{
	void *skip = nullptr;
	{
		std::unique_lock<std::mutex> lock(_partialMutex);
		skip = _skip;
	}

	// snapshot every element's CURRENT position once - within a single
	// tick, no element's position actually changes until the deferred
	// "adjustments" apply below, so gradient() previously ended up
	// calling getter() (a lambda plus, unless _modelIsIdentity, a 4x4
	// matrix-vector multiply) up to N times over for the very same
	// element - once per every other element that considered it - an
	// avoidable O(N) of those expensive calls per pair, O(N^2) per tick
	// overall. This makes it O(N) calls total, with the O(N^2) pairwise
	// work down to cheap vector lookups/subtractions instead.
	size_t n = _objects.size();
	std::vector<Element *> elePtrs;
	std::vector<glm::vec3> positions;
	elePtrs.reserve(n);
	positions.reserve(n);

	for (Element &ele : _objects)
	{
		elePtrs.push_back(&ele);
		positions.push_back(ele.getter());
	}

	// plain struct, not std::function - the previous per-element lambda
	// closure (capturing a reference, a vec3, two floats) was large
	// enough to exceed most std::function small-object buffers, meaning
	// a heap allocation per element per tick just to defer the write;
	// this vector is sized once instead.
	struct Adjustment
	{
		Element *ele;
		glm::vec3 motion;
	};

	std::vector<Adjustment> adjustments;
	adjustments.reserve(n);

	float alpha = 0.2;
	float learning_rate = 0.0012;
	glm::vec3 average = {};
	float count = 0;

	for (size_t i = 0; i < n; i++)
	{
		Element &ele = *elePtrs[i];
		if (ele.reference == skip)
		{
			ele.momentum = {};
			continue;
		}

		glm::vec3 move = gradient(i, elePtrs, positions);
		glm::vec3 motion = (1 - alpha) * ele.momentum + alpha * move;
		if (_num == 0)
		{
			motion = move;
		}

		average += motion;
		count++;

		adjustments.push_back({&ele, motion});
	}

	average /= count;

	float z = _z;

	{
		std::unique_lock<std::mutex> lock(_partialMutex);
		for (Adjustment &a : adjustments)
		{
			glm::vec3 current = a.ele->getter();
			current += a.motion * learning_rate - average * learning_rate;
			current.z = z;
			a.ele->setter(current);
			a.ele->momentum = a.motion;
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
