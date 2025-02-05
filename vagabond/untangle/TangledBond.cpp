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

#include "TangledBond.h"
#include <vagabond/core/BondTorsion.h>
#include <vagabond/core/BondLength.h>
#include <vagabond/core/BondAngle.h>
#include <vagabond/core/Atom.h>
#include <vagabond/core/matrix_functions.h>

TangledBond::TangledBond(BondLength &src) : _src(src)
{

}

float TangledBond::simple_score(const std::string &q, const std::string &r) 
{
//	return length_score(q, q, r, r);
	return sqrt(length_score(q, q, r, r) * angle_score(q, q, r, r));
}

std::vector<std::pair<std::string, std::string>> TangledBond::options()
{
	OpSet<std::string> lefts = _src.atom(0)->conformerList();
	OpSet<std::string> rights = _src.atom(1)->conformerList();
	OpSet<std::pair<std::string, std::string>> results;

	for (const std::string &first : lefts)
	{
		for (const std::string &second : rights)
		{
			std::pair<std::string, std::string> pair;
			pair = {first, second};
			results.insert(pair);
		}
	}
	
	return results.toVector();
}

template <typename Func>
void make_score(TangledBond *bond, Func &do_with_score)
{
	std::vector<std::pair<std::string, std::string>> pairs;
	pairs = bond->options();
	for (const std::pair<std::string, std::string> &pair : pairs)
	{
		float score = bond->simple_score(pair.first, pair.second);
		do_with_score(pair.first, pair.second, score);
	}
}

float TangledBond::volatility()
{
	struct Assimilate
	{
		void operator()
		(const std::string &first, const std::string &second, const float &score)
		{
			(first == second ? positive : negative) += score;
		}
		
		float operator()()
		{
			return negative / positive;
		}

		float positive = 0;
		float negative = 0;
	};

	Assimilate positive_minus_negative;
	make_score(this, positive_minus_negative);
	return positive_minus_negative();
}

float TangledBond::total_score(float bias)
{
	float total = 0; float count = 0;
	float penalty = 0; float penalty_count = 0;
	OpSet<std::string> lefts = _src.atom(0)->conformerList();
	OpSet<std::string> rights = _src.atom(1)->conformerList();

	for (const std::string &first : lefts)
	{
		if (first == "") continue;
		for (const std::string &second : rights)
		{
			if (second == "") continue;
			if (first == second)
			{
				float add = length_score(first, first, second, second);
				add *= angle_score(first, first, second, second);
				add = sqrt(add);
				total += add;
				count++;
			}
			else
			{
			}
		}
	}
	
	if (penalty_count <= 0 || count <= 0)
	{
//		return 0;
	}
	
//	penalty /= penalty_count;
	total /= count;
	
	return total - penalty;
}

float measured_ang(Atom *left, Atom *centre, Atom *right, const std::string &p, 
                   const std::string &q, const std::string &r)
{
	if ((left->conformerPositions().count(p) == 0)
	    || (centre->conformerPositions().count(q) == 0)
	    || (right->conformerPositions().count(r) == 0))
	{
		return 0;
	}

	glm::vec3 u = left->conformerPositions()[p].pos.ave;
	glm::vec3 v = centre->conformerPositions()[q].pos.ave;
	glm::vec3 w = right->conformerPositions()[r].pos.ave;

	float angle = rad2deg(glm::angle(normalize(u - v),
	                                 normalize(w - v)));

	return angle;
}

Atom *remaining_atom(BondAngle *angle, std::set<Atom *> others)
{
	for (size_t i = 0; i < 3; i++)
	{
		bool ok = true;
		
		for (Atom *atom : others)
		{
			if (angle->atom(i) == atom)
			{
				ok = false;
			}
		}
		
		if (ok) return angle->atom(i);
	}

	return nullptr;
}

float measured_torsion(Atom *left, Atom *midleft, Atom *midright, Atom *right,
                       const std::string &p, const std::string &q, 
                       const std::string &r, const std::string &s)
{
	glm::vec3 u = left->conformerPositions()[p].pos.ave;
	glm::vec3 v = midleft->conformerPositions()[q].pos.ave;
	glm::vec3 w = midright->conformerPositions()[r].pos.ave;
	glm::vec3 x = right->conformerPositions()[s].pos.ave;

	glm::vec3 positions[] = {u, v, w, x};
	float torsion = measure_bond_torsion(positions);

	return torsion;
}

