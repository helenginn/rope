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

#ifndef __vagabond__PathFinder__
#define __vagabond__PathFinder__

#include <atomic>
#include "Responder.h"
#include "StructureModification.h"
#include <vagabond/core/MetadataGroup.h>
#include <vagabond/c4x/Cluster.h>

class Route;
class Molecule;

class PathFinder : public StructureModification,
public HasResponder<Responder<PathFinder>>
{
public:
	PathFinder(Molecule *start, Cluster<MetadataGroup> *cluster, int dims);

	void setTarget(Molecule *end);
	
	void setup();
	void start();

	size_t nodeCount()
	{
		return _lastCount;
	}
	
	const int parentIndex(int i)
	{
		return _nodes[i].parent;
	}
	
	const float score(int i)
	{
		return _nodes[i].score;
	}

	const std::vector<float> &placement(int i) const
	{
		return _nodes[i].vec;
	}
	
	Molecule *begin()
	{
		return _start;
	}
	
	Molecule *end()
	{
		return _end;
	}
	
	std::vector<float> mapNodeToRope(int i);

	void addAxis(std::vector<ResidueTorsion> &list, 
	             std::vector<float> &values, bool split_big = false);
	
	size_t routeCount()
	{
		return _arrivals.size();
	}

	Route *route(int i);
protected:
	typedef std::vector<float> Placement;
	int submitJob(Placement &n);
	void collectResults();

	virtual void customModifications(BondCalculator *calc, bool has_mol);
private:
	Molecule *_start = nullptr;
	Molecule *_end = nullptr;
	
	struct Node
	{
		Placement vec; /**< vector in RoPE space */
		Placement last_dir;
		float score = 0;
		float distance = 0;

		int parent = -1;
		std::vector<int> next{};
		
		void add_to(Placement &dir)
		{
			for (size_t i = 0; i < dir.size(); i++)
			{
				dir[i] += vec[i];
			}
		}
	};

	std::vector<float> mapNodeToRope(Node &n);
	std::vector<float> nodeToTorsionList(Node &n);

	std::map<int, int> _scoreMap;
	std::vector<Node> _nodes;
	std::vector<int> _list;
	void findDistanceToAim(Node &n);
	void findDistanceAngleSpace(Node &n);
	void findDistanceRopeSpace(Node &n);
	void switchToAngleSpace();

	Placement generateDirection(int idx);
	bool testDirection(Placement dir, int idx);
	void addFromAllNodes();
	void traceDone();

	bool doJobs();
	bool identifyNextLeads();
	void submitNextJobs();
	void submitNode(Placement &dir, int idx);
	void extendNode(int idx);
	
	std::atomic<int> _lastCount{0};
	Placement _aim;
	Placement _goalAngles;
	std::vector<int> _mask;
	std::vector<std::pair<float, float> > _goalTrig;
	float _scale = 0.01;
	int _heads = 4;
	float _toGo = FLT_MAX;
	int _cycles = 100;
	int _dimsInUse = 0;
	int _self = 0;
	bool _ropeSpace = true;
	
	std::vector<int> _arrivals;
	int _candidate = 0;
};

#endif
