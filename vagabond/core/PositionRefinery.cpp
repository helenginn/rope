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

#include "PositionRefinery.h"
#include "AtomGroup.h"
#include "BondCalculator.h"
#include "BondSequenceHandler.h"
#include "BondSequence.h"
#include "TorsionBasis.h"

PositionRefinery::PositionRefinery(AtomGroup *group) : SimplexEngine()
{
	_group = group;
}

PositionRefinery::~PositionRefinery()
{

}

void PositionRefinery::refine()
{
	if (_group == nullptr)
	{
		throw std::runtime_error("Attempting to refine group, but no "
		                         "group specified.");
	}
	
	std::cout << "Refining" << std::endl;
	
	std::vector<AtomGroup *> units = _group->connectedGroups();

	for (size_t i = 0; i < units.size(); i++)
	{
		if (units[i]->size() > 1)
		{
			try
			{
				refine(units[i]);
			}
			catch (const std::runtime_error &err)
			{
				std::cout << "Giving up: " << err.what() << std::endl;
			}
		}
	}
	
	if (_group->responder() != nullptr)
	{
		_group->responder()->finishedRefinement();
	}
	
	_done = true;
}

void PositionRefinery::calculateActiveTorsions()
{
	_nActive = 0;
	_mask = _calculator->sequenceHandler()->depthLimitMask();

	for (size_t i = 0; i < _mask.size(); i++)
	{
		if (_mask[i])
		{
			_nActive++;
		}
	}
}

bool PositionRefinery::refineBetween(int start, int end, int side_max)
{
	_start = start;
	_end = end;

	_calculator->setMinMaxDepth(_start, _end);
	_calculator->setMaxSideDepth(side_max);
	_calculator->start();

	calculateActiveTorsions();

	if (_nActive == 0)
	{
		_calculator->finish();
		return false;
	}

	setDimensionCount(_nActive);
	chooseStepSizes(_steps);
	setMaxJobsPerVertex(1);

	bool improved = run();

	if (improved)
	{
		const Point &trial = bestPoint();
		Point best = expandPoint(trial);
		TorsionBasis *basis = _calculator->sequenceHandler()->torsionBasis();
		basis->absorbVector(&best[0], best.size());
	}

	_calculator->finish();

	return true;
}

double PositionRefinery::fullResidual()
{
	_calculator->setMinMaxDepth(0, INT_MAX);
	_calculator->start();

	Job job{};
	job.requests = (JobType)(JobCalculateDeviations | JobExtractPositions);
	_calculator->submitJob(job);

	Result *result = _calculator->acquireResult();
	float dev = result->deviation;
	
	result->transplantPositions();

	_calculator->finish();
	result->destroy();

	return dev;
}

void PositionRefinery::fullRefinement(AtomGroup *group)
{
	refineBetween(0, _nBonds);
}

bool *PositionRefinery::generateAbsorptionMask(std::set<Atom *> done)
{
	TorsionBasis *basis = _calculator->sequenceHandler()->torsionBasis();
	
	bool *mask = new bool[_nActive];
	
	for (size_t i = 0; i < _nActive; i++)
	{
		mask[i] = done.count(basis->atom(i)) == 0;
	}
	
	size_t i = 0;
	for (i = 0; i < _nActive; i++)
	{
		if (!mask[i])
		{
			delete [] mask;
			return nullptr;
			for (size_t j = i; j < _nActive; j++)
			{
				mask[j] = false;
			}

			break;
		}
	}

	for (size_t i = 0; i < _nActive; i++)
	{
//		std::cout << mask[i];
	}
//	std::cout << std::endl;
	
	if (i == 0)
	{
		delete [] mask;
		return nullptr;
	}

	return mask;
}

