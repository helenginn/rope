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

#include <vagabond/utils/FileReader.h>
#include "CompareDistances.h"
#include "BondSequence.h"
#include "TorsionWarp.h"
#include "AtomGroup.h"
#include "Instance.h"
#include "Warp.h"

Warp::Warp(Instance *ref, size_t num_axes) 
: StructureModification(ref)
{
	_dims = num_axes;
	_threads = 6;
}

int Warp::submitJob(bool show, const std::vector<float> &vals)
{
	for (BondCalculator *calc : _calculators)
	{
		Job job{};
		job.parameters = vals;
		job.pos_sampler = this;
		job.atomTargets = [this](const Coord::Get &get, const int &idx)
		{
			glm::vec3 p = _atom_positions_for_coord(get, idx);
			p += _base.positions[idx];
			return p;
		};

		job.requests = static_cast<JobType>(JobPositionVector |
		                                    JobCalculateDeviations);
		if (!show)
		{
			job.requests = JobCalculateDeviations;
		}

		int t = calc->submitJob(job);
		_ticket2Point[t] = _jobNum;
	}

	_point2Score[_jobNum] = Score{};
	_jobNum++;
	return _jobNum - 1;
}

std::function<float()> Warp::score(const std::vector<Floats> &points)
{
	std::function<float()> func;

	func = [points, this]() -> float
	{
		_alwaysShow = false;
		clearComparison();
		bool show = (_jobNum % 11 == 0);
		
		for (const Floats &fs : points)
		{
			submitJob(true, fs);
		}
		
		retrieve();

		float sum = 0; float count = 0;
		for (TicketScores::iterator it = _point2Score.begin();
		     it != _point2Score.end(); it++)
		{
			sum += it->second.deviations;
			count++;
		}
		sum /= count;

		exposeDistanceMatrix();
		_alwaysShow = true;
		clearTickets();

//		return sum;
		return compare()->quickScore();
	};
	
	return func;
}

void Warp::setup()
{
	_base = {};
	startCalculator();
	prepareAtoms();
	prepareBonds();
	
	_filter = [](Atom *const &atom)
	{
		return (atom->isReporterAtom());
	};
	
	_displayTargets = true;
}

glm::vec3 Warp::prewarnAtom(BondSequence *bc, const Coord::Get &get, int index)
{
	glm::vec3 p = _atom_positions_for_coord(get, index);
	p += _base.positions[index];
	return p;
}

bool Warp::prewarnAtoms(BondSequence *bc, const Coord::Get &get, Vec3s &ps)
{
	ps.clear();
	return true;
}


float Warp::prewarnBond(BondSequence *bc, const Coord::Get &get, int index)
{
	float t = _torsion_angles_for_coord(get, index);
	t += _base.torsions[index];
	return t;
}

void Warp::prewarnBonds(BondSequence *seq, const Coord::Get &get, Floats &ts)
{
	// set ts to the torsion angles for this get
	ts.clear();
	return;
}

void Warp::prepareAtoms()
{
	BondCalculator *calc = _instanceToCalculator[_instance];
	BondSequence *seq = calc->sequence();

	const std::vector<AtomBlock> &blocks = seq->blocks();

	for (int i = 0; i < blocks.size(); i++)
	{
		Atom *search = blocks[i].atom;
		_base.atoms.push_back(search);

		if (!search)
		{
			_base.positions.push_back(glm::vec3(NAN));
			continue;
		}

		glm::vec3 pos = search->derivedPosition();
		_base.positions.push_back(pos);
	}
	
	_atom_positions_for_coord = [](const Coord::Get &get, int num)
	{
		return glm::vec3{};
	};
}	

void Warp::prepareBonds()
{
	BondCalculator *calc = _instanceToCalculator[_instance];
	BondSequence *seq = calc->sequence();

	TorsionBasis *basis = seq->torsionBasis();
	_base.torsions.resize(basis->parameterCount());
	int i = 0;
	for (float &f : _base.torsions)
	{
		_base.parameters.push_back(basis->parameter(i));
		f = basis->parameter(i)->value();
		i++;
	}
	
	Floats &torsions = _base.torsions;
	_torsion_angles_for_coord = [torsions](const Coord::Get &get, int num)
	{
		return torsions[num];
	};
}

void Warp::exposeDistanceMatrix()
{
	if (compare()->hasMatrix() && _showMatrix)
	{
		freeMatrix(&_distances);
		_distances = compare()->matrix();
		sendResponse("atom_matrix", &_distances);
	}
}

void Warp::cleanup()
{
	sendResponse("cleanup", nullptr);
}

bool Warp::handleAtomList(AtomPosList &list)
{

	bool show = (_count++ % 107 == 0) || _alwaysShow;

	// handle list;
	compare()->process(list);

	return show;
}

CompareDistances *Warp::compare()
{
	if (!_compare)
	{
		_compare = new CompareDistances();
		compare()->setFiltersEqual(_filter);
	}

	return _compare;
}

void Warp::customModifications(BondCalculator *calc, bool has_mol)
{
	calc->setSuperpose(true);
	calc->prepareToSkipSections(true);
	calc->manager().setDefaultCoordTransform(JobManager::identityTransform());
}

void Warp::resetComparison()
{
	compare()->reset();
}

void Warp::clearComparison()
{
	compare()->clearMatrix();
}

void Warp::clearFilters()
{
	compare()->setFiltersEqual(_filter);
}

void loadJson(const std::string &filename, TorsionWarp *tw)
{
	if (!file_exists(filename))
	{
		return;
	}
	json data;
	std::ifstream f;
	f.open(filename);
	f >> data;
	f.close();
	
	tw->coefficientsFromJson(data);
}

Warp *Warp::warpFromFile(Instance *reference, std::string file)
{
	const int target_dims = 2;
	const int coeffs = 1;

	Warp *warp = new Warp(reference, target_dims);
	warp->setup();
	
	TorsionWarp *tw = new TorsionWarp(warp->parameterList(), target_dims, coeffs);
	
	std::function<float(const Coord::Get &, int num)> func;
	func = [tw](const Coord::Get &get, int num)
	{
		return tw->torsion(get, num);
	};
	
	warp->setBondMotions(func);
	warp->setTorsionWarp(tw);

	loadJson(file, tw);
	return warp;
}