float TangledBond::length_score(const std::string &p, const std::string &q, 
                                const std::string &r, const std::string &s)
{
	BondLength &bl = _src;
	
	Atom *left = bl.atom(0);
	Atom *right = bl.atom(1);

	std::set<Atom *> pair = {left, right};

	float total = 0;
	float count = 0;
	for (Atom *atom : pair)
	{
		Atom *other = (atom == left ? right : left);
		std::string myconf = (atom == left ? q : r);
		glm::vec3 v = atom->conformerPositions()[myconf].pos.ave;

		for (int i = 0; i < atom->bondLengthCount(); i++)
		{
			Atom *connected = atom->connectedAtom(i);
			if (connected->elementSymbol() == "H")
			{
				continue;
			}
			std::string otherconf = (atom == right ? q : r);
			
			if (other != connected)
			{
				otherconf = (atom == left ? p : s);
			}

			if (connected->conformerPositions().count(otherconf) == 0)
			{
				continue;
			}

			glm::vec3 w = connected->conformerPositions()[otherconf].pos.ave;

			float measured = glm::length(v - w);

			BondLength *length = atom->bondLength(i);
			float zsc = length->as_z_score(measured);
			total += -zsc * zsc;
			count++;
		}
	}

	total = exp(total / count);
	if (total != total) return 0;
	
	return total;
}
	
float TangledBond::angle_score(const std::string &p, const std::string &q, 
                               const std::string &r, const std::string &s)
{
	BondLength &bl = _src;
	
	Atom *left = bl.atom(0);
	Atom *right = bl.atom(1);

	std::set<Atom *> pair = {left, right};

	float total = 0;
	float count = 0;
	
	std::vector<BondAngle *> angles = left->findBondAngles(pair);

	for (BondAngle *angle : angles)
	{
		bool dir = angle->atomIsTerminal(right);
		Atom *last = remaining_atom(angle, pair);
		if (last->elementSymbol() == "H")
		{
			continue;
		}

		float measured = 0;
		
		if (!dir)
		{
			measured = measured_ang(left, right, last, q, r, s);
		}
		else
		{
			measured = measured_ang(right, left, last, r, q, p);
		}
		
		float zsc = angle->as_z_score(measured);
		float contrib = -zsc * zsc;
		total += contrib;
		count++;
	}

	if (((left->atomName() == "N" || right->atomName() == "C") || 
	              (left->atomName() == "C" || right->atomName() == "N")))
	{
		BondTorsion *t = left->findBondTorsion("CA-N-C-CA");
		if (t)
		{
			Atom *tmostleft = t->atom(0);
			Atom *tleft = t->atom(1);
			Atom *tright = t->atom(2);
			Atom *tmostright = t->atom(3);
			
			float torsion = 180;
			if (tmostleft == left)
			{
				torsion = measured_torsion(tmostleft, tleft, tright, tmostright,
				                           p, q, r, r);
			}
			else if (tleft == left)
			{
				torsion = measured_torsion(tmostleft, tleft, tright, tmostright,
				                           p, q, r, s);
			}
			else if (tright == left)
			{
				torsion = measured_torsion(tmostleft, tleft, tright, tmostright,
				                           p, p, q, r);
			}
			else if (tmostleft == right)
			{
				torsion = measured_torsion(tmostleft, tleft, tright, tmostright,
				                           p, p, p, q);
			}
			else if (tmostright == left)
			{
				torsion = measured_torsion(tmostleft, tleft, tright, tmostright,
				                           r, s, s, s);
			}

			// get it into a reasonable reference frame
			if (torsion < 0.f) torsion += 360.f;

			float stdev = 4.f;
			torsion -= 180.f; // minus mean
			float zsc = fabs(torsion / stdev);
//			std::cout << left->desc() << ": " << exp(-zsc * zsc) << std::endl;
			total += -zsc * zsc;
			count++;
		}
	}
	
	if (total != total) return 0;
	if (count == 0) return 0;

//	total /= count;
	
	total = exp(total / count);
	
	return total;
	return exp(-total);
}

std::string TangledBond::desc() const
{
	BondLength &bl = _src;
	Atom *left = bl.atom(0);
	Atom *right = bl.atom(1);

	std::string str = left->desc() + " <-> " + right->desc();
	return str;
}

bool TangledBond::hasHydrogens() const
{
	BondLength &bl = _src;
	Atom *left = bl.atom(0);
	Atom *right = bl.atom(1);

	return (left->elementSymbol() == "H" || right->elementSymbol() == "H");
}

Atom *TangledBond::atom(int i)
{
	return _src.atom(i);
}