void PositionRefinery::measureAtoms()
{
	const Point &trial = bestPoint();
	sendJob(trial, true);
	double empty = 0;
	awaitResult(&empty);

	const BondSequence *seq = _calculator->sequence();
	std::vector<Atom *> currAtoms = seq->addedAtoms();
	
	std::set<Atom *> check;
	for (Atom *atom : currAtoms)
	{
		check.insert(atom);
	}

	std::set<BondTorsion *> total;

	for (Atom *atom : currAtoms)
	{
		for (size_t i = 0; i < atom->bondTorsionCount(); i++)
		{
			BondTorsion *t = atom->bondTorsion(i);
			
			bool outside = false;
			for (size_t n = 0; n < 4; n++)
			{
				if (!check.count(t->atom(n)))
				{
					outside = true;
				}
			}
			
			if (outside == false)
			{
				total.insert(t);
			}
		}
	}
	
	for (BondTorsion *t : total)
	{
		float measured = t->measurement(BondTorsion::SourceDerived);
		t->setRefinedAngle(measured);
	}
}

void PositionRefinery::stepRefine()
{
	std::set<Atom *> done, added;
	Atom *first = _atomQueue.front();

	while (_atomQueue.size())
	{
		_calculator->reset();

		Atom *atom = _atomQueue.front();
		AnchorExtension ext = _atom2Ext[atom];

		_atomQueue.pop();
		
		if (done.count(atom) > 0)
		{
			continue;
		}

		std::cout << "Doing: " << atom->desc() << std::endl;
		_calculator->addAnchorExtension(ext);
		_calculator->setup();
		_calculator->start();

		calculateActiveTorsions();

		if (_nActive == 0)
		{
			done.insert(atom);
			_calculator->finish();
			continue;
		}

		bool *mask = generateAbsorptionMask(done);
		
		if (!mask)
		{
			done.insert(atom);
			continue;
		}

		TorsionBasis *basis = _calculator->sequenceHandler()->torsionBasis();
		
		BondTorsion *t = basis->torsion(0);
		double diff = fabs(t->refinedAngle() - ext.block.torsion);
		
		if (diff > 1e-2)
		{
			std::cout << "WARNING! " << atom->desc() << " ";
			std::cout << t->refinedAngle() << " " << ext.block.torsion << std::endl;
		}

		setDimensionCount(_nActive);
		setMaxJobsPerVertex(1);
		_steps = std::vector<float>(_nActive, _step);
		chooseStepSizes(_steps);

		bool improved = run();

		const Point &trial = bestPoint();
		basis->absorbVector(&trial[0], trial.size());
		sendJob(trial, true);
		
		done.insert(atom);

		const BondSequence *seq = _calculator->sequence();
		const Grapher &g = seq->grapher();

		for (size_t i = 0; i < seq->blockCount(); i++)
		{
			Atom *a = seq->atomForBlock(i);
			if (!a)
			{
				continue;
			}

			AtomGraph *next = g.graph(a);
			AnchorExtension ext = seq->getExtension(a);
			ext.count = 8;
			_atom2Ext[a] = ext;

			if (!next->torsion)
			{
				continue;
			}

			next->torsion->setRefinedAngle(ext.block.torsion);

			if (added.count(a))
			{
				continue;
			}

			std::cout << "Adding ";
			std::cout << a->desc() << "!" << std::endl;

			added.insert(a);
			_atomQueue.push(a);
		}

		/*
		for (size_t i = 0; i < graph->children.size(); i++)
		{
			AtomGraph *next = graph->children[i];
			
			if (!next->torsion)
			{
				continue;
			}

			Atom *atom = next->atom;

			AnchorExtension ext = seq->getExtension(atom);
			ext.count = 8;

			_atom2Ext[atom] = ext;
			if (added.count(atom))
			{
				continue;
			}
			added.insert(atom);
			_atomQueue.push(atom);
		}
		*/

		delete [] mask;
	}

	_calculator->reset();
	AnchorExtension ext(first);
	_calculator->addAnchorExtension(ext);
	_calculator->setup();
	_calculator->start();

}

