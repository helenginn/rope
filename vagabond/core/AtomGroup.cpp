// vagabond
// Copyright (C) 2019 Helen Ginn
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

#include "AtomGroup.h"
#include "BondLength.h"
#include "../utils/FileReader.h"
#include "BondTorsion.h"
#include "BondCalculator.h"
#include "PositionRefinery.h"

#include <algorithm>
#include <iostream>
#include <thread>

AtomGroup::AtomGroup()
{

}

void AtomGroup::cancelRefinement()
{
	if (_refine != nullptr)
	{
		std::cout << "Finishing engine" << std::endl;
		_engine->finish();
		_engine = nullptr;
		_refine->join();
		delete _refine;
		_refine = nullptr;
	}
}

AtomGroup::~AtomGroup()
{
	deleteBondstraints();
	
}

bool AtomGroup::hasAtom(Atom *a)
{
	AtomVector::iterator it = std::find(_atoms.begin(), _atoms.end(), a);

	return (it != _atoms.end());
}

void AtomGroup::operator+=(Atom *a)
{
	if (!hasAtom(a))
	{
		_atoms.push_back(a);
	}
}

void AtomGroup::operator-=(Atom *a)
{
	AtomVector::iterator it = std::find(_atoms.begin(), _atoms.end(), a);

	if (it != _atoms.end())
	{
		_atoms.erase(it);
	}
}

AtomPtr AtomGroup::operator[](int i) const
{
	return _atoms[i];
}

AtomPtr AtomGroup::operator[](std::string str) const
{
	return firstAtomWithName(str);
}

Atom *AtomGroup::possibleAnchor(int i)
{
	if (_anchors.size() == 0)
	{
		findPossibleAnchors();
	}

	return _anchors[i];
}

size_t AtomGroup::possibleAnchorCount()
{
	if (_anchors.size() == 0)
	{
		findPossibleAnchors();
	}

	return _anchors.size();
}

void AtomGroup::findPossibleAnchors()
{
	AtomVector trials = _atoms;
	AtomVector tooManyConnections;
	_anchors.clear();

	for (size_t i = 0; i < trials.size(); i++)
	{
		/* we don't want to start on a hydrogen */
		if (trials[i]->elementSymbol() == "H")
		{
			trials[i] = nullptr;
			continue;
		}
		
		int nonHydrogen = 0;
		for (size_t j = 0; j < trials[i]->bondLengthCount(); j++)
		{
			Atom *other = trials[i]->connectedAtom(j);
			
			if (other->elementSymbol() != "H")
			{
				nonHydrogen++;
			}
		}

		if (nonHydrogen > 1)
		{
			tooManyConnections.push_back(trials[i]);
			trials[i] = nullptr;
		}
	}

	for (size_t i = 0; i < trials.size(); i++)
	{
		if (trials[i] != nullptr)
		{
			_anchors.push_back(trials[i]);
		}
	}

	if (_anchors.size() == 0)
	{
		_anchors = tooManyConnections;
	}
}

AtomVector AtomGroup::atomsWithName(std::string name) const
{
	to_upper(name);
	AtomVector v;
	for (size_t i = 0; i < _atoms.size(); i++)
	{
		if (_atoms[i]->atomName() == name)
		{
			v.push_back(_atoms[i]);
		}
	}

	return v;
}

Atom *AtomGroup::firstAtomWithName(std::string name) const
{
	to_upper(name);

	for (size_t i = 0; i < _atoms.size(); i++)
	{
		if (_atoms[i]->atomName() == name)
		{
			return _atoms[i];
		}
	}

	return nullptr;
}

void AtomGroup::recalculate()
{
	Atom *anchor = possibleAnchor(0);

	BondCalculator calculator;
	calculator.setPipelineType(BondCalculator::PipelineAtomPositions);
	calculator.setMaxSimultaneousThreads(2);
	calculator.addAnchorExtension(anchor);
	calculator.setup();

	calculator.start();

	Job job{};
	job.requests = JobExtractPositions;
	calculator.submitJob(job);
	
	Result *result = calculator.acquireResult();
	calculator.finish();
	result->transplantPositions();

	delete result;
}

void AtomGroup::refinePositions()
{
	PositionRefinery *refinery = new PositionRefinery(this);
	
	cancelRefinement();
	
	if (_engine != nullptr)
	{
		delete _engine;
		_engine = nullptr;
	}
	
	if (_refine != nullptr)
	{
		delete _refine;
		_refine = nullptr;
	}

	_engine = refinery;
	_refine = new std::thread(&PositionRefinery::backgroundRefine, refinery);
}

std::vector<AtomGroup *> AtomGroup::connectedGroups()
{
	std::vector<AtomGroup *> groups;
	groups.push_back(new AtomGroup(*this));
	return groups;
}

