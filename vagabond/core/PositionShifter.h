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

#ifndef __vagabond__PositionShifter__
#define __vagabond__PositionShifter__

#include <map>
#include <atomic>
#include <thread>
#include <functional>
#include <vagabond/utils/glm_import.h>
#include <vagabond/utils/OpSet.h>

class PositionShifter
{
public:
	PositionShifter(const glm::mat4x4 &model);
	~PositionShifter();
	
	typedef std::function<glm::vec3()> Getter;
	typedef std::function<void(const glm::vec3 &)> Setter;
	typedef std::function<void()> Tidy;

	void addPosition(void *ptr, const Getter &getter, const Setter &setter);
	
	void limitSensitivity(void *ptr, OpSet<void *> &white_list)
	{
		_sensitivities[ptr] = white_list;
	}
	
	glm::vec3 getPosition(void *ptr);
	void setPosition(void *ptr, glm::vec3 pos);
	
	void setSkip(void *ptr)
	{
		_skip = ptr;
	}
	
	std::mutex &skip_lock()
	{
		return _mutex;
	}
	
	void setTidy(const Tidy &tidy)
	{
		_tidy = tidy;
	}
	
	void setup();
	void run();
private:
	struct Element
	{
		Getter getter{};
		Setter setter{};

		void *reference;
		Getter raw_getter{};
		Setter raw_setter{};
		glm::vec3 init{};
		glm::vec3 momentum{};
	};
	
	int _num = 0;
	float score();
	void move();
	glm::vec3 gradient(Element *ele);

	void setupGetterSetters();
	std::map<void *, Element *> _map;
	std::map<void *, OpSet<void *>> _sensitivities;

	std::vector<Element> _objects;
	Tidy _tidy{};
	
	void *_skip = nullptr; // not allowed to set/get.
	
	float _z{};

	glm::mat4x4 _model;
	glm::mat4x4 _inv;

	bool _stop{false};
	std::thread *_worker = nullptr;
	std::mutex _mutex{};
};

#endif
