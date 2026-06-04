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

#include <vagabond/gui/elements/Icosahedron.h>

#include "Positions.h"
#include "ContactPoint.h"
#include "AbWatch.h"
#include "Mab.h"

Positions::Positions(Antigen &antigen, const Competition &comp, Mab &mab,
                     const FromMesh &fm, const RandomFromMesh &rm) 
: _antigen(antigen), _mab(mab), _fromMesh(fm), _random(rm)
{
	OpSet<std::string> list;

	std::string antigen_name = comp.antigen;

	for (Fiducial &fid : _mab.fiducials)
	{
		if (fid.antigen == antigen_name)
		{
			ContactPoint *cp = fid.contact;
			Symmetry &sym = *cp;

			int n = sym.transforms().size();
			int i = _raw.size();
			AntibodyPos ap = {&fid, fid.name, i, n, sym, 0, nullptr};
			ap.mut = new std::mutex();
			_raw.resize(_raw.size() + n);
			ap.setPosition(_raw, sym.reference(), _fromMesh);

			list += fid.name;
			_positions.push_back(ap);
		}
	}
	
	OpSet<std::string> more = comp.antibody_names();

	for (const std::string &name : more)
	{
		if (list.count(name))
		{
			continue;
		}

		Symmetry &sym = _antigen.sym;

		int n = sym.transforms().size();
		int i = _raw.size();
		AntibodyPos ap = {nullptr, name, i, n, sym, 0, nullptr};
		ap.mut = new std::mutex();
		_raw.resize(_raw.size() + n);
		glm::vec3 random = _random();
		ap.setPosition(_raw, random, _fromMesh);

		list += name;
		_positions.push_back(ap);
	}
	
	std::cout << "Collected " << _positions.size() << " antibodies, covering "
	<< _raw.size() << " positions in total." << std::endl;
	
	for (AntibodyPos &ap : _positions)
	{
		_lookup[ap.name] = &ap;
	}
}

Positions::~Positions()
{
	for (AntibodyPos &pos : _positions)
	{
		delete pos.mut;
	}

}

void Positions::randomise()
{
	for (AntibodyPos &ap : _positions)
	{
		if (ap.fid)
		{
			continue;
		}
		
		glm::vec3 random = _random();
		ap.setPosition(_raw, random, _fromMesh);
	}
}

AbWatch *Positions::AntibodyPos::icoAbWatch(const std::vector<glm::vec3> &raw,
                                             int offset) const
{
	Icosahedron ico;
	ico.triangulate();
//	ico.setPosition(raw[start_idx + offset]);

	std::string title = name + "(" + std::to_string(offset) + ")";
	AbWatch *aw = new AbWatch(ico, version, mut, 
	                          raw[start_idx + offset], title);
	
	return aw;
}

void Positions::setPosition(const std::string &name, glm::vec3 pos, 
                            float snappiness)
{
	if (!_lookup.count(name))
	{
		std::cout << "Antibody not found: " << name << std::endl;
		return;
	}
	
	FromMesh snappy = [snappiness, name, this](const glm::vec3 &pos, 
	                                           glm::vec3 &update, int &idx)
	{
		glm::vec3 closest = {};
		_fromMesh(pos, closest, idx);
		glm::vec3 add = snappiness * (closest - pos);
		update = pos + add;

	};

	_lookup[name]->setPosition(_raw, pos, 
	                           (snappiness >= 1 ? snappy : snappy));
}

void Positions::AntibodyPos::setPosition(std::vector<glm::vec3> &raw, 
                                         glm::vec3 ref, const FromMesh &fromMesh)
{
	int i = start_idx;
	int offset = 0;

	std::unique_lock<std::mutex> lock(*mut);

	sym.reset();
	for (const glm::mat4x4 &transform : sym.transforms())
	{
		sym.applyTransform(transform, Symmetry::next_pointer(&ref));
		glm::vec3 old = raw[i + offset];
		int m = -1;
		if (fromMesh)
		{
			fromMesh(ref, raw[i + offset], m);
			if (offset == 0) meshIdx = m;
		}
		else
		{
			raw[i + offset] = ref;
		}
		offset++;
	}

	version++;
}

void Positions::loadAntibodiesInto(HasRenderables *bucket,
                                   std::vector<AbWatch *> &watches)
{
	for (const AntibodyPos &ap : _positions)
	{
		for (int i = 0; i < ap.num; i++)
		{
			AbWatch *aw = ap.icoAbWatch(_raw, i);
			
			if (!ap.fid)
			{
				aw->setSelectable(true);
				aw->setColour(0.2, 0.2, 0.6);
			}

			bucket->addObject(aw);
			watches.push_back(aw);
		}
	}
}

const glm::vec3 &Positions::operator()(const std::string &name,
                                       const glm::vec3 *closest) const
{
	if (_lookup.count(name) == 0)
	{
		throw std::runtime_error("Antibody name not found");
	}
	AntibodyPos *ap = _lookup.at(name);
	return ap->closest_to(_raw, closest);
}

const glm::vec3 &
Positions::AntibodyPos::closest_to(const std::vector<glm::vec3> &raw,
                                   const glm::vec3 *other) const
{
	 std::unique_lock<std::mutex> lock(*mut);
	if (other == nullptr)
	{
		return raw[start_idx];
	}

	float best = FLT_MAX; int n = -1;
	for (int i = start_idx; i < start_idx + num; i++)
	{
		const glm::vec3 &candidate = raw[i];
		glm::vec3 diff = candidate - *other;
		float sql = glm::dot(diff, diff);

		if (sql < best)
		{
			best = sql;
			n = i;
		}
	}

	if (n < 0)
	{
		std::cout << "WARNING!" << std::endl;
	}
	return raw[n];
}

int Positions::paramCount()
{
	int total = 0;
	for (const AntibodyPos &ap : _positions)
	{
		if (!ap.fid)
		{
			total++;
		}
	}

	return total * 3;
}

bool Positions::isFixed(const std::string &name) const
{
	return (_lookup.at(name)->fid);
}
