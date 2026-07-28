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

#include "ExhaustiveSearch.h"
#include "CertainStates.h"
#include "Network.h"
#include <unistd.h>
#include <random>
#include <algorithm>

ExhaustiveSearch::ExhaustiveSearch(const OpSet<Probe *> &interesting,
                                   const OpSet<Probe *> &wider)
: _all(interesting), _wider(wider)
{
	_wider += interesting;

	// only probes still uncertain at the outset are worth re-checking as
	// the search proceeds - anything already certain here is a constant
	// for every result, so recording it per-result would just add
	// meaningless columns to the correlation data.
	_wider.filter([](Probe *const &probe)
	{
		return !probe->is_certain();
	});
}

void ExhaustiveSearch::setup()
{
	hnet::ConnectBase::_silent = true;

	auto make_bond_decree = [this](BondProbe *probe) -> IteratedProbe *
	{
		if (probe->_obj.is_certain()) { return nullptr; }
		if (probe->is_covalent()) { return nullptr; };
		std::vector<hnet::Bond::Values> options;
		options = probe->_obj.values();
		IteratedProbe *ip = 
		new IterateDecree<hnet::BondConnector, hnet::Bond::Values>
		(probe, probe->_obj, probe->_exist, options, "bond");
		return ip;
	};

	auto make_exist_atom_decree = [this]<class Existing>(Existing *probe)
	-> IteratedProbe *
	{
		if (probe->_exist.is_certain()) { return nullptr; }
		std::vector<hnet::Existence::Values> options = probe->_exist.values();
		IteratedProbe *ip = 
		new IterateDecree<hnet::ExistenceConnector, hnet::Existence::Values>
		(probe, probe->_exist, probe->_exist, options, "exist");
		return ip;
	};

	auto make_exist_bond_decree = [this]<class Existing>(Existing *probe)
	-> IteratedProbe *
	{
		if (probe->_exist.is_certain()) { return nullptr; }
		if (probe->is_covalent()) { return nullptr; };
		std::vector<hnet::Existence::Values> options = probe->_exist.values();
		IteratedProbe *ip = 
		new IterateDecree<hnet::ExistenceConnector, hnet::Existence::Values>
		(probe, probe->_exist, probe->_exist, options, "exist");
		return ip;
	};
	
	std::cout << "Searching " << _all.size() << " probes" << std::endl;

	for (Probe *const &probe : _all)
	{
		if (probe->is_bond())
		{
			BondProbe *bp = static_cast<BondProbe *>(probe);
			{
				IteratedProbe *ip = make_exist_bond_decree(bp);
				if (ip) { _iterations.push_back(ip); }
			}
		}
		else if (probe->is_atom())
		{
			AtomProbe *ap = static_cast<AtomProbe *>(probe);
			{
				IteratedProbe *ip = make_exist_atom_decree(ap);
				if (ip) { _iterations.push_back(ip); }
			}
		}
	}

	std::random_device rd;
    std::mt19937 g(rd());
	std::vector<IteratedProbe *> copy(_iterations.begin(), _iterations.end());
    std::shuffle(copy.begin(), copy.end(), g);
	_iterations = std::list<IteratedProbe *>(copy.begin(), copy.end());
	
	for (Probe *const &probe : _all)
	{
		if (probe->is_bond())
		{
			BondProbe *bp = static_cast<BondProbe *>(probe);
			IteratedProbe *ip = make_bond_decree(bp);
			if (ip) { _iterations.push_back(ip); }
		}
	}
	
	std::cout << "All iterators:" << std::endl;
	for (IteratedProbe *it : _iterations)
	{
		std::cout << "Iterator: " << it->desc() << std::endl;
	}
	std::cout << std::endl;
	hnet::ConnectBase::_silent = true;
}
	
void ExhaustiveSearch::cleanup()
{
	_states = new CertainStates(_results);
	_results.clear();

	for (IteratedProbe *it : _iterations)
	{
		it->reset(_cv, _m);
		delete it;
	}
	hnet::ConnectBase::_silent = false;
}

void ExhaustiveSearch::search()
{
	setup();
	
	while (true)
	{
		if (!next())
		{
			break;
		}
	}

	cleanup();
}

