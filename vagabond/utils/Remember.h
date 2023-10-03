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

#ifndef __vagabond__Remember__
#define __vagabond__Remember__

#include <map>

template <typename In, typename Out>
struct Remember
{
	Remember(std::function<Out(In)> &func) : _func(func) {} 
	Remember(std::map<In, Out> &map) : _results(map) {} 

	Out operator()(In in)
	{
		if (_results.count(in))
		{
			return _results.at(in);
		}

		if (_func)
		{
			Out out = _func(in);
			_results[in] = out;
			return out;
		}
		else
		{
			return Out{};
		}
	}

	std::function<Out(In)> _func{};
	std::map<In, Out> _results{};
};


#endif
