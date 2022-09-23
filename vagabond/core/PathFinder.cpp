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

#include "PathFinder.h"
#include "Molecule.h"
#include "AtomGroup.h"
#include <algorithm>
#include "Route.h"

PathFinder::PathFinder(Molecule *start, Cluster<MetadataGroup> *cluster, int dims)
: StructureModification(start, 1, dims)
{
	_start = start;
	_threads = 8;
	_cluster = cluster;
	_pType = BondCalculator::PipelineForceField;
	AtomGroup *grp = _molecule->currentAtoms();
	_fullAtoms = grp;
	_fullAtoms->assignForceField(nullptr);
	startCalculator();
}

void PathFinder::setTarget(Molecule *end)
{
	_end = end;
	int idx = _cluster->dataGroup()->indexOfObject(end);
	int start = _cluster->dataGroup()->indexOfObject(_start);
	_aim = Placement(_dims, 0);

	for (size_t i = 0; i < _cluster->rows() && i < _dims; i++)
	{
		_aim[i] = _cluster->value(idx, i);
		_aim[i] -= _cluster->value(start, i);
	}

	Node m; m.vec = _aim;
}

void PathFinder::setup()
{
	for (BondCalculator *calc : _calculators)
	{
		Job job{};
		job.custom.allocate_vectors(1, _dims, _num);
		for (size_t i = 0; i < _dims; i++)
		{
			job.custom.vecs[0].mean[i] = _aim[i];
		}

		job.requests = static_cast<JobType>(JobExtractPositions);
		calc->submitJob(job);
	}

	for (BondCalculator *calc : _calculators)
	{
		Result *r = calc->acquireResult();
		if (r)
		{
			r->transplantPositions(); 
			delete r;
		}
	}
	
	for (size_t i = 0; i < _fullAtoms->size(); i++)
	{
		glm::vec3 pos = (*_fullAtoms)[i]->derivedPosition();
		(*_fullAtoms)[i]->setOtherPosition("alt", pos);
	}
	
	Placement start = Placement(_dims, 0);
	submitJob(start);
	collectResults();
}

int PathFinder::submitJob(Placement &n)
{
	assert(n.size() == _dims);
	for (BondCalculator *calc : _calculators)
	{
		if (!(calc->pipelineType() & BondCalculator::PipelineForceField))
		{
			continue;
		}

		Job job{};
		job.custom.allocate_vectors(1, _dims, _num);
		for (size_t i = 0; i < _dims; i++)
		{
			job.custom.vecs[0].mean[i] = n[i];
		}

		job.requests = static_cast<JobType>(JobExtractPositions | 
		                                    JobScoreStructure);
		int job_num = calc->submitJob(job);
		_scoreMap[job_num] = _nodes.size();
	}

	Node node{};

	node.vec = n;
	findDistanceToAim(node);
	_nodes.push_back(node);
	return _nodes.size() - 1;
}

void PathFinder::customModifications(BondCalculator *calc, bool has_mol)
{
	if (!has_mol)
	{
		return;
	}

	calc->setPipelineType(_pType);
	FFProperties props;
	props.group = _molecule->currentAtoms();
	props.t = FFProperties::CAlphaSeparation;
	calc->setForceFieldProperties(props);
}

void PathFinder::collectResults()
{
	int count = 0;
	bool more = true;
	while (more)
	{
		more = false;
		for (BondCalculator *calc : _calculators)
		{
			Result *r = calc->acquireResult();

			if (r == nullptr)
			{
				continue;
			}

			more = true;

			int num = r->ticket;
			if (r->requests & JobScoreStructure)
			{
				if (_scoreMap.count(num))
				{
					int idx = _scoreMap[num];
					_nodes[idx].score += r->score;
				}
			}
			
			r->destroy();
			
			count++;
		}
	}

	if (_lastCount == 0)
	{
		std::cout << "Starting node score: " << _nodes[0].score << std::endl;
	}
	_lastCount = _nodes.size();
}

bool PathFinder::testDirection(Placement dir, int idx)
{
	if (_end == nullptr)
	{
		return true;
	}

	Node copy = _nodes[idx];
	findDistanceToAim(copy);
	float before = copy.distance;

	copy.add_to(dir);
	copy.vec = dir;
	findDistanceToAim(copy);
	float after = copy.distance;
	
	return (after < before);
}