void PositionRefinery::stepwiseRefinement(AtomGroup *group)
{
	std::chrono::high_resolution_clock::time_point tstart;
	tstart = std::chrono::high_resolution_clock::now();
	
	int nb = _calculator->sequence()->blockCount() + 1;

	if (false)
	{
		for (size_t i = 0; i < nb; i++)
		{
			refineBetween(i, i + _depthRange);

			if (_finish)
			{
				break;
			}
		}
	}
	
	stepRefine();

	std::chrono::high_resolution_clock::time_point tend;
	tend = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double, std::milli> time_span = tend - tstart;

//	std::cout << "Milliseconds taken: " <<  time_span.count() << std::endl;
	float seconds = time_span.count() / 1000.;
	float rate = (float)_ncalls / seconds;
	_ncalls = 0;
//	std::cout << "Calculations per second: " <<  rate << std::endl;
	
	_calculator->finish();

}

void PositionRefinery::testTransfer(AnchorExtension ext)
{
	_calculator = new BondCalculator();
	_calculator->setPipelineType(BondCalculator::PipelineAtomPositions);
	_calculator->setMaxSimultaneousThreads(1);
	_calculator->setTotalSamples(1);
	_calculator->setTorsionBasisType(_type);
	_calculator->addAnchorExtension(ext);
	_calculator->setIgnoreHydrogens(true);
	_calculator->setup();

	double res = fullResidual();
	std::cout << "Recalculate with new basis: " 
	<< res << " Angstroms over " << _group->size() << " atoms." << std::endl;

	delete _calculator;
	_calculator = nullptr;
}

void PositionRefinery::refine(AtomGroup *group)
{
	Atom *anchor = group->chosenAnchor();

	_calculator = new BondCalculator();
	_calculator->setPipelineType(BondCalculator::PipelineAtomPositions);
	_calculator->setMaxSimultaneousThreads(1);
	_calculator->setTotalSamples(1);
	_calculator->setTorsionBasisType(_type);
	_calculator->addAnchorExtension(anchor);
	_calculator->setIgnoreHydrogens(true);
	_calculator->setup();

	_nBonds = _calculator->maxCustomVectorSize();
	
	double res = fullResidual();
	AnchorExtension ext(anchor, 8);
	_atomQueue.push(anchor);
	_atom2Ext[anchor] = ext;
	
	if (res < 0.3)
	{
		_depthRange = 7.;
	}
	else if (res < 0.2)
	{
		_depthRange = 8.;
	}
	else if (res < 0.1)
	{
		_depthRange = 9.;
	}

	_steps = std::vector<float>(_nBonds, _step);
	
	stepwiseRefinement(group);
	
	res = fullResidual();
	std::cout << "Overall average distance after refinement: "
	<< res << " Angstroms over " << group->size() << " atoms." << std::endl;
	
	delete _calculator;
	_calculator = nullptr;
}

int PositionRefinery::awaitResult(double *eval)
{
	while (true)
	{
		Result *result = _calculator->acquireResult();
		if (result == nullptr)
		{
			return -1;
		}

		if (result->requests & JobExtractPositions)
		{
			result->transplantPositions();
		}
		if (result->requests & JobCalculateDeviations)
		{
			*eval = result->deviation;
		}

		int ticket = result->ticket;
		result->destroy();
		return ticket;
	}
}

PositionRefinery::Point PositionRefinery::expandPoint(const Point &p)
{
	Point expanded;
	expanded.reserve(_mask.size());
	int num = 0;

	for (size_t i = 0; i < _mask.size(); i++)
	{
		expanded.push_back(_mask[i] ? p[num] : 0);
		
		if (_mask[i])
		{
			num++;
		}
	}
	
	return expanded;
}

int PositionRefinery::sendJob(const Point &trial, bool force_update)
{
	Job job{};
	job.requests = JobCalculateDeviations;

	job.custom.allocate_vectors(1, _mask.size(), 0);

	Point expanded = expandPoint(trial);

	for (size_t i = 0; i < _mask.size(); i++)
	{
		job.custom.vecs[0].mean[i] = expanded[i];
	}

	if (_ncalls % 200 == 0 || force_update)
	{
		job.requests = static_cast<JobType>(JobCalculateDeviations | 
		                                    JobExtractPositions);
	}

	int ticket = _calculator->submitJob(job);
	_ncalls++;
	
	return ticket;
}

void PositionRefinery::finish()
{
	SimplexEngine::finish();
}
