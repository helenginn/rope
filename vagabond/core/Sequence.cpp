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

Sequence::Sequence(std::string str)
{
	for (size_t i = 0; i < str.length(); i++)
	{
		char ch = str[i];

		std::string tlc = gemmi::expand_protein_one_letter(ch);
		Residue r{ResidueId(i), tlc, " "};
		
		*this += r;
	}
}

Sequence::Sequence(Atom *anchor)
{
	_anchor = anchor;
	findSequence();
}

Sequence &Sequence::operator+=(Sequence *&other)
{
	for (size_t i = 0; i < other->_residues.size(); i++)
	{
		*this += other->_residues[i];
	}

	return *this;
}

Sequence &Sequence::operator+=(Residue &res)
{
	res.setSequence(this);
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
		resvec.push_back(_residues[i].code());
	}

	std::string olc = gemmi::one_letter_code(resvec);
	std::replace(olc.begin(), olc.end(), 'X', ' ');
	
	return olc;
}
