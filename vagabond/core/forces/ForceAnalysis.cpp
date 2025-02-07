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

#include <gemmi/elem.hpp>

#include <vagabond/utils/FileReader.h>
#include "ForceAnalysis.h"
#include "BondTorsion.h"
#include "BondLength.h"
#include "BondAngle.h"
#include "AtomContent.h"
#include "SymMates.h"
#include "Particle.h"
#include "Torque.h"
#include "Force.h"
#include "Atom.h"

#include "Particle.h"
#include "Rod.h"

ForceAnalysis::ForceAnalysis(AtomContent *group) : _group(group)
{
}

void ForceAnalysis::applyForce(Particle *p, Force *force, float magnitude)
{
	p->applyForce(force, magnitude);

	if (_handleForce)
	{
		_handleForce(p, force, magnitude);
	}

	_allForces.insert(force);
}

void ForceAnalysis::applyTorque(Particle *p, Rod *rod, 
                                Torque *torque, float magnitude)
{
	rod->applyTorque(torque, magnitude);

	if (_handleTorque)
	{
		_handleTorque(p, rod, torque, magnitude);
	}
	
	_allForces.insert(torque);
}

void ForceAnalysis::createParticles()
{
	for (Atom *atom : _group->atomVector())
	{
		Particle *p = new Particle();
		p->setDescription([atom] () { return atom->desc(); });
		p->setPositionGetter([atom]() { return atom->initialPosition(); });
		_forces.addParticle(p);
		_atom2Particle[atom] = p;

		if (atom->bondLengthCount() == 0 || atom->symmetryCopyOf() != nullptr)
		{
			continue;
		}

		Force *force = new Force(Force::StatusUnknown, 
		                         Force::ReasonReactionForce);
		applyForce(p, force, 1);
	}
}

void ForceAnalysis::createRods()
{
	for (int i = 0; i < _group->bondLengthCount(); i++)
	{
		BondLength *bond = _group->bondLength(i);
		Atom *left = bond->atom(0);
		Atom *right = bond->atom(1);
		
		Particle *p = _atom2Particle[left];
		Particle *q = _atom2Particle[right];

		Rod *rod = new Rod(p, q);

		_forces.addRod(rod);
		_bond2Rod[bond] = rod;

		if (left->elementSymbol() == "H" || right->elementSymbol() == "H")
		{
//			continue;
		}
		
		rod->setCategory(1);

		auto get_unit = [bond]()
		{
			glm::vec3 a = bond->atom(0)->initialPosition();
			glm::vec3 b = bond->atom(1)->initialPosition();
			glm::vec3 perp = glm::normalize(b - a);
			return perp;
		};

		auto get_mag = [bond]()
		{
			float z_score = bond->as_signed_z_score(bond->measurement());
			return z_score;
		};

		// FORCE ATTRIBUTABLE TO COMPRESSION/TENSION ON BOND LENGTH
		Force *force = new Force(Force::StatusKnown, Force::ReasonBondLength);
		force->setUnitGetter(get_unit);
		force->setMagGetter(get_mag);

		applyForce(p, force, 1);
		applyForce(q, force, -1);
	}
}