PathFinder::Placement PathFinder::generateDirection(int idx)
{
	Placement dir(_dims, 0);

	for (size_t i = 0; i < _dimsInUse; i++)
	{
		float r = rand() / (double)RAND_MAX;
		dir[i] = r * 2. - 1.;
		dir[i] *= _scale / sqrt(_dimsInUse);
		
		if (i == _self)
		{
			dir[i] *= _toGo;
		}
	}
	
	return dir;
}

void PathFinder::submitNode(Placement &dir, int idx)
{
	Placement old = dir;
	_nodes[idx].add_to(dir);
	int ndx = submitJob(dir);
	
	_nodes[ndx].last_dir = old;
	_nodes[ndx].parent = idx;
	_nodes[idx].next.push_back(ndx);
}

void PathFinder::extendNode(int idx)
{
	Placement dir;
	int num = _heads;
	dir = generateDirection(idx);
	if (idx == 0)
	{
		num = 10;
	}
	else
	{
	}

	for (size_t i = 0; i < num - 1; i++)
	{
		bool next = true;
		int count = 0;
		
		while (next && count < 100)
		{
			dir = generateDirection(idx);
			next = (!testDirection(dir, idx));
		}

		submitNode(dir, idx);
	}
}

bool PathFinder::doJobs()
{
	submitNextJobs();
	collectResults();
	triggerResponse();
	bool finish = identifyNextLeads();
	_cycles--;
	return finish;
}

struct Candidate
{
	float score;
	int idx;
	
	const bool operator<(const Candidate &b) const
	{
		return score < b.score;
	}
	
	const bool operator>(const Candidate &b) const
	{
		return score > b.score;
	}
};

void PathFinder::addFromAllNodes()
{
	std::vector<Candidate> tmp;

	for (size_t i = 0; i < _nodes.size(); i++)
	{
		Candidate c{};
		Node &n = _nodes[i];
//		if (n.score > _nodes[0].score + 0.5)
		if (n.score > 0.5)
		{
			continue;
		}
		
		if (n.next.size() > 0)
		{
			continue;
		}
		
		c.score = n.distance;
		c.idx = i;
		tmp.push_back(c);
	}

	std::sort(tmp.begin(), tmp.end(), std::less<Candidate>());

	for (size_t i = 0; i < tmp.size() && _list.size() < 24; i++)
	{
		if (std::find(_list.begin(), _list.end(), tmp[i].idx) != _list.end())
		{
			continue;
		}

		_list.push_back(tmp[i].idx);
	}
}

bool PathFinder::identifyNextLeads()
{
	std::vector<Candidate> tmp;

	_list.clear();

	for (std::map<int, int>::iterator it = _scoreMap.begin(); 
	     it != _scoreMap.end(); it++)
	{
		Candidate c{};
		Node &n = _nodes[it->second];
		findDistanceToAim(n);
		float energy = n.score;
		if (energy > 0.5)
		{
			continue;
		}

		c.idx = it->second;
		c.score = n.distance;
		tmp.push_back(c);
	}
	
	std::sort(tmp.begin(), tmp.end(), std::less<Candidate>());
	if (_nodes.size() >= 100 && false)
	{
		_arrivals.push_back(99);
		sendResponse("find_route", nullptr);
		return true;
	}

	for (size_t i = 0; i < tmp.size(); i++)
	{
		Node &n = _nodes[tmp[i].idx];
		findDistanceToAim(n);
		n.distance /= _nodes[i].distance;
		tmp[i].score = n.score;

		if (n.distance < _toGo)
		{
			_toGo = n.distance;
			std::cout << "New best: " << _toGo *100 << "% to go" << std::endl;
			_candidate = tmp[i].idx;
			if (_cycles < 100)
			{
				_cycles = 100;
			}

			if (_toGo < 0.15)
			{
				switchToAngleSpace();
			}
			
			if (_toGo < 0.01)
			{
				_arrivals.push_back(tmp[i].idx);
				sendResponse("find_route", nullptr);
				return true;
			}
		}
	}
	
//	std::sort(tmp.begin(), tmp.end(), std::less<Candidate>());
	
	for (size_t i = 0; i < tmp.size() && i < 12; i++)
	{
		_list.push_back(tmp[i].idx);
	}
	
	addFromAllNodes();

	_scoreMap.clear();
	return false;
}

void PathFinder::submitNextJobs()
{
	for (size_t i = 0; i < _list.size(); i++)
	{
		int idx = _list[i];
		extendNode(idx);
	}
}

void PathFinder::start()
{
	while (_cycles > 0)
	{
		bool done = doJobs();
		
		if (done)
		{
			break;
		}
	}
	_arrivals.push_back(_candidate);
	sendResponse("find_route", nullptr);
	std::cout << "done" << std::endl;
	
	traceDone();
}