GetScore ExhaustiveSearch::score_wider_clique()
{
	GuiltVersion gv = _gv;
	
	std::vector<std::pair<hnet::GetEnergy, std::string>> jobs;
	for (Probe *const &probe : _wider)
	{
		hnet::GetEnergy contrib = probe->energy();
		if (contrib)
		{
			jobs.push_back({contrib, probe->desc()});
		}
	}

	return [jobs]()
	{
		float score = 0;
		for (const auto &job : jobs)
		{
			float contrib = job.first();
			if (contrib != contrib)
			{
				std::cout << "NAN for: " << job.second << std::endl;
			}
			score += contrib;
		}

		return score;
	};
}

bool ExhaustiveSearch::next()
{
	if (_iterations.size() <= 1)
	{
		return false;
	}

	auto check_certainty = [this](const Config &c)
	{
		for (const unsigned int &i : c)
		{
			int num_one = 0;
			for (int bit = 0; bit < 32; bit++)
			{
				num_one += ((1 << bit) & i) ? 1 : 0;
			}
			if (num_one != 1)
			{
				return false;
			}
		}
		return true;
	};
	
	
	auto as_configuration = [this]()
	{
		Config results(_iterations.size());
		int i = 0;
		for (IteratedProbe *ip : _iterations)
		{
			float masked = ip->value_as_int();
			results[i] = ip->value_as_int();
			i++;
		}
		return results;
	};
	
	auto add_result = [this, &check_certainty]
	(const Config &c)
	{
		bool cert = check_certainty(c);
		if (cert)
		{
			_certain++;
		}

		if (cert && _configs.count(c) == 0)
		{
			_configs += c;

			// built here, not in a later deferred pass: by the time any
			// pass over all of _configs would run, the search has moved
			// on and the live connector state no longer corresponds to
			// this specific historical config - is_certain()/
			// certainValueAsInt() have to be read while c is still live.
			GetScore score = score_wider_clique();
			ProbeResult result({}, score);

			// _wider only holds probes that were still uncertain at the
			// outset (see the constructor) - anything now certain became
			// so as a side effect of this particular decree combination,
			// covering every decreed probe too (they are a subset of
			// _wider), so nothing else needs recording separately.
			for (Probe *const &probe : _wider)
			{
				if (!probe->is_certain() || probe->is_covalent())
				{
					continue;
				}
				
				if (!probe->is_bond() && !probe->is_atom())
				{
					continue;
				}
				
				if (probe->atom() &&
				    (probe->atom()->elementSymbol() == "H" ||
				     probe->atom()->elementSymbol() == "C"))
				{
					continue;
				}

				if (probe->is_bond())
				{
					result.addResult({probe, hnet::Types::BondType,
					                  probe->certainValueAsInt()});
				}
				else
				{
					result.addResult({probe, hnet::Types::ExistenceType,
					                  probe->certainValueAsInt()});
				}
			}

			_total += result.results().size();
			_results.push_back(result);
		}
	};
	
	auto iterate_until_okay_and_done = [this]
	(IteratedProbe &iterate)
	{
		bool okay = false;
		do 
		{
			okay = iterate(_cv, _m);
		}
		while (!okay && !iterate.done());

		return iterate.done();
	};
	
	auto it = _iterations.begin();
	for (int i = 0; i < _counter; i++)
	{
		it++;
	}

	auto count_down = [&it, this]()
	{
		while (_counter >= _iterations.size())
		{
			it--;
			_counter--;
		}
		
		while ((**it).done())
		{
			(**it).reset(_cv, _m);
			it--;
			_counter--;
			
			if (_counter == 0)
			{
				break;
			}
		}
	};
	auto count_up = [&it, &count_down, this]()
	{
		it++;
		_counter++;
		if (_counter >= _iterations.size())
		{
			count_down();
		}
	};
	
	IteratedProbe &iterate = **it;
	bool done = iterate_until_okay_and_done(iterate);
	Config result = as_configuration();
	add_result(result);
	
	if (iterate.is_certain())
	{
		count_up();
	}
	else if (_counter > 0)
	{
		count_down();
	}
	
	if (_counter < 0 || (_counter == 0 && (**it).done()))
	{
		return false;
	}

	return true;
}

