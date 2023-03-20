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
#include "AlignmentTool.h"
#include "BondLength.h"
#include "Grapher.h"
#include "../utils/FileReader.h"
#include "BondTorsion.h"
#include "engine/Mechanics.h"
#include "TorsionBasis.h"
#include "BondCalculator.h"
#include "PositionRefinery.h"
#include "Sequence.h"
#include "PdbFile.h"

#include <algorithm>
#include <iostream>
#include <thread>

AtomGroup::AtomGroup() : HasBondstraints()
{

}

void AtomGroup::writeToFile(std::string name)
{
	PdbFile::writeAtoms(this, name);
}

void AtomGroup::alignAnchor()
{
	try
	{
		AlignmentTool tool(this);
		tool.run();
		recalculate();
	}
	catch (const std::runtime_error &err)
	{
		std::cout << "Giving up: " << err.what() << std::endl;
	}
}

void AtomGroup::cancelRefinement()
{
	if (_refinery != nullptr)
	{
		_refinery->finish();
	}
}

void AtomGroup::cleanupRefinement()
{
	if (_refinery != nullptr)
	{
		delete _refinery;
		_refinery = nullptr;
	}
	
	if (_refine != nullptr)
	{
		_refine->join();
		delete _refine;
		_refine = nullptr;
	}
}

void AtomGroup::deleteConnectedGroups()
{
	for (size_t i = 0; i < _connectedGroups.size(); i++)
	{
		delete _connectedGroups[i];
	}

	_connectedGroups.clear();
}

AtomGroup::~AtomGroup()
{
	cancelRefinement();
	deleteBondstraints();
	deleteConnectedGroups();
	
	AtomVector().swap(_atoms);
}

bool AtomGroup::hasAtom(Atom *a)
{
	if (_orderedPointers.count(a))
	{
		return true;
	}

	return (_desc2Atom.count(a->desc()) > 0);

	AtomVector::iterator it = std::find(_atoms.begin(), _atoms.end(), a);

	return (it != _atoms.end());
}

void AtomGroup::add(const AtomVector &av)
{
	for (Atom *a : av)
	{
		add(a);
	}
}

void AtomGroup::add(Atom *a)
{
	if (a != nullptr && !hasAtom(a))
	{
		_atoms.push_back(a);
		addBondstraintsFrom(a);
		
		_desc2Atom[a->desc()] = a;
		_orderedPointers.insert(a);

		_anchors.clear();
	}
}

void AtomGroup::operator+=(Atom *a)
{
	add(a);
}

