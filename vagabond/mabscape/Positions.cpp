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
		if (fid.antigen == antigen_name && fid.contact)
		{
			ContactPoint *cp = fid.contact;
			Symmetry &sym = *cp;

			int n = sym.transforms().size();
			int i = _raw.size();
			AntibodyPos ap = {&fid, fid.name, i, n, sym, 0, nullptr};
			ap.mut = new std::mutex();
			_raw.resize(_raw.size() + n);
			ap.fixed = true;
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

	_shown = _raw;
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
		if (ap.fixed)
		{
			continue;
		}
		
		glm::vec3 random = _random();
		ap.setPosition(_raw, random, _fromMesh);
	}
}

AbWatch *Positions::AntibodyPos::icoAbWatch(const AbWatch::Source &source,
                                            int offset) const
{
	Icosahedron ico;
	ico.triangulate();

	std::string title = name + "(" + std::to_string(offset) + ")";
	AbWatch *aw = new AbWatch(ico, source, mut, title);
	aw->setAntibody(name, offset);
	
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
			// the live position, or the one on show while viewing; the
			// number changes with either, and with the switch between them
			const AntibodyPos *p = &ap;
			AbWatch::Source source = [this, p, i](glm::vec3 &where)
			{
				if (_viewing)
				{
					where = _shown[p->start_idx + i];
					return (unsigned int)(0x80000000u | p->shownVersion);
				}

				where = _raw[p->start_idx + i];
				return p->version;
			};

			AbWatch *aw = ap.icoAbWatch(source, i);
			
			aw->setFixedStyle(ap.fixed);

			bucket->addObject(aw);
			watches.push_back(aw);
		}
	}
}

const glm::vec3 &Positions::operator()(const std::string &name,
                                       const glm::vec3 *closest,
                                       bool shown) const
{
	if (_lookup.count(name) == 0)
	{
		throw std::runtime_error("Antibody name not found");
	}
	AntibodyPos *ap = _lookup.at(name);
	return ap->closest_to(shown ? _shown : _raw, closest);
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
		if (!ap.fixed)
		{
			total++;
		}
	}

	return total * 3;
}

std::vector<Positions::Placement> Positions::snapshot() const
{
	std::vector<Placement> placements;

	for (const AntibodyPos &ap : _positions)
	{
		std::unique_lock<std::mutex> lock(*ap.mut);
		Placement pl;
		pl.name = ap.name;
		pl.fixed = ap.fixed;
		pl.meshIdx = ap.meshIdx;
		pl.copies.assign(_raw.begin() + ap.start_idx, 
		                 _raw.begin() + ap.start_idx + ap.num);
		placements.push_back(pl);
	}

	return placements;
}

void Positions::apply(const std::vector<Placement> &placements)
{
	for (const Placement &pl : placements)
	{
		// whether it is fixed now, not when the result was made
		if (!_lookup.count(pl.name) || _lookup.at(pl.name)->fixed)
		{
			continue;
		}

		AntibodyPos &ap = *_lookup.at(pl.name);
		std::unique_lock<std::mutex> lock(*ap.mut);

		for (int i = 0; i < ap.num && i < (int)pl.copies.size(); i++)
		{
			_raw[ap.start_idx + i] = pl.copies[i];
		}

		ap.meshIdx = pl.meshIdx;
		ap.version++;
	}
}

void Positions::setCopies(const std::string &name,
                          const std::vector<glm::vec3> &copies)
{
	if (!_lookup.count(name))
	{
		return;
	}

	AntibodyPos &ap = *_lookup.at(name);
	std::unique_lock<std::mutex> lock(*ap.mut);

	for (int i = 0; i < ap.num && i < (int)copies.size(); i++)
	{
		int m = -1;
		if (_fromMesh)
		{
			_fromMesh(copies[i], _raw[ap.start_idx + i], m);
		}
		else
		{
			_raw[ap.start_idx + i] = copies[i];
		}

		if (i == 0)
		{
			ap.meshIdx = m;
		}
	}

	ap.version++;
}

bool Positions::cluster(const std::vector<const std::vector<Placement> *> &all,
                        const std::string &name, Clusters &out)
{
	if (all.empty())
	{
		return false;
	}

	const Placement *first = nullptr;
	for (const Placement &pl : *all[0])
	{
		if (pl.name == name && !pl.fixed && pl.copies.size())
		{
			first = &pl;
		}
	}

	if (!first)
	{
		return false;
	}

	// k-means, one cluster per symmetry operator, seeded by the first
	// result's copies so that cluster k stays with operator k
	size_t n = first->copies.size();
	out.points.clear();
	out.resultOf.clear();
	for (size_t r = 0; r < all.size(); r++)
	{
		for (const Placement &pl : *all[r])
		{
			if (pl.name == name && pl.copies.size() == n)
			{
				out.points.insert(out.points.end(), pl.copies.begin(), 
				                  pl.copies.end());
				out.resultOf.insert(out.resultOf.end(), n, r);
			}
		}
	}

	out.centres = first->copies;
	out.member.assign(out.points.size(), n);

	for (int iter = 0; iter < 50; iter++)
	{
		bool changed = false;
		std::vector<glm::vec3> sums(n, glm::vec3(0.f));
		std::vector<int> counts(n, 0);

		for (size_t p = 0; p < out.points.size(); p++)
		{
			size_t best = 0; float bestSq = FLT_MAX;
			for (size_t k = 0; k < n; k++)
			{
				glm::vec3 diff = out.points[p] - out.centres[k];
				float sq = glm::dot(diff, diff);
				if (sq < bestSq)
				{
					bestSq = sq;
					best = k;
				}
			}

			changed |= (out.member[p] != best);
			out.member[p] = best;
			sums[best] += out.points[p];
			counts[best]++;
		}

		for (size_t k = 0; k < n; k++)
		{
			if (counts[k])
			{
				out.centres[k] = sums[k] / (float)counts[k];
			}
		}

		if (!changed)
		{
			break;
		}
	}

	return true;
}

