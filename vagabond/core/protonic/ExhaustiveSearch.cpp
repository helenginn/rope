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
		(probe, probe->_obj, probe->_exist, &probe->_obj, options, "bond");
		return ip;
	};

	auto make_exist_atom_decree = [this]<class Existing>(Existing *probe)
	-> IteratedProbe *
	{
		if (probe->_exist.is_certain()) { return nullptr; }
		std::vector<hnet::Existence::Values> options = probe->_exist.values();
		IteratedProbe *ip = 
		new IterateDecree<hnet::ExistenceConnector, hnet::Existence::Values>
		(probe, probe->_exist, probe->_exist, nullptr, options, "exist");
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
		(probe, probe->_exist, probe->_exist, &probe->_obj, options, "exist");
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
	OpSet<Probe *> check = _wider;
	GuiltVersion gv = _gv;
	
	std::vector<std::pair<hnet::GetEnergy, std::string>> jobs;
	for (Probe *const &probe : check)
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
	
	auto print = [](const Config &c)
	{
		std::cout << "Config: ";
		for (const unsigned int &i : c)
		{
			std::cout << i << "-";
		}
	};
	
	auto add_result = [this, &check_certainty, &print]
	(const Config &c)
	{
		bool cert = check_certainty(c);
		if (cert)
		{
			_certain++;
		}
		
		if (cert && _configs.count(c) == 0)
		{
			GetScore score = score_wider_clique();
			_configs += c;
			_scores[c] = score;
//			print(c);
//			std::cout << "\t" << score;
//			std::cout << std::endl;
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
	
	auto process_result = [this](const Config &c)
	{
		int i = 0;
		GetScore sc = _scores[c];
		ProbeResult result({}, sc);
		for (IteratedProbe *ip : _iterations)
		{
			int val = c[i];
			hnet::Types type = (ip->type() == "exist"
			                   ? hnet::Types::ExistenceType
			                   : hnet::Types::BondType);
			Probe *const &pr = ip->probe();
			i++;
			
			OneProbe pres = {pr, type, val};
			result.addResult(pres);
		}

		_results.push_back(result);
		return result.results().size();
	};
	
	if (_counter < 0 || (_counter == 0 && (**it).done()))
	{
		for (const Config &c : _configs)
		{
			int extra = process_result(c);
			_total += extra;
		}

		return false;
	}

	return true;
}

