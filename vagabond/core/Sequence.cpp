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

#include "Sequence.h"
#include "Residue.h"
#include "Grapher.h"
#include "Atom.h"
#include <sstream>
#include <gemmi/polyheur.hpp>

Sequence::Sequence()
{

}

Sequence::Sequence(Atom *anchor)
{
	_anchor = anchor;
	findSequence();
}

Sequence &Sequence::operator+=(Sequence *&other)
{
	_residues.reserve(_residues.size() + other->_residues.size());
	_residues.insert(_residues.end(), other->_residues.begin(),
	                 other->_residues.end());
	return *this;
}

Sequence &Sequence::operator+=(Residue &res)
{
	_residues.push_back(res);
	return *this;
}

void Sequence::findSequence()
{
	Grapher gr;
	gr.generateGraphs(_anchor);

	int max = gr.graphCount();
	
	if (max <= 0)
	{
		return;
	}

	AtomGraph *last = gr.graph(0);
	
	while (last != nullptr)
	{
		Atom *ref = last->atom;
		ResidueId id = ref->residueId();
		std::string code = ref->code();
		std::string chain = ref->chain();
		Residue res{id, code, chain};
		_residues.push_back(res);
		
		last = gr.firstGraphNextResidue(last);
	}
}

std::string Sequence::str()
{
	std::ostringstream ss;
	std::vector<std::string> resvec;
	
	for (size_t i = 0; i < _residues.size(); i++)
	{
		ss << _residues[i].code() << " ";
		resvec.push_back(_residues[i].code());
	}

	std::string str = ss.str();
	
	if (_residues.size())
	{
		str.pop_back();
	}
	
	std::string olc = gemmi::one_letter_code(resvec);
	std::replace(olc.begin(), olc.end(), 'X', ' ');
	
	return olc;
}