void ForceAnalysis::createTorsionTorques()
{
	for (int i = 0; i < _group->bondLengthCount(); i++)
	{
		BondLength *bond = _group->bondLength(i);
		Rod *rod = _bond2Rod[bond];
		Atom *left = bond->atom(0);
		Atom *right = bond->atom(1);

		auto get_unit = [left, right]()
		{
			glm::vec3 a = left->initialPosition(); // replace with particle
			glm::vec3 b = right->initialPosition(); // replace with particle
			glm::vec3 perp = glm::normalize(b - a);
			return perp;
		};

		std::vector<BondTorsion *> torsions = 
		left->findBondTorsions(nullptr, left, right, nullptr);
		
		for (BondTorsion *t : torsions)
		{
			if (t->isConstrained())
			{
				continue;
			}

			auto get_mag = [t, left]() -> float
			{
				int backward = (t->atom(1) == left);
				int mult = backward ? -1 : 1;
				float lw = t->atom(0)->elementSymbol() == "H" ? 1 : 4;
				float rw = t->atom(3)->elementSymbol() == "H" ? 1 : 4;
				float candidate = t->measurement(BondTorsion::SourceDerived);
				float limit = 60;
				float mag = 0;
				if (fabs(candidate) < limit)
				{
					float transformed = candidate / limit * M_PI;
					float sine = -sin(transformed);
					mag = sine * lw * rw * 0.05;
				}
				return mag;
			};
			
			if (fabs(get_mag()) < 1e-6)
			{
				continue;
			}

			Torque *torque = new Torque(Torque::StatusKnown,
			                            Torque::ReasonBondTorsion);
			torque->setUnitGetter(get_unit);
			torque->setMagGetter(get_mag);

			auto add_to_torsion = [this, t, torque](int n, int m, float dir)
			{
				Particle *point = _atom2Particle[t->atom(m)];
				BondLength *applied = t->atom(m)->findBondLength(t->atom(m),
				                                                 t->atom(n));
				Rod *corresponding_rod = _bond2Rod[applied];
				applyTorque(point, corresponding_rod, torque, dir);
			};
			
			add_to_torsion(1, 0, 1);
			add_to_torsion(2, 3, -1);
		}
	}
}

void ForceAnalysis::createBondAngleTorques()
{
	for (int i = 0; i < _group->bondAngleCount(); i++)
	{
		BondAngle *ang = _group->bondAngle(i);
		BondLength *l = _group->findBondLength(ang->atom(0), ang->atom(1));
		BondLength *r = _group->findBondLength(ang->atom(1), ang->atom(2));
		
		Rod *left = _bond2Rod[l];
		Rod *right = _bond2Rod[r];
		if (ang->atom(0)->elementSymbol() == "H" || 
		    ang->atom(2)->elementSymbol() == "H")
		{
			continue;
		}
		
		Particle *centre = _atom2Particle[ang->atom(1)];
		
		auto get_moment_unit = [ang]()
		{
			glm::vec3 a = ang->atom(0)->initialPosition();
			glm::vec3 b = ang->atom(1)->initialPosition();
			glm::vec3 c = ang->atom(2)->initialPosition();
			glm::vec3 perp = glm::normalize(glm::cross(c - a, b - a));
			return perp;
		};

		auto get_moment_mag = [ang]()
		{
			float z_score = ang->as_signed_z_score(ang->measurement());
			return -z_score;
		};
		
		Torque *torque = new Torque(Torque::StatusKnown, 
		                            Torque::ReasonBondAngle);
		torque->setUnitGetter(get_moment_unit);
		torque->setMagGetter(get_moment_mag);

		applyTorque(centre, left, torque, 1);
		applyTorque(centre, right, torque, -1);
	}
}

