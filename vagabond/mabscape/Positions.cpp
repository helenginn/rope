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

Positions::Positions(Antigen &antigen, const Competition &comp, Mab &mab) 
: _antigen(antigen), _mab(mab)
{
	OpSet<std::string> list;

	std::string antigen_name = comp.antigen;

	for (Fiducial &fid : _mab.fiducials)
	{
		if (fid.antigen == antigen_name)
		{
			Symmetry *sym = fid.contact;
			if (!sym) { continue; }

			int n = sym->transforms().size();
			int i = _raw.size();
			AntibodyPos ap = {&fid, fid.name, i, n, sym, 0, nullptr};
			ap.mut = new std::mutex();
			_raw.resize(_raw.size() + n);
			ap.setPosition(_raw, sym->reference());

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
		AntibodyPos ap = {nullptr, name, i, n, &sym, 0, nullptr};
		ap.mut = new std::mutex();
		_raw.resize(_raw.size() + n);

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

AbWatch *Positions::AntibodyPos::icoAbWatch(const std::vector<glm::vec3> &raw,
                                             int offset) const
{
	Icosahedron ico;
	ico.triangulate();
	ico.setPosition(raw[start_idx + offset]);

	AbWatch *aw = new AbWatch(ico, version, mut, raw[start_idx + offset]);
	return aw;
}

void Positions::setPosition(const std::string &name, glm::vec3 pos)
{
	if (!_lookup.count(name))
	{
		std::cout << "Antibody not found: " << name << std::endl;
		return;
	}

	_lookup[name]->setPosition(_raw, pos);
}

void Positions::AntibodyPos::setPosition(std::vector<glm::vec3> &raw, 
                                         glm::vec3 ref)
{
	int i = start_idx;

	std::unique_lock<std::mutex> lock(*mut);

	for (const glm::mat4x4 &transform : sym->transforms())
	{
		sym->applyTransform(transform, Symmetry::next_pointer(&ref));
		raw[i] = ref;
		i++;
	}

	version++;
}

void Positions::loadAntibodiesInto(HasRenderables *bucket)
{
	for (const AntibodyPos &ap : _positions)
	{
		for (int i = 0; i < ap.num; i++)
		{
			AbWatch *aw = ap.icoAbWatch(_raw, i);
			bucket->addObject(aw);
		}
	}
}
