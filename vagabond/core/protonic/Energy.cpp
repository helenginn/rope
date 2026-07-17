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

#include "Energy.h"
#include "matrix_functions.h"

using namespace hnet;

Energy::Energy()
{
	_sources[Torsion] = false;
	_sources[Acceptor] = false;
	_sources[Bulk] = false;
	_sources[Distance] = false;
	_sources[Angle] = false;
}

hnet::EnergyWrapper
Energy::energy_wrapper_for_covalent(BondProbe &bp)
{
	Probe &left = bp.left();
	Probe &right = bp.right();
	
	auto acquire_connected_atoms_for = [&bp](Probe &probe)
	{
		std::vector<Probe *> connected;
		for (Probe *const &other : probe.others())
		{
			if (!other->is_covalent())
			{
				continue;
			}
			BondProbe *bOther = static_cast<BondProbe *>(other);
			if (bOther == &bp)
			{
				continue;
			}
			if (bOther->existence().value() != Existence::Present)
			{
				continue;
			}

			if (&bOther->left() == &probe)
			{
				connected.push_back(&bOther->right());
			}
			else 
			{
				connected.push_back(&bOther->left());
			}
		}
		return connected;
	};

	auto func = [&bp, &left, &right, acquire_connected_atoms_for]
	()
	{
		if (bp.existence().value() != Existence::Present)
		{
			return 0.f;
		}
		std::vector<Probe *> lefts = acquire_connected_atoms_for(left);

		std::vector<Probe *> rights = acquire_connected_atoms_for(right);
		
		glm::vec3 l = left.position();
		glm::vec3 r = right.position();
		float sum = 0;
		for (Probe *const &extrLeft : lefts)
		{
			glm::vec3 ll = extrLeft->position();
			bool isHL = (extrLeft->atom()->elementSymbol() == "H");

			for (Probe *const &extrRight : rights)
			{
				glm::vec3 rr = extrRight->position();
				glm::vec3 poz[] = {ll, l, r, rr};
				bool isHR = (extrRight->atom()->elementSymbol() == "H");
				float torsion = measure_bond_torsion(poz);

				if (fabs(torsion) > 60.f)
				{
					continue;
				}

				float x = deg2rad(torsion * 3.f);
				float cosine = cos(x); // relative
				if (isHL) cosine *= 0.5;
				if (isHR) cosine *= 0.5;
				sum += cosine;
			}
		}

		return sum;
	};
	return modulate({{func, Torsion}});
}

auto evaluate_hbond(BondProbe &bp)
{
	return [&bp](const hnet::Bond::Values &expect = hnet::Bond::Acceptor,
	             bool only = true)
	{
		hnet::Existence::Values ex = bp._exist.value();

		if (ex == hnet::Existence::Absent && only)
		{
			return false;
		}

		hnet::Bond::Values val = bp._obj.value();
		if (only && (int)val == (int)expect)
		{
			return true;
		}
		else if (!only && val & expect)
		{
			return true;
		}
		return false;
	};
}

hnet::EnergyWrapper 
Energy::energy_wrapper_for_hbond_angle(HydrogenProbe *probe, 
                                       BondProbe &left, BondProbe &right,
                                       AtomProbe &lAtom, AtomProbe &rAtom)
{
	auto hbond_angle_for_bond = [probe](BondProbe &bp, AtomProbe &centre,
	                                     AtomProbe &other)
	{
		auto evaluate = evaluate_hbond(bp);
		if (!evaluate(hnet::Bond::Acceptor, true))
		{
			return 0.f;
		}

		centre.realign();
		other.realign();

		
		glm::vec3 lPos = centre.position();
		glm::vec3 rPos = other.position();
		glm::vec3 hPos = probe->position();
		glm::vec3 hl = glm::normalize(hPos - lPos);
		glm::vec3 rl = glm::normalize(rPos - lPos);
		float dot = glm::dot(hl, rl);
		float angle = acos(dot);
		float exag = angle * 3;
		float damping = std::max(cos(exag), 0.f);
		
		return 1.0f * -damping;
	};

	auto hbond_angle = [&, hbond_angle_for_bond]()
	{
		float total = 0;
		total += hbond_angle_for_bond(left, lAtom, rAtom);
		total += hbond_angle_for_bond(right, rAtom, lAtom);
		return total;
	};

	return modulate({{hbond_angle, Angle}});
}

hnet::EnergyWrapper
Energy::energy_wrapper_for_half_hbond(HydrogenProbe *probe, BondProbe &bp, 
                                      glm::vec3 pos)
{
	auto evaluate = evaluate_hbond(bp);

	auto hbond_base = [evaluate, &bp]()
	{
		float base = 1.0;
		if (evaluate(hnet::Bond::Acceptor, true))
		{
			float contrib = -base;
			return contrib;
		}
		else if (evaluate(hnet::Bond::Acceptor, false))
		{
			float contrib = -base;
			if (bp.existence().value() & hnet::Existence::Absent)
			{
				contrib /= 2.f;
			}
			return contrib;
		}
	};

	auto hbond_dist = [evaluate, probe, pos]()
	{
		if (!evaluate(hnet::Bond::Acceptor, false))
		{
			return 0.f;
		}

		glm::vec3 hPos = probe->position();
		float dist = glm::length(hPos - pos);
		if (dist != dist)
		{
			return 0.f;
		}

		float base = 1.0;
		float mod = 1 / (dist * dist);
		float contrib = -base * mod;
		if (!evaluate(hnet::Bond::Acceptor, true))
		{
			contrib /= 2;
		}
		return contrib;
	};

	return modulate({{hbond_base, Acceptor}, {hbond_dist, Distance}});
}

hnet::EnergyWrapper
Energy::energy_wrapper_for_liberated_bulk(AtomProbe &bulk)
{
	ExistenceConnector &bulkExist = bulk.existence();

	auto func = [&bulkExist]()
	{
		float base = 1.0;
		hnet::Existence::Values ex = bulkExist.value();

		if (ex == hnet::Existence::Absent)
		{
			return 0.f;
		}

		// bulk solvent will provide 3.6 hydrogen bonds on average
		float contrib = -base * 1.0;
		return contrib;
	};
	
	return modulate({{func, Bulk}});
}

hnet::EnergyWrapper
Energy::modulate(const std::vector<SourcedEnergy> &sources)
{
	return [this, sources]() -> hnet::GetEnergy
	{
		struct Cached
		{
			bool *ptr;
			float *amp;
			float energy;
		};

		std::vector<Cached> cache;

		for (const SourcedEnergy &se : sources)
		{
			float cached_energy = se.first();
			if (fabs(cached_energy) < 1e-6)
			{
				continue;
			}
			if (cached_energy != cached_energy)
			{
				continue;
				cached_energy = 0.f;
			}
			bool *source_ptr = &_sources[se.second];
			float *amp_ptr = &_amplifiers[se.second];
			cache.push_back({source_ptr, amp_ptr, cached_energy});
		}
		
		if (cache.size() == 0) return {};
		
		return [this, cache]()
		{
			float total = 0;
			for (const auto &entry : cache)
			{
				if (*entry.ptr)
				{
					total += entry.energy * exp(*entry.amp);
				}
				if (total != total)
				{
					std::cout << "Entry: " << *entry.ptr << " " << *entry.amp << " " <<
					entry.energy << std::endl;
				}
			}

			return total;
		};
	};
}
