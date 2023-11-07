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
#include "BondCalculator.h"
#include "CompareDistances.h"
#include "BondSequence.h"
#include "engine/Tasks.h"
#include "engine/Task.h"
#include "TorsionWarp.h"
#include "AtomGroup.h"
#include "Instance.h"
#include <fstream>
#include "Warp.h"

Warp::Warp(Instance *inst, size_t num_axes) 
{
	setInstance(inst);
	_dims = num_axes;
	_threads = 6;
}

void Warp::addTorsionsToJob(Job *job)
{
	job->fetchTorsion = [this](const Coord::Get &get, const int &idx)
	{
		float t = _torsion_angles_for_coord(get, idx);
		return t;
	};
}

int Warp::submitJob(bool show, const std::vector<float> &vals)
{
	_ticket++;

	BaseTask *first_hook = nullptr;
	CalcTask *final_hook = nullptr;
	
	/* get easy references to resources */
	BondCalculator *const &calculator = _resources.calculator;
	BondSequenceHandler *const &sequences = _resources.sequences;

	/* this final task returns the result to the pool to collect later */
	Task<Result, void *> *submit_result = calculator->submitResult(_ticket);

	Flag::Calc calc = Flag::Calc(Flag::DoTorsions | Flag::DoPositions
	                             | Flag::DoSuperpose);
	
	Flag::Extract extract = Flag::Extract(Flag::AtomVector);
	
	Task<BondSequence *, AtomPosList *> *list = nullptr;

	/* calculation of torsion angle-derived and target-derived
	 * atom positions */
	sequences->calculate(calc, vals, &first_hook, &final_hook);
	sequences->extract_compare_distances(submit_result, _compare, final_hook);
	
	_resources.tasks->addTask(first_hook);

	_ticket2Point[_ticket] = _jobNum;
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
		_compare->reset();
		_resources.calculator->holdHorses();

		for (const Floats &fs : points)
		{
			submitJob(true, fs);
		}
		_resources.calculator->releaseHorses();
		
		retrieve();

		exposeDistanceMatrix();
		_alwaysShow = true;
		clearTickets();

		return compare()->quickScore();
	};
	
	return func;
}

void Warp::setup()
{
	_base = {};
	_filter = [](Atom *const &atom)
	{
		return (atom->isReporterAtom());
	};
	
	_displayTargets = true;

	prepareResources();
	prepareAtoms();
	prepareBonds();
}

void Warp::prepareAtoms()
{
	BondSequence *seq = _resources.sequences->sequence();

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
	BondSequence *seq = _resources.sequences->sequence();

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

void Warp::saveSpace(const std::string &filename)
{
	if (filename == "rope.json")
	{
		throw std::runtime_error("rope.json is a reserved filename");
	}

	json j = *torsionWarp();

	std::ofstream file;
	file.open(filename);
	file << j.dump(1);
	file << std::endl;
	file.close();

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

const std::vector<Parameter *> &Warp::parameterList() const
{
	return _resources.sequences->torsionBasis()->parameters();
}

void Warp::prepareResources()
{
	_resources.allocateMinimum(_threads);

	AtomGroup *group = _instance->currentAtoms();

	std::vector<AtomGroup *> subsets = group->connectedGroups();
	for (AtomGroup *subset : subsets)
	{
		Atom *anchor = subset->chosenAnchor();
		_resources.sequences->addAnchorExtension(anchor);
	}

	_resources.sequences->setIgnoreHydrogens(true);
	_resources.sequences->prepareToSkipSections(true);
	_resources.sequences->setup();
	_resources.sequences->prepareSequences();

	const std::vector<AtomBlock> &blocks = 
	_resources.sequences->sequence()->blocks();

}

void Warp::setAtomMotions(std::function<glm::vec3(const Coord::Get &,
                                                  int num)> &func)
{
	_atom_positions_for_coord = func;

	auto fetch = [this](const Coord::Get &get, const int &idx)
	{
		glm::vec3 p = _atom_positions_for_coord(get, idx);
		p += _base.positions[idx];
		return p;
	};
	
	CoordManager *manager = _resources.sequences->manager();
	manager->setAtomFetcher(fetch);
}

void Warp::setBondMotions(std::function<float(const Coord::Get &, int num)> &func)
{
	_torsion_angles_for_coord = func;

	auto fetch = [this](const Coord::Get &get, const int &idx)
	{
		float t = _torsion_angles_for_coord(get, idx);
		return t;
	};

	CoordManager *manager = _resources.sequences->manager();
	manager->setTorsionFetcher(fetch);
}
