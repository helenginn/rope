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

#ifndef __vagabond__MemoryTangle__
#define __vagabond__MemoryTangle__

#include <vector>
#include <vagabond/utils/OpSet.h>

class Atom;
class Untangle;
class TangledBond;

class MemoryTangle
{
public:
	MemoryTangle(Untangle *untangle);

	void tryTangle(Atom *atom);
	
	void setMaxLead(int lead)
	{
		_maxLead = lead;
	}
	
	void cancel()
	{
		_cancelled = true;
	}
	
	const bool &cancelled() const
	{
		return _cancelled;
	}

	int numberTanglesWithoutImprovement();
private:
	bool incrementTangleTrial();

	struct TangledState
	{
		TangledState(MemoryTangle *parent, Atom *atom);
		
		bool inLastConfiguration()
		{
			return (option == optionCount() - 1);
		}
		
		typedef std::pair<std::string, std::string> StringString;

		OpSet<StringString> all_switches(Atom *atom);
		
		void touch();
		void increment();
		void setOption(int idx);
		int optionCount();

		TangledBond *bond;
		Atom *atom;
		std::vector<StringString> options;
		int option;
		MemoryTangle *memory;
	};
	
	void reinstate(std::vector<TangledState> &states);
	
	void retouch(std::vector<TangledState> &states);

	std::vector<TangledState> _tangles;
	int _improved = 0;
	int _maxLead = 3;
	bool _cancelled = false;
	
	Untangle *_untangle;
};

#endif
