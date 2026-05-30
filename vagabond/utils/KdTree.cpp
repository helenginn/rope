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

#include <set>
#include <iostream>

#include "KdTree.h"

KdTree::KdTree(const NextIdxPos &next)
{
	auto make_compare = [](int coord)
	{
		return [coord](const PairedVec &a, const PairedVec &b)
		{
			return a.first[coord] < b.first[coord];

		};
	};
	
	PairedVecs list;
	
	auto reorder_list = [&make_compare](PairedVecs &list,
	                                    int coord)
	{
		std::sort(list.begin(), list.end(), make_compare(coord));
	};
	
	
	while (true)
	{
		size_t idx = -1; glm::vec3 vec;
		if (!next(idx, vec))
		{
			break;
		}

		list.push_back({vec, idx});
	}

	reorder_list(list, 0);
	_nodes.resize(list.size());

	int min = 0;
	_nodes[min] = Node(list, 0);
	int max = min + 1;
	
	auto split_node = [this, &min, &max, &reorder_list]
	(Node *curr, PairedVecs &which, Node **assign, int i)
	{
		if (which.size())
		{
			reorder_list(which, ((curr->coord + 1) % 3));
			curr->split(which, assign, &_nodes[max]);
			max++;
		}

		if (i == min && curr->lefts.size() == 0 && curr->rights.size() == 0)
		{
			min++;
		}
	};

	//maybe this exits cleanly...
	while (true)
	{
		for (int i = min; i < max; i++)
		{
			Node *curr = &_nodes[i];
			split_node(curr, curr->lefts, &curr->left, i);
		}

		for (int i = min; i < max; i++)
		{
			Node *curr = &_nodes[i];
			split_node(curr, curr->rights, &curr->right, i);
		}
		
		if (min == _nodes.size())
		{
			break;
		}
	}
}

KdTree::Node *KdTree::Node::wind(const glm::vec3 &t)
{
	auto distance = [&t](Node *node)
	{
		float d = glm::length(node->pos - t);
		return d;
	};

	auto closest = [&t, &distance](Node *a, Node *b) -> Node *
	{
		if (!a) return b;
		if (!b) return a;

		float adsq = glm::dot(a->pos - t, a->pos - t);
		float bdsq = glm::dot(b->pos - t, b->pos - t);
		Node *best = (adsq < bdsq ? a : b);
		return best;
	};

	if (!left && !right)
	{
		return this;
	}

	bool is_less = t[coord] < pos[coord];
	
	Node *main = (is_less ? left : right);
	Node *other = (is_less ? right : left);
	
	Node *drill = nullptr;

	if (main)
	{
		drill = main->wind(t);
	}

	Node *best = closest(this, drill);

	float rootsq = glm::dot(t - best->pos, t - best->pos);
	float axial = t[coord] - pos[coord];
	axial *= axial;

	if (rootsq >= axial && other)
	{
		Node *otherdrill = other->wind(t);
		best = closest(best, otherdrill);
	}
	
	return best;
}

int KdTree::nearestVertex(const glm::vec3 &v)
{
	Node *root = &_nodes[0];
	Node *best = root->wind(v);
	return best->idx;
}

KdTree::Node::Node(PairedVecs &ordered, int k)
{
	coord = k;
	int inc = (ordered.size() / 2);
	PairedVecs::iterator mid = ordered.begin();
	mid += inc; 
	pos = mid->first;
	idx = mid->second;

	lefts = PairedVecs(ordered.begin(), mid);
	rights = PairedVecs(mid + 1, ordered.end());
}

void KdTree::Node::split(PairedVecs &which, Node **assign, Node *next)
{
	*next = Node(which, ((coord + 1) % 3));
	*assign = next;
	which = {};

}

