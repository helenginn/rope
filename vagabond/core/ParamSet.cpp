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

#include "Atom.h"
#include "ParamSet.h"
#include "Parameter.h"

void ParamSet::expandNeighbours()
{
	ParamSet copy(*this);

	for (Parameter *p : copy)
	{
		ParamSet more(p->relatedParameters());
		*this += more;
	}
}

void ParamSet::excludeBeyond(int residue_num, int direction)
{
	ParamSet copy;
	for (Parameter *p : *this)
	{
		float actual = p->residueId().as_num();
		float diff = actual - residue_num;

		if (diff * direction < 0) // acceptable
		{
			copy.insert(p);
		}
	}
	
	*this = copy;
}

std::ostream &operator<<(std::ostream &ss, const ParamSet &set) 
{
	ss << "Parameters: ";
	for (Parameter *p : set)
	{
		ss << p->residueId() << ":" << p->desc() << ",";
	}
	ss << std::endl;
	return ss;
}

ParamSet ParamSet::terminalSubset() const
{
	ParamSet subset;
	for (Parameter *const &p : *this)
	{
		Atom *terminals[2] = {p->atom(0), p->atom(3)};
		
		bool ok[2] = {true, true};
		for (Parameter *const &q : *this)
		{
			if (p == q || !q->isTorsion()) { continue; }
			
			Atom *check[2] = {q->atom(1), q->atom(2)};
			
			if (check[0] == terminals[0] || check[1] == terminals[0])
			{
				ok[0] = false;
			}
			else if (check[0] == terminals[1] || check[1] == terminals[1])
			{
				ok[1] = false;
				break;
			}
		}
		
		if (ok[0] || ok[1])
		{
			subset.insert(p);
		}
	}
	
	return subset;
}