void ForceAnalysis::createElectrostaticContacts()
{
	float ave_charge = 0;

	auto charge = [&ave_charge](Atom *a) -> float
	{
		float charge = 0;
		if (a->elementSymbol() == "C")
		{
			charge += 0.3;
		}
		if (a->elementSymbol() == "H")
		{
			charge += 0.1;
		}
		else if (a->elementSymbol() == "N")
		{
			charge -= 0.6;
		}
		else if (a->elementSymbol() == "O")
		{
			charge -= 0.7;
		}

		return charge;
	};

	auto electrostatic_gradient = [this, charge](Atom *a, Atom *b)
	{
		Particle *p = _atom2Particle[a];
		Particle *q = _atom2Particle[b];

		float ch1 = charge(a);
		float ch2 = charge(b);

		const glm::vec3 &apos = p->pos();
		const glm::vec3 &bpos = q->pos();

		glm::vec3 posdiff = apos - bpos;
		float r = glm::length(posdiff);
		float force = -ch1 * ch2 / (r * r);
		return force * 12.;
	};
	
	std::map<Particle *, glm::vec3> prep;

	float total = 0; float count = 0;
	for (int i = 0; i < _group->size(); i++)
	{
		Atom *atom = (*_group)[i];
		total += charge(atom); count++;
	}
	ave_charge = total / count;
	std::cout << "Average charge: " << ave_charge << std::endl;

	float thresh = 5.f;
	for (int i = 0; i < _group->size(); i++)
	{
		Atom *left = (*_group)[i];
		if (left->bondLengthCount() < 1 || left->symmetryCopyOf()) continue;
		for (int j = 0; j < _group->size(); j++)
		{
			Atom *right = (*_group)[j];
			if (left == right) continue;
			if (left->bondLengthCount() < 1 && 
			    !left->symmetryCopyOf()) continue;

			bool too_close = false;

			for (size_t k = 0; k < left->bondAngleCount(); k++)
			{
				if (left->bondAngle(k)->hasAtom(right))
				{
					too_close = true;
				}
			}

			if (too_close) continue;
			
			glm::vec3 diff = right->initialPosition() - left->initialPosition();
			if (fabs(diff.x) > thresh || fabs(diff.y) > thresh ||
			    fabs(diff.z) > thresh)
			{
				continue;
			}
			if (glm::dot(diff, diff) > thresh*thresh)
			{
				continue;
			}

			Particle *p = _atom2Particle[left];
			Particle *q = _atom2Particle[right];

			auto get_mag = [this, left, right, electrostatic_gradient]()
			{
				float magnitude = electrostatic_gradient(left, right);
				return magnitude;
			};
			
			prep[p] += glm::normalize(diff) * get_mag();

			/*
			Force *force = new Force(Force::StatusKnown, 
			                         Force::ReasonVdwContact);
			force->setUnitGetter(get_unit);
			force->setMagGetter(get_mag);
			
						applyForce(p, force, 1);
						applyForce(q, force, -1);
						*/
		}
	}

	for (auto it = prep.begin(); it != prep.end(); it++)
	{
		Particle *p = it->first;
		glm::vec3 vec = it->second;
		glm::vec3 unit = glm::normalize(vec);
		float mag = glm::length(vec);

		auto get_unit = [vec]()
		{
			return vec;
		};

		auto get_mag = [mag]()
		{
			return mag;
		};

		Force *force = new Force(Force::StatusKnown, 
		                         Force::ReasonElectrostaticContact);
		force->setUnitGetter(get_unit);
		force->setMagGetter(get_mag);

		applyForce(p, force, 1);
	}
}

void ForceAnalysis::createCloseContacts()
{
	auto vdw_gradient = [this](Atom *a, Atom *b)
	{
		Particle *p = _atom2Particle[a];
		Particle *q = _atom2Particle[b];

		float d = 0; float weight = 0;
		{
			gemmi::Element ele = gemmi::Element(a->elementSymbol());
			d += ele.vdw_r();
			weight += ele.atomic_number();
		}
		{
			gemmi::Element ele = gemmi::Element(b->elementSymbol());
			d += ele.vdw_r();
			weight += ele.atomic_number();
		}

		const glm::vec3 &apos = p->pos();
		const glm::vec3 &bpos = q->pos();

		glm::vec3 posdiff = apos - bpos;
		float r = glm::length(posdiff);
		float force = weight * (d - r) / 1000.f;
		if (force < 0) force = 0;
		return force;

		float dto3 = d * d * d;
		float dto6 = dto3 * dto3;
		float dto12 = dto6 * dto6;
		long double to3 = r * r * r;
		long double to6 = to3 * to3;
		long double to12 = to6 * to6;
		long double to13 = to12 * r;
		long double to7 = to6 * r;

		long double potential = 6 * dto6 / to7 - 12 * dto12 / to13;
		potential *= weight / 500;
		return (float)potential;
	};
	
	std::map<Particle *, glm::vec3> prep;

	float thresh = 8.f;
	for (int i = 0; i < _group->size(); i++)
	{
		Atom *left = (*_group)[i];
		if (left->bondLengthCount() < 1 || left->symmetryCopyOf()) continue;
		for (int j = 0; j < _group->size(); j++)
		{
			Atom *right = (*_group)[j];
			if (left == right) continue;
			if (right->bondLengthCount() < 1 && !right->symmetryCopyOf()) continue;

			bool too_close = false;

			for (size_t k = 0; k < left->bondAngleCount(); k++)
			{
				if (left->bondAngle(k)->hasAtom(right))
				{
					too_close = true;
				}
			}

			for (size_t k = 0; k < left->bondTorsionCount(); k++)
			{
				if (left->bondTorsion(k)->hasAtom(right))
				{
					too_close = true;
				}
			}

			if (too_close) continue;
			
			glm::vec3 diff = left->initialPosition() - right->initialPosition();
			if (fabs(diff.x) > thresh || fabs(diff.y) > thresh ||
			    fabs(diff.z) > thresh)
			{
				continue;
			}
			if (glm::dot(diff, diff) > thresh*thresh)
			{
				continue;
			}

			Particle *p = _atom2Particle[left];
			Particle *q = _atom2Particle[right];
			auto get_unit = [this, p, q]()
			{
				glm::vec3 diff = glm::normalize(p->pos() - q->pos());
				return diff;
			};

			auto get_mag = [this, left, right, vdw_gradient]()
			{
				float magnitude = vdw_gradient(left, right);
				return magnitude;
			};
			
			prep[p] += glm::normalize(diff) * get_mag();
//			prep[q] -= glm::normalize(diff) * get_mag();

			if (fabs(get_mag()) > 0.1)
			{
				std::cout << p->desc() << " to " << q->desc() << ": " << get_mag() 
				<< std::endl;
			}

			//			applyForce(p, force, 1);
			//			applyForce(q, force, -1);
		}
	}

	for (auto it = prep.begin(); it != prep.end(); it++)
	{
		Particle *p = it->first;
		glm::vec3 vec = it->second;
		glm::vec3 unit = glm::normalize(vec);
		float mag = glm::length(vec);

		auto get_unit = [vec]()
		{
			return vec;
		};

		auto get_mag = [mag]()
		{
			return mag;
		};

		Force *force = new Force(Force::StatusKnown, 
		                         Force::ReasonVdwContact);
		force->setUnitGetter(get_unit);
		force->setMagGetter(get_mag);

		applyForce(p, force, 1);
	}
}

