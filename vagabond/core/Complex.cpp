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

#include "Instance.h"
#include "Complex.h"

Complex::Complex()
{

}

void Complex::addLink(Node *existing, Interface *interface,
                                      Node *other)
{
	if (existing == other)
	{
		return;
	}

	Connection con_existing{};
	con_existing.interface = interface;
	con_existing.node = other;

	Connection con_other{};
	con_other.interface = interface;
	con_other.node = existing;

	existing->connections.push_back(con_existing);
	other->connections.push_back(con_other);
}

Complex::Node *Complex::addNode(Instance *instance)
{
	Entity *entity = instance->entity();
	Node node;
	node.entity = entity;
	_nodes.push_back(node);
	
	_inst2Node[instance] = &_nodes.back();
	return &_nodes.back();
}

Complex::Node *Complex::node(Instance *instance) const
{
	if (_inst2Node.count(instance))
	{
		return _inst2Node.at(instance);
	}

	return nullptr;
}

std::map<Entity *, int> entityCounts(Complex::Node *node)
{
	std::map<Entity *, int> node_map;
	for (Complex::Connection &c : node->connections)
	{
		if (!c.node->flag)
		{
			node_map[c.node->entity]++;
		}
	}

	return node_map;
}

bool Complex::compatibleChildren(Node *one, Node *two) const
{
	for (Connection &c : one->connections)
	{
		for (Connection &d : two->connections)
		{
			if (compatibleNodes(c.node, d.node) && 
			    !c.node->flag && !d.node->flag)
			{
				return compatibleChildren(c.node, d.node);
			}
		}
	}

	return compatibleNodes(one, two);
}

bool Complex::compatibleNodes(Node *one, Node *two) const
{
	if (one->entity != two->entity)
	{
		return false;
	}
	
	if (one->connections.size() != two->connections.size())
	{
		return false;
	}

	std::map<Entity *, int> one_map = entityCounts(one);
	std::map<Entity *, int> two_map = entityCounts(two);

	if (one_map != two_map)
	{
		return false;
	}
	
	one->flag = true;
	two->flag = true;
	
	return true;
	
	return true;
}

bool Complex::nodeLikeNode(Node *one, Node *two)
{
	for (Node &node : _nodes)
	{
		node.flag = false;
	}

	if (!compatibleNodes(one, two) || !compatibleChildren(one, two))
	{
		return false;
	}
	
	return true;
}

size_t Complex::ambiguities()
{
	std::map<Entity *, std::vector<Node *> > entity2Nodes;
	std::map<Entity *, std::vector<Node *> >::iterator it;

	for (Node &n : _nodes)
	{
		entity2Nodes[n.entity].push_back(&n);
	}
	
	size_t count = 1;
	
	for (it = entity2Nodes.begin(); it != entity2Nodes.end(); it++)
	{
		std::vector<Node *> &nodes = it->second;
		
		for (size_t i = 0; i < nodes.size() - 1; i++)
		{
			Node *one = nodes[i];

			for (size_t j = i + 1; j < nodes.size(); j++)
			{
				Node *two = nodes[j];
				
				if (nodeLikeNode(one, two))
				{
					count++;
				}
			}
		}
	}

	return count;
}

