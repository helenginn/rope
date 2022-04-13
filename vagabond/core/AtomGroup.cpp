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
#include "BondSequence.h"
#include "../utils/FileReader.h"
#include "BondTorsion.h"
#include "TorsionBasis.h"
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
	if (_engine != nullptr)
	{
		_engine->finish();
	}
}

void AtomGroup::cleanupRefinement()
{
	if (_engine != nullptr)
	{
		delete _engine;
		_engine = nullptr;
	}
	
	if (_refine != nullptr)
	{
		_refine->join();
		delete _refine;
		_refine = nullptr;
	}
}

AtomGroup::~AtomGroup()
{
	cancelRefinement();
	deleteBondstraints(this);

	/*
	delete _engine;
	delete _refine;
	_engine = nullptr;
	_refine = nullptr;
	*/
}

bool AtomGroup::hasAtom(Atom *a)
{
	AtomVector::iterator it = std::find(_atoms.begin(), _atoms.end(), a);

	return (it != _atoms.end());
}

void AtomGroup::operator+=(Atom *a)
{
	if (a != nullptr && !hasAtom(a))
	{
		_atoms.push_back(a);
		
		for (size_t j = 0; j < a->bondLengthCount(); j++)
		{
			addBondstraint(a->bondLength(j));
		}

		_anchors.clear();
	}
}

void AtomGroup::operator-=(Atom *a)
{
	AtomVector::iterator it = std::find(_atoms.begin(), _atoms.end(), a);

	if (it != _atoms.end())
	{
		_atoms.erase(it);
		_anchors.clear();
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

Atom *AtomGroup::chosenAnchor()
{
	if (_chosenAnchor != nullptr)
	{
		return _chosenAnchor;
	}

	int count = possibleAnchorCount();
	if (count == 0)
	{
		return nullptr;
	}

	_chosenAnchor = possibleAnchor(0);
	return _chosenAnchor;
}

void AtomGroup::recalculate()
{
	connectedGroups();

	for (size_t i = 0; i < _subgroups.size(); i++)
	{
		Atom *anchor = _subgroups[i]->chosenAnchor();

		BondCalculator calculator;
		calculator.setPipelineType(BondCalculator::PipelineAtomPositions);
		calculator.setMaxSimultaneousThreads(1);
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
}

void AtomGroup::refinePositions()
{
	PositionRefinery *refinery = new PositionRefinery(this);

	cancelRefinement();
	cleanupRefinement();

	_engine = refinery;
	_refine = new std::thread(&PositionRefinery::backgroundRefine, refinery);
}

void AtomGroup::organiseSamples(int n)
{

}

void AtomGroup::remove(AtomGroup *g)
{
	for (size_t i = 0; i < g->size(); i++)
	{
		remove((*g)[i]);
	}
}

void AtomGroup::add(AtomGroup *g)
{
	for (size_t i = 0; i < g->size(); i++)
	{
		add((*g)[i]);
	}
}

std::vector<AtomGroup *> AtomGroup::connectedGroups()
{
	if (_subgroups.size())
	{
		return _subgroups;
	}

	std::vector<AtomGroup *> groups;
	AtomGroup total = AtomGroup(*this);
	
	while (total.size() > 0)
	{chosenAnchor();
		BondSequence *sequence = new BondSequence();
		Atom *anchor = total.possibleAnchor(0);

		sequence->addToGraph(anchor);
		
		AtomGroup *next = new AtomGroup();
		for (size_t i = 0; i < sequence->blockCount(); i++)
		{
			Atom *a = sequence->atomForBlock(i); 
			if (a)
			{
				*next += a;
				total -= a;
			}
		}
		
		if (next->size() == 0)
		{
			delete sequence;
			break;
		}
		
		delete sequence;
		groups.push_back(next);
	}

	_subgroups = groups;
	return _subgroups;
}

