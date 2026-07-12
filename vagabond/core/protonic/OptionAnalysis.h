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

#ifndef __vagabond__OptionAnalysis__
#define __vagabond__OptionAnalysis__

#include "hnet.h"
#include "Connector.h"

class Network;

namespace hnet
{
class OptionAnalysis
{
public:
	OptionAnalysis(Network &network);
	               
	void setOptions(const std::vector<CountConnector *> &group)
	{
		_group = group;
	}

	void addState(const std::vector<Count::Values> &state);
	
	size_t stateCount()
	{
		return _states.size();
	}
	
	void initialConstants();
	void deriveConstraints();
private:
	typedef std::vector<Count::Values> ValueGroup;

	std::vector<ValueGroup> statesGiven(CountConnector *cc, 
	                                    const Count::Values &fixed);

	OptionAnalysis::ValueGroup 
	optionsForStates(const std::vector<ValueGroup> &states);

	ValueGroup optionsGiven(CountConnector *cc, 
	                        const Count::Values &fixed);


	void checkElimination(CountConnector *cc, 
	                      const Count::Values &eliminate);

	std::vector<CountConnector *> _group;
	ValueGroup _allOptions;
	std::vector<ValueGroup> _states;
	Network &_network;
};
}

#endif