void ForceAnalysis::augmentSymmetry()
{
	std::string spg_name = _group->spaceGroup();
	std::array<double, 6> uc = _group->unitCell();
	std::cout << "Sym info: " << std::endl;
	std::cout << spg_name << std::endl;
	if (uc.size())
	{
		std::cout << uc[0] << std::endl;
	}
	AtomGroup *mates = SymMates::getSymmetryMates(_group, spg_name, uc, 8.0);
	std::cout << "Found: " << mates->size() << std::endl;
	
	_group->add(mates);
}

void ForceAnalysis::convert()
{
	augmentSymmetry();
	createParticles();
	createRods();
	createTorsionTorques();
	createBondAngleTorques();
	createCloseContacts();
	createElectrostaticContacts();
}

void ForceAnalysis::toggleReason(AbstractForce::Reason reason, bool include)
{
	for (AbstractForce *force : _allForces)
	{
		if (force->reason() == reason)
		{
			force->setStatus(include ? AbstractForce::StatusKnown : 
			                 AbstractForce::StatusIgnored);
		}
	}
}

void ForceAnalysis::resetUnknowns()
{
	for (AbstractForce *force : _allForces)
	{
		if (force->status() == AbstractForce::StatusCalculatedDirOnly)
		{
			force->setStatus(AbstractForce::StatusKnownDirOnly);
		}
		else if (force->status() == AbstractForce::StatusCalculated)
		{
			force->setStatus(AbstractForce::StatusUnknown);
		}
	}

}

void ForceAnalysis::calculateUnknown()
{
	resetUnknowns();

	OpSet<AbstractForce *> active = _allForces;
	active.filter([](AbstractForce *f)
	{
		return (f->status() == AbstractForce::StatusUnknown || 
		        f->status() == AbstractForce::StatusKnownDirOnly);
	});
	
	OpSet<ForceCoordinate> coordinates;

	for (AbstractForce *force : active)
	{
		if (force->status() == AbstractForce::StatusKnownDirOnly)
		{
			coordinates.insert({force, 0});
		}
		else
		{
			for (int i = 0; i < 3; i++)
			{
				coordinates.insert({force, i});
			}
		}
	}

	std::map<ForceCoordinate, int> index_map = coordinates.indexing();

	for (auto it = index_map.begin(); it != index_map.end(); it++)
	{
		std::cout << it->first.first << ", " << it->first.second << ": " 
		<< it->second << std::endl;
		
	}

	_forces.calculateUnknowns(index_map);
}