void PathFinder::traceDone()
{
	for (size_t i = 0; i < _arrivals.size(); i++)
	{
	}

}

void PathFinder::addAxis(std::vector<ResidueTorsion> &list, 
                         std::vector<float> &values, bool split_big)
{
	if (split_big)
	{
		for (size_t i = 0; i < values.size(); i++)
		{
			float s = sin(deg2rad(values[i]));
			float c = cos(deg2rad(values[i]));
			_goalTrig.push_back(std::pair<float, float>(s, c));
			_goalAngles.push_back(values[i]);
		}

		_mask = std::vector<int>(values.size(), 1);
		for (size_t i = 0; i < list.size(); i++)
		{
			if (!Atom::isMainChain(list[i].torsion.atomName(0)) ||
			    !Atom::isMainChain(list[i].torsion.atomName(3)))
			{
				continue;
			}

			if (fabs(values[i]) > 90.f)
			{
				std::vector<float> tmp = std::vector<float>(list.size(), 0);
				tmp[i] = values[i];
				_mask[i] = 0;

				supplyTorsions(list, tmp);
				_dimsInUse++;
			}
		}
	}
	
	for (size_t i = 0; i < list.size(); i++)
	{
		if (i < _mask.size())
		{
			values[i] *= _mask[i];
		}
	}

	supplyTorsions(list, values);
	_self = _dimsInUse;
	_dimsInUse++;
}

std::vector<float> PathFinder::mapNodeToRope(int i)
{
	return mapNodeToRope(_nodes[i]);
}

std::vector<float> PathFinder::nodeToTorsionList(Node &n)
{
	int l = _cluster->dataGroup()->length();
	Placement pos(l, 0);

	for (size_t i = 0; i < _torsionLists.size(); i++)
	{
		const std::vector<float> &vals = _torsionLists[i].values;
		
		for (size_t j = 0; j < l; j++)
		{
			pos[j] += vals[j] * n.vec[i];
		}
	}

	return pos;
}

std::vector<float> PathFinder::mapNodeToRope(Node &n)
{
	Placement pos = nodeToTorsionList(n);
	Placement place = _cluster->mapVector(pos);
	
	return place;
}

void PathFinder::switchToAngleSpace()
{
	if (!_ropeSpace)
	{
		return;
	}

	_ropeSpace = false;
	for (Node &n : _nodes)
	{
		findDistanceToAim(n);
	}
	std::cout << "Switching to angle space" << std::endl;
	_cycles += 500;
	
	_toGo = 1.;
}

void PathFinder::findDistanceToAim(Node &n)
{
	if (_ropeSpace)
	{
		findDistanceRopeSpace(n);
	}
	else
	{
		findDistanceAngleSpace(n);
	}
}

void PathFinder::findDistanceAngleSpace(Node &n)
{
	Node m;
	m.vec = _aim;
	Placement torsion_space = nodeToTorsionList(n);

	float distance = 0;
	for (size_t i = 0; i < _goalTrig.size(); i++)
	{
		float s = _goalTrig[i].first;
		float c = _goalTrig[i].second;

		float ns = sin(deg2rad(torsion_space[i]));
		float nc = cos(deg2rad(torsion_space[i]));
		float add = (s - ns) * (s - ns) + (c - nc) * (c - nc);
		add *= fabs(_goalAngles[i]);
		distance += add;
	}

	distance = sqrt(distance);
	n.distance = distance;
}

void PathFinder::findDistanceRopeSpace(Node &n)
{
	Placement rope_space = mapNodeToRope(n);
	float distance = 0;
	
	for (size_t i = 0; i < rope_space.size(); i++)
	{
		float add = rope_space[i] - _aim[i];
		distance += add * add;
	}

	distance = sqrt(distance);
	n.distance = distance;
}

Route *PathFinder::route(int idx)
{
	int end = _arrivals[idx];
	int next_node = end;

	std::vector<int> list;

	while (next_node >= 0)
	{
		list.push_back(next_node);
		next_node = _nodes[next_node].parent;
	}
	
	int l = _cluster->dataGroup()->length();
	Route *route = new Route(_molecule, _cluster, l);

	for (int i = (int)list.size() - 1; i >= 0; i--)
	{
//		Placement rope_space = mapNodeToRope(_nodes[list[i]]);
		Placement torsion_space = nodeToTorsionList(_nodes[list[i]]);
		route->addPoint(torsion_space);
	}

	return route;
}
