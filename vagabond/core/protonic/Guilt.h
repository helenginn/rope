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

#ifndef __vagabond__Guilt__
#define __vagabond__Guilt__

#include <vagabond/utils/OpSet.h>
#include <functional>

class Guilt
{
public:
	void addGuilt(void *guilt)
	{
		if (_all.count(guilt))
		{
			return;
		}

		if (_current == nullptr)
		{
			_current = guilt;
			_top = guilt;
		}
		else
		{
			_guiltTree[_current] = guilt;
			_backwards[guilt] = _current;
			_current = guilt;
		}
		
		_all += guilt;
	}
	
	OpSet<void *> rollBackBefore(void *guilt)
	{
		OpSet<void *> acquired;
		
		std::function<void(OpSet<void *>)> get_next;
		get_next = [&acquired, &get_next, this]
		(const OpSet<void *> &next)
		{
			for (void *guilt : next)
			{
				acquired += guilt;
				get_next(_guiltTree[guilt]);
			}
		};
		
		OpSet<void *> start = _guiltTree[guilt];
		get_next(start);
		acquired += guilt;
		_current = _backwards[guilt];
		if (_current == _top)
		{
			std::cout << "Clearing the guilt tree!" << std::endl;
			_guiltTree.clear();
			_backwards.clear();
			_all = {_current};
		}
		return acquired;
	}
	
	static Guilt &guilt()
	{
		return _guilt;
	}
private:
	void *_current{};
	void *_top{};
	OpSet<void *> _all;
	std::map<void *, OpSet<void *>> _guiltTree;
	std::map<void *, void *> _backwards;

	static Guilt _guilt;
};


#endif
