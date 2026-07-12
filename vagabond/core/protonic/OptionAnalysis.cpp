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

#include "OptionAnalysis.h"
#include "Connector.h"
#include "Constraint.h"
#include "Network.h"

using namespace hnet;

OptionAnalysis::OptionAnalysis(Network &network)
: _network(network)
{
	Count::Values init;
	init_unassigned(init);
	_allOptions = ValueGroup(_group.size(), init);
}

auto make_is_this_fixed_connector(const std::vector<CountConnector *> &ref,
                                  CountConnector *cc)
{
	return [&ref, cc](int i)
	{
		return (ref[i] == cc);
	};
}
	
std::vector<OptionAnalysis::ValueGroup> 
OptionAnalysis::statesGiven(CountConnector *cc, const Count::Values &expect)
{
	auto is_this_fixed = make_is_this_fixed_connector(_group, cc);

	auto this_state_has_expected_value = [&](const ValueGroup &state)
	{
		for (int i = 0; i < state.size(); i++)
		{
			if (is_this_fixed(i))
			{
				return (bool)(state[i] & expect);
			}
		}
		return false;
	};

	std::vector<ValueGroup> list;
	for (const ValueGroup &state : _states)
	{
		if (this_state_has_expected_value(state))
		{
			list.push_back(state);
		}
	}
	return list;
}

OptionAnalysis::ValueGroup 
OptionAnalysis::optionsForStates(const std::vector<ValueGroup> &states)
{
	ValueGroup ret = ValueGroup(_group.size(), {});

	for (const ValueGroup &state : states)
	{
		auto it = ret.begin();
		for (const Count::Values &val : state)
		{
			*it = Count::Values(*it | val);
			it++;
		}
	}

	return ret;
}

OptionAnalysis::ValueGroup 
OptionAnalysis::optionsGiven(CountConnector *cc, 
                             const Count::Values &expectation)
{
	std::vector<ValueGroup> states = statesGiven(cc, expectation);
	std::cout << "Collected " << states.size() << " out of " << 
	_states.size() << " states" << std::endl;
	ValueGroup options = optionsForStates(states);
	return options;
}

void OptionAnalysis::addState(const std::vector<Count::Values> &state)
{
	_states.push_back(state);
}

void OptionAnalysis::checkElimination(CountConnector *cc, 
                                      const Count::Values &eliminate)
{
	Count::Values all_but_eliminate;
	init_unassigned(all_but_eliminate);
	all_but_eliminate = (Count::Values)(~eliminate & all_but_eliminate);

	// find all options to start with
	auto is_this_fixed = make_is_this_fixed_connector(_group, cc);

	auto total_options_for = [this, is_this_fixed](CountConnector *const &cc)
	-> Count::Values
	{
		for (int i = 0; i < _allOptions.size(); i++)
		{
			if (is_this_fixed(i))
			{
				return _allOptions[i];
			}
		}
		return {};
	};

	Count::Values everything = total_options_for(cc);
	Count::Values minus_eliminate = 
	(Count::Values)(everything & all_but_eliminate);

	// time to build the constraints
	ValueGroup limited = optionsGiven(cc, minus_eliminate);

	for (int i = 0; i < limited.size(); i++)
	{
		// we don't need to constrain our same connector on its own value
		if (is_this_fixed(i))
		{
			continue; 
		}

		Count::Values all = _allOptions[i];
		
		// no change means no need to constrain
		if (all == limited[i])
		{
			continue;
		}
		
		std::cout << _group[i]->desc() << " got limited from " << all
		<< " to " << limited[i] << std::endl;

		Count::Values limit = (Count::Values)~limited[i];
		// otherwise if cc is equal to eliminate, then imposed connector 
		// gets limited
		CountConnector *imposed = _group[i];
		auto if_cc_loses_eliminate = [cc, eliminate]() -> bool
		{
			Count::Values curr = cc->value();
			Count::Values intersect = (Count::Values)(cc->value() & eliminate);
			return !intersect;
		};
		
		std::cout << "Making constraint on " << imposed->desc() << " due to "
		<< cc->desc() << std::endl;
		StrictCount &result = 
		_network.add_constraint(new StrictCount({cc}, if_cc_loses_eliminate, 
		                                        *imposed, limited[i]));
	}
}

void OptionAnalysis::deriveConstraints()
{
	_allOptions = optionsForStates(_states);

	for (int i = 0; i < _allOptions.size(); i++)
	{
		CountConnector *const &cc = _group[i];
		std::vector<int> possibilities = possible_values(_allOptions[i]);
		if (possibilities.size() <= 1)
		{
			continue;
		}

		for (const int &p : possibilities)
		{
			Count::Values val = int_to_count(p);
			checkElimination(cc, val);
		}
	}
}

void OptionAnalysis::initialConstants()
{
	_allOptions = optionsForStates(_states);

	for (int i = 0; i < _allOptions.size(); i++)
	{
		CountConnector *cc = _group[i];
		_network.add_constraint(new CountConstant(*cc, _allOptions[i]));
	}
}