void AtomGroup::operator-=(Atom *a)
{
	AtomVector::iterator it = std::find(_atoms.begin(), _atoms.end(), a);

	if (it != _atoms.end())
	{
		_atoms.erase(it);
		
		if (_desc2Atom.at(a->desc()) == a)
		{
			_desc2Atom.erase(a->desc());
		}

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
	
	if (_anchors.size() == 0)
	{
		return nullptr;
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
	AtomVector tooFewConnections;
	_anchors.clear();
	
	if (_atoms.size() == 1)
	{
		_anchors.push_back(_atoms[0]);
		return;
	}

	for (size_t i = 0; i < trials.size(); i++)
	{
		/* we don't want to start on a hydrogen */
		if (trials[i]->elementSymbol() == "H")
		{
			tooFewConnections.push_back(trials[i]);
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

	if (_anchors.size() == 0)
	{
		_anchors = tooFewConnections;
	}
}

Atom *AtomGroup::atomByDesc(std::string desc) const
{
	for (size_t i = 0; i < _atoms.size(); i++)
	{
		if (_atoms[i]->desc() == desc)
		{
			return _atoms[i];
		}
	}
	
	return nullptr;

}

Atom *AtomGroup::atomByIdName(const ResidueId &id, std::string name,
                              std::string chain) const
{
	to_upper(name);
	for (size_t i = 0; i < _atoms.size(); i++)
	{
		if (chain.length() > 0 && _atoms[i]->chain() != chain)
		{
			continue;
		}

		if (_atoms[i]->residueId() == id && name == "")
		{
			return _atoms[i];
		}

		if (_atoms[i]->residueId() == id && _atoms[i]->atomName() == name)
		{
			return _atoms[i];
		}
	}
	
	return nullptr;
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

Atom *AtomGroup::chosenAnchor(bool min)
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
	
	int found_res = (min ? INT_MAX : -INT_MAX);
	
	for (size_t i = 0; i < possibleAnchorCount(); i++)
	{
		int res = possibleAnchor(i)->residueNumber();
		if ((min && (res < found_res)) || (!min && (res > found_res)))
		{
			_chosenAnchor = possibleAnchor(i);
			found_res = res;
		}
	}

	return _chosenAnchor;
}

void AtomGroup::recalculate()
{
	try
	{
		connectedGroups();

		for (size_t i = 0; i < _connectedGroups.size(); i++)
		{
			Atom *anchor = _connectedGroups[i]->chosenAnchor();

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
	catch (const std::runtime_error &err)
	{
		std::cout << "Giving up: " << err.what() << std::endl;
	}
}

void AtomGroup::refinePositions(bool sameThread, bool thorough)
{
	if (_refinery && !_refinery->isDone())
	{
		return;
	}

	PositionRefinery *refinery = new PositionRefinery(this);
	refinery->setThorough(true);

	cancelRefinement();
	cleanupRefinement();

	if (sameThread)
	{
		refinery->refine();
	}
	else
	{
		_refinery = refinery;
		_refine = new std::thread(&PositionRefinery::backgroundRefine, refinery);
	}
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

std::vector<AtomGroup *> &AtomGroup::connectedGroups(bool forSequence)
{
	if (_connectedGroups.size() && _forSequence == forSequence)
	{
		return _connectedGroups;
	}

	_forSequence = forSequence;
	deleteConnectedGroups();

	std::vector<AtomGroup *> groups;
	AtomGroup total = AtomGroup(*this);
	
	while (total.size() > 0)
	{
		Grapher grapher;
		Atom *anchor = total.possibleAnchor(0);
		
		if (anchor == nullptr)
		{
			continue;
		}

		AnchorExtension ext{anchor, UINT_MAX};
		
		if (forSequence)
		{
			grapher.setInSequence(true);
		}

		grapher.generateGraphs(ext);
		
		AtomGroup *next = new AtomGroup();
		for (size_t i = 0; i < grapher.graphCount(); i++)
		{
			Atom *a = grapher.graph(i)->atom;
			if (a)
			{
				if (!total.hasAtom(a))
				{
					continue;
				}

				*next += a;
				total -= a;
			}
		}
		
		if (next->size() == 0)
		{
			break;
		}
		
		groups.push_back(next);
		_anchor2Group[anchor] = next;
	}

	_connectedGroups = groups;
	return _connectedGroups;
}

void AtomGroup::getLimitingResidues(int *min, int *max)
{
	*min = INT_MAX;
	*max = -INT_MAX;
	
	for (size_t i = 0; i < size(); i++)
	{
		if (atom(i)->hetatm())
		{
			continue;
		}

		int num = atom(i)->residueNumber();
		*min = std::min(*min, num);
		*max = std::max(*max, num);
	}
	
	if (size() == 0)
	{
		*min = 0;
		*max = 0;
	}
}

Sequence *AtomGroup::sequence()
{
	int min, max;
	getLimitingResidues(&min, &max);
	
	Atom *best = nullptr;
	int lowest = INT_MAX;
	
	for (size_t i = 0; i < possibleAnchorCount(); i++)
	{
		int res = possibleAnchor(i)->residueNumber();

		if (res == min) // shortcut to victory
		{
			best = possibleAnchor(i);
			break;
		}
		
		if (res < lowest)
		{
			lowest = res;
			best = possibleAnchor(i);
		}
	}

	Sequence *sequence = new Sequence(best);
	return sequence;
}

void AtomGroup::mechanics()
{
	cancelRefinement();
	cleanupRefinement();

	Mechanics *m = new Mechanics(this);
	_mech = m;

	_mechThread = new std::thread(&Mechanics::run, _mech);
}

void AtomGroup::addTransformedAnchor(Atom *a, glm::mat4x4 transform)
{
	a->setTransformation(transform);
	_transformedAnchors.push_back(a);

}

float AtomGroup::rmsd() const
{
	float sum = 0;
	float weights = 0;
	for (size_t i = 0; i < size(); i++)
	{
		Atom *a = atom(i);
		if (a->elementSymbol() == "H")
		{
			continue;
		}

		glm::vec3 diff = a->initialPosition() - a->derivedPosition();
		float sqlength = glm::dot(diff, diff);
		sum += sqlength;
		weights++;
	}

	sum = sqrt(sum / weights);
	return sum;
}

glm::vec3 AtomGroup::initialCentre()
{
	glm::vec3 sum = glm::vec3(0.f);
	double weight = 0;
	for (Atom *a : _atoms)
	{
		sum += a->initialPosition();
		weight++;
	}

	sum /= weight;
	return sum;
}

void AtomGroup::finishedRefining()
{
	triggerResponse();
}

AtomGroup AtomGroup::extractFragment(Sequence frag)
{
	AtomGroup select;
	
	for (Atom *a : _atoms)
	{
		for (const Residue &r : frag.residues())
		{
			if (a->residueId() == r.id())
			{
				select += a;
			}
		}
	}

	return select;
}
