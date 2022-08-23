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

#ifndef __vagabond__Complex__
#define __vagabond__Complex__

#include <list>
#include <map>

class Interface;
class Molecule;
class Entity;

class Complex
{
public:
	Complex();

	struct Node;

	struct Connection
	{
		Interface *interface = nullptr;
		Node *node = nullptr;
	};

	struct Node
	{
		Entity *entity = nullptr;
		std::string tag;
		std::list<Connection> connections;
		bool flag = false;
	};
	
	size_t ambiguities();

	bool nodeLikeNode(Node *one, Node *two);

	Node *node(Molecule *mol) const;

	void addLink(Node *existing, Interface *interface, Node *other);
	Node *addNode(Molecule *molecule);
private:
	bool compatibleChildren(Node *one, Node *two) const;
	bool compatibleNodes(Node *one, Node *two) const;

	std::list<Node> _nodes;
	std::map<Molecule *, Node *> _mole2Node;

};

#endif
