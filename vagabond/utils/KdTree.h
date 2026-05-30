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

#ifndef __vagabond__KdTree__
#define __vagabond__KdTree__

#include "glm_import.h"

class KdTree
{
public:
	typedef std::function<bool(size_t &, glm::vec3 &)> NextIdxPos;

	KdTree(const NextIdxPos &next);

	int nearestVertex(const glm::vec3 &v);

private:
	typedef std::pair<glm::vec3, size_t> PairedVec;
	typedef std::vector<PairedVec> PairedVecs;

	struct Node
	{
		Node()
		{

		}

		Node *wind(const glm::vec3 &t);

		Node(PairedVecs &ordered, int k);
		
		void split(PairedVecs &what, Node **assign, Node *next);

		int coord{};
		glm::vec3 pos{};
		size_t idx{};

		Node *left{};
		Node *right{};
		PairedVecs lefts{};
		PairedVecs rights{};
	};
	
	std::vector<Node> _nodes;
};

#endif
