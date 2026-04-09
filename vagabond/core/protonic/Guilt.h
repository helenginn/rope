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

typedef long int GuiltVersion;

class Guilt
{
public:
	static GuiltVersion issueNext()
	{
		return ++(_guilt._worst);
	}

	static GuiltVersion popLast()
	{
		return --(_guilt._worst);
	}
	
	static Guilt &guilt()
	{
		return _guilt;
	}
private:
	OpSet<void *> _all;
	GuiltVersion _worst{};
	std::map<void *, OpSet<void *>> _guiltTree;
	std::map<void *, void *> _backwards;

	static Guilt _guilt;
};


#endif