void Positions::applyAverage(const std::vector<const std::vector<Placement> *> 
                             &all)
{
	if (all.empty())
	{
		return;
	}

	for (const Placement &first : *all[0])
	{
		if (!_lookup.count(first.name) || _lookup.at(first.name)->fixed)
		{
			continue;
		}

		Clusters clusters;
		if (cluster(all, first.name, clusters))
		{
			setCopies(first.name, clusters.centres);
		}
	}
}

void Positions::copyLiveToShown()
{
	for (const AntibodyPos &ap : _positions)
	{
		std::unique_lock<std::mutex> lock(*ap.mut);
		std::copy(_raw.begin() + ap.start_idx, 
		          _raw.begin() + ap.start_idx + ap.num, 
		          _shown.begin() + ap.start_idx);
	}
}

void Positions::AntibodyPos::setShown(std::vector<glm::vec3> &shown,
                                      const std::vector<glm::vec3> &copies,
                                      const FromMesh &fromMesh)
{
	for (int i = 0; i < num && i < (int)copies.size(); i++)
	{
		int m = -1;
		if (fromMesh)
		{
			fromMesh(copies[i], shown[start_idx + i], m);
		}
		else
		{
			shown[start_idx + i] = copies[i];
		}
	}
}

void Positions::view(const std::vector<Placement> &placements)
{
	copyLiveToShown();

	for (const Placement &pl : placements)
	{
		// fixed now, not when the result was made
		if (!_lookup.count(pl.name) || _lookup.at(pl.name)->fixed)
		{
			continue;
		}

		const AntibodyPos &ap = *_lookup.at(pl.name);
		for (int i = 0; i < ap.num && i < (int)pl.copies.size(); i++)
		{
			_shown[ap.start_idx + i] = pl.copies[i];
		}
	}

	for (AntibodyPos &ap : _positions)
	{
		std::unique_lock<std::mutex> lock(*ap.mut);
		ap.shownVersion++;
	}

	_viewing = true;
}

void Positions::viewAverage(const std::vector<const std::vector<Placement> *> 
                            &all)
{
	if (all.empty())
	{
		return;
	}

	copyLiveToShown();

	for (const Placement &first : *all[0])
	{
		if (!_lookup.count(first.name) || _lookup.at(first.name)->fixed)
		{
			continue;
		}

		Clusters clusters;
		if (cluster(all, first.name, clusters))
		{
			_lookup.at(first.name)->setShown(_shown, clusters.centres, 
			                                 _fromMesh);
		}
	}

	for (AntibodyPos &ap : _positions)
	{
		std::unique_lock<std::mutex> lock(*ap.mut);
		ap.shownVersion++;
	}

	_viewing = true;
}

void Positions::resumeLive()
{
	_viewing = false;
}

bool Positions::isFixed(const std::string &name) const
{
	return _lookup.at(name)->fixed;
}

std::vector<std::string> Positions::structuralAntibodies() const
{
	std::vector<std::string> names;
	for (const AntibodyPos &ap : _positions)
	{
		if (ap.fid)
		{
			names.push_back(ap.name);
		}
	}

	return names;
}

std::vector<glm::vec3> Positions::structuralCopies(const std::string &name,
                                                   bool snapped) const
{
	std::vector<glm::vec3> copies;
	if (!_lookup.count(name) || !_lookup.at(name)->fid)
	{
		return copies;
	}

	const AntibodyPos &ap = *_lookup.at(name);
	Symmetry sym = ap.sym;
	glm::vec3 ref = sym.reference();

	sym.reset();
	for (const glm::mat4x4 &transform : sym.transforms())
	{
		sym.applyTransform(transform, Symmetry::next_pointer(&ref));

		glm::vec3 where = ref;
		int idx = -1;
		if (snapped && _fromMesh)
		{
			_fromMesh(ref, where, idx);
		}

		copies.push_back(where);
	}

	return copies;
}

float Positions::randomDistance(int samples) const
{
	if (!_random || samples <= 0)
	{
		return 0.f;
	}

	Symmetry sym = _antigen.sym;
	double total = 0;

	for (int i = 0; i < samples; i++)
	{
		glm::vec3 one = _random();
		glm::vec3 other = _random();
		float nearest = glm::length(one - other);

		// the other vertex put through each symmetry operator in turn
		sym.reset();
		for (const glm::mat4x4 &transform : sym.transforms())
		{
			sym.applyTransform(transform, Symmetry::next_pointer(&other));
			nearest = std::min(nearest, glm::length(one - other));
		}

		total += nearest;
	}

	return total / samples;
}

void Positions::setFixed(const std::string &name, bool fixed)
{
	if (!_lookup.count(name) || !_lookup.at(name)->fid)
	{
		return;
	}

	AntibodyPos &ap = *_lookup.at(name);
	bool changed = (ap.fixed != fixed);
	ap.fixed = fixed;

	// back to the structural position; when freed it simply stays put
	if (fixed && changed)
	{
		ap.setPosition(_raw, ap.sym.reference(), _fromMesh);
	}
}
