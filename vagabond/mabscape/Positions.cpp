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
#include <algorithm>
#include "ContactPoint.h"
#include "AbWatch.h"
#include "Mab.h"
#include <vagabond/utils/KdTree.h>
#include <cmath>
#include <array>
#include <unordered_map>

Positions::Positions(Antigen &antigen, const Competition &comp, Mab &mab,
                     const FromMesh &fm, const RandomFromMesh &rm) 
: _antigen(antigen), _mab(mab), _fromMesh(fm), _random(rm)
{
	OpSet<std::string> list;

	setIgnored(comp.naughty_corner);
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
	for (const std::string &naughty : comp.naughty_corner)
	{
		more -= naughty;
	}

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

	// room for those left out, to come back later without moving what is
	// already there (the watches read these from another thread)
	size_t extra = comp.naughty_corner.size() * _antigen.sym.transforms().size();
	_raw.reserve(_raw.size() + extra);
	_shown = _raw;
	_shown.reserve(_raw.capacity());
}

bool Positions::addAntibody(const std::string &name)
{
	if (_lookup.count(name))
	{
		return true;
	}

	Symmetry &sym = _antigen.sym;
	int n = sym.transforms().size();
	int i = _raw.size();
	if (_raw.size() + n > _raw.capacity() || _shown.size() + n > _shown.capacity())
	{
		return false;
	}

	AntibodyPos ap = {nullptr, name, i, n, sym, 0, nullptr};
	ap.mut = new std::mutex();
	_raw.resize(_raw.size() + n);
	ap.setPosition(_raw, _random(), _fromMesh);

	_shown.resize(_shown.size() + n);
	for (int k = 0; k < n; k++)
	{
		_shown[i + k] = _raw[i + k];
	}

	_positions.push_back(ap);
	_lookup[name] = &_positions.back();
	return true;
}

Positions::~Positions()
{
	for (AntibodyPos &pos : _positions)
	{
		delete pos.mut;
	}

}

static std::set<std::string> &ignoredNames()
{
	static std::set<std::string> names;
	return names;
}

static std::mutex &ignoredMutex()
{
	static std::mutex mutex;
	return mutex;
}

void Positions::setIgnored(const std::vector<std::string> &names)
{
	std::unique_lock<std::mutex> lock(ignoredMutex());
	ignoredNames() = std::set<std::string>(names.begin(), names.end());
}

bool Positions::ignored(const std::string &name)
{
	std::unique_lock<std::mutex> lock(ignoredMutex());
	return ignoredNames().count(name) > 0;
}

void Positions::randomise()
{
	for (AntibodyPos &ap : _positions)
	{
		if (ap.fixed || ignored(ap.name))
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
		loadOne(ap, bucket, watches);
	}
}

void Positions::loadAntibodyInto(HasRenderables *bucket,
                                 std::vector<AbWatch *> &watches,
                                 const std::string &name)
{
	if (_lookup.count(name))
	{
		loadOne(*_lookup.at(name), bucket, watches);
	}
}

void Positions::loadOne(const AntibodyPos &ap, HasRenderables *bucket,
                        std::vector<AbWatch *> &watches)
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

bool Positions::extent(const std::string &name, int &start, int &num) const
{
	auto it = _lookup.find(name);
	if (it == _lookup.end())
	{
		return false;
	}

	start = it->second->start_idx;
	num = it->second->num;
	return true;
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
		if (ignored(ap.name))
		{
			continue;
		}

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
		if (!_lookup.count(pl.name) || _lookup.at(pl.name)->fixed ||
		    ignored(pl.name))
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

namespace
{
	// which column goes with each row of a square cost matrix, for the least
	// total cost (the Hungarian algorithm)
	std::vector<size_t> best_matching(const std::vector<std::vector<double>> &a)
	{
		size_t n = a.size();
		const double inf = 1e300;
		std::vector<double> u(n + 1, 0.), v(n + 1, 0.);
		std::vector<size_t> p(n + 1, 0), way(n + 1, 0);

		for (size_t i = 1; i <= n; i++)
		{
			p[0] = i;
			size_t j0 = 0;
			std::vector<double> minv(n + 1, inf);
			std::vector<bool> used(n + 1, false);

			do
			{
				used[j0] = true;
				size_t i0 = p[j0], j1 = 0;
				double delta = inf;

				for (size_t j = 1; j <= n; j++)
				{
					if (used[j]) { continue; }
					double cur = a[i0 - 1][j - 1] - u[i0] - v[j];
					if (cur < minv[j]) { minv[j] = cur; way[j] = j0; }
					if (minv[j] < delta) { delta = minv[j]; j1 = j; }
				}

				for (size_t j = 0; j <= n; j++)
				{
					if (used[j]) { u[p[j]] += delta; v[j] -= delta; }
					else { minv[j] -= delta; }
				}

				j0 = j1;
			}
			while (p[j0] != 0);

			do
			{
				size_t j1 = way[j0];
				p[j0] = p[j1];
				j0 = j1;
			}
			while (j0);
		}

		std::vector<size_t> match(n, 0);
		for (size_t j = 1; j <= n; j++)
		{
			match[p[j] - 1] = j - 1;
		}

		return match;
	}
}

bool Positions::cluster(const std::vector<const std::vector<Placement> *> &all,
                        const std::string &name, Clusters &out)
{
	if (all.empty() || ignored(name))
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
	// result's copies so that cluster k stays with operator k. A result's
	// copies are the same antibody under each operator, so a result gives one
	// copy to each cluster, never two to one: each result's copies are dealt
	// out to the clusters by the best one-to-one matching (which copy goes to
	// which cluster, for the least total distance), not each to its nearest.
	// Left to go by the nearest alone, the clusters drift out of symmetry,
	// when a result's antibody sits far from where the first one's did,
	// and then some of its copies share a cluster while another has none.
	size_t n = first->copies.size();
	out.points.clear();
	out.resultOf.clear();
	std::vector<size_t> starts; // where each result's block of copies begins
	for (size_t r = 0; r < all.size(); r++)
	{
		for (const Placement &pl : *all[r])
		{
			if (pl.name == name && pl.copies.size() == n)
			{
				starts.push_back(out.points.size());
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

		for (size_t start : starts)
		{
			std::vector<std::vector<double>> cost(n, std::vector<double>(n));
			for (size_t i = 0; i < n; i++)
			{
				for (size_t k = 0; k < n; k++)
				{
					glm::vec3 diff = out.points[start + i] - out.centres[k];
					cost[i][k] = glm::dot(diff, diff);
				}
			}

			std::vector<size_t> match = best_matching(cost);
			for (size_t i = 0; i < n; i++)
			{
				size_t p = start + i;
				size_t best = match[i];
				changed |= (out.member[p] != best);
				out.member[p] = best;
				sums[best] += out.points[p];
				counts[best]++;
			}
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

std::vector<glm::vec3> Positions::medoids(const Clusters &clusters) const
{
	// the centre of a cluster is a mean, which is not generally a vertex
	// anyone sits on; its median is the vertex of the mesh with the least
	// total distance to every vertex that its members occupied, counting a
	// vertex as often as it was occupied. The best of the vertices need not
	// be one of the occupied ones.
	std::vector<glm::vec3> out = clusters.centres;
	size_t n = clusters.centres.size();

	for (size_t k = 0; k < n; k++)
	{
		// the occupied vertices, each once with how often it was occupied
		std::vector<std::pair<glm::vec3, float>> occupied;
		for (size_t p = 0; p < clusters.points.size(); p++)
		{
			if (clusters.member[p] != k)
			{
				continue;
			}

			bool seen = false;
			for (auto &entry : occupied)
			{
				if (glm::length(entry.first - clusters.points[p]) < 1e-3f)
				{
					entry.second += 1;
					seen = true;
					break;
				}
			}

			if (!seen)
			{
				occupied.push_back({clusters.points[p], 1.f});
			}
		}

		if (occupied.empty())
		{
			continue;
		}

		auto total = [&occupied](const glm::vec3 &from)
		{
			float sum = 0;
			for (const auto &entry : occupied)
			{
				sum += entry.second * glm::length(entry.first - from);
			}
			return sum;
		};

		// The occupied vertices themselves to begin with, so that this is
		// never worse than choosing from them alone.
		float best = FLT_MAX;
		for (const auto &entry : occupied)
		{
			float sum = total(entry.first);
			if (sum < best)
			{
				best = sum;
				out[k] = entry.first;
			}
		}

		// The point of least total distance lies within the box around the
		// occupied vertices, so only the vertices a little way around that
		// are looked at.
		glm::vec3 lo(FLT_MAX), hi(-FLT_MAX);
		for (const auto &entry : occupied)
		{
			lo = glm::min(lo, entry.first);
			hi = glm::max(hi, entry.first);
		}

		const float margin = 6.f;
		lo -= glm::vec3(margin);
		hi += glm::vec3(margin);

		for (const glm::vec3 &vertex : _meshVertices)
		{
			if (vertex.x < lo.x || vertex.x > hi.x || vertex.y < lo.y || 
			    vertex.y > hi.y || vertex.z < lo.z || vertex.z > hi.z)
			{
				continue;
			}

			float sum = total(vertex);
			if (sum < best)
			{
				best = sum;
				out[k] = vertex;
			}
		}
	}

	return out;
}

float Positions::silhouette(const Clusters &clusters)
{
	size_t n = clusters.centres.size();
	if (n < 2)
	{
		return 1.f;
	}

	if (clusters.points.empty())
	{
		return 0.f;
	}

	double total = 0;
	for (size_t p = 0; p < clusters.points.size(); p++)
	{
		size_t own = clusters.member[p];
		float a = glm::length(clusters.points[p] - clusters.centres[own]);
		float b = FLT_MAX;

		for (size_t k = 0; k < n; k++)
		{
			if (k != own)
			{
				b = std::min(b, glm::length(clusters.points[p] - 
				                            clusters.centres[k]));
			}
		}

		float biggest = std::max(a, b);
		total += (biggest > 0 ? (b - a) / biggest : 0.f);
	}

	float mean = total / clusters.points.size();
	return std::min(1.f, std::max(0.f, mean));
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
			setCopies(first.name, medoids(clusters));
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
			_lookup.at(first.name)->setShown(_shown, medoids(clusters),
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

float Positions::curvatureZ(const glm::vec3 &pos) const
{
	return (hasCurvature() ? curvatureZAtVertex(vertexAt(pos)) : 0.f);
}

void Positions::buildFootprints()
{
	_footprints.clear();
	if (_meshVertices.empty())
	{
		return;
	}

	const float reach = 5.f;
	bool computed = false;
	size_t n = _meshVertices.size();

	typedef std::array<int, 3> Cell;
	auto cell_of = [reach](const glm::vec3 &p)
	{
		return Cell{(int)std::floor(p.x / reach), (int)std::floor(p.y / reach),
		            (int)std::floor(p.z / reach)};
	};
	auto key_of = [](int x, int y, int z)
	{
		return ((int64_t)(x & 0x1FFFFF) << 42) | ((int64_t)(y & 0x1FFFFF) << 21)
		| (int64_t)(z & 0x1FFFFF);
	};

	for (AntibodyPos &ap : _positions)
	{
		if (!ap.fid || !ap.fid->contact)
		{
			continue;
		}

		ContactPoint *contact = ap.fid->contact;

		// the models are loaded for this once, and it is kept
		if (!contact->hasFootprint())
		{
			try
			{
				if (contact->computeFootprint())
				{
					ap.fid->alignment = contact->alignmentJson();
					computed = true;
				}
			}
			catch (const std::exception &err)
			{
				std::cout << "No footprint for antibody " << ap.name << ": "
				<< err.what() << std::endl;
			}
		}

		if (!contact->hasFootprint() || contact->orientations().empty())
		{
			continue;
		}

		// the atoms of the antibody where every orientation puts them
		std::unordered_map<int64_t, std::vector<glm::vec3>> grid;
		for (const ContactPoint::Orientation &o : contact->orientations())
		{
			for (const glm::vec3 &atom : contact->footprint())
			{
				glm::vec3 p = glm::vec3(o.transform * glm::vec4(atom, 1.f));
				Cell c = cell_of(p);
				grid[key_of(c[0], c[1], c[2])].push_back(p);
			}
		}

		// the vertices with one of them within reach
		std::vector<int> inside;
		glm::vec3 sum{};
		for (size_t v = 0; v < n; v++)
		{
			const glm::vec3 &pos = _meshVertices[v];
			Cell c = cell_of(pos);
			bool found = false;

			for (int x = -1; x <= 1 && !found; x++)
			for (int y = -1; y <= 1 && !found; y++)
			for (int z = -1; z <= 1 && !found; z++)
			{
				auto it = grid.find(key_of(c[0] + x, c[1] + y, c[2] + z));
				if (it == grid.end()) { continue; }

				for (const glm::vec3 &p : it->second)
				{
					glm::vec3 d = p - pos;
					if (glm::dot(d, d) <= reach * reach)
					{
						found = true;
						break;
					}
				}
			}

			if (found)
			{
				inside.push_back(v);
				sum += pos;
			}
		}

		if (inside.empty())
		{
			continue;
		}

		struct ReturnPos
		{
			ReturnPos(const std::vector<int> &list, 
			          const std::vector<glm::vec3> &all)
			: _list(list), _all(all) {}

			bool operator()(size_t &idx, glm::vec3 &v)
			{
				if (n >= _list.size()) return false;
				idx = n;
				v = _all[_list[n]];
				n++;
				return true;
			};

			const std::vector<int> &_list;
			const std::vector<glm::vec3> &_all;
			size_t n = 0;
		};

		KdTree tree(ReturnPos(inside, _meshVertices));

		Footprint footprint;
		footprint.count = inside.size();
		footprint.centroid = sum / (float)inside.size();
		footprint.dist.resize(n);
		footprint.nearest.resize(n);
		for (size_t v = 0; v < n; v++)
		{
			int which = tree.nearestVertex(_meshVertices[v]);
			int vertex = inside[std::max(0, which)];
			footprint.nearest[v] = vertex;
			footprint.dist[v] = glm::length(_meshVertices[v] - 
			                                _meshVertices[vertex]);
		}

		std::cout << "Footprint of " << ap.name << ": " << footprint.count 
		<< " of " << n << " mesh vertices" << std::endl;
		_footprints[ap.name] = footprint;
	}

	// the footprints worked out are kept with the alignments
	if (computed)
	{
		_mab.save();
	}
}

std::vector<int> Positions::footprintVertices(const std::string &name) const
{
	std::vector<int> vertices;
	const Footprint *footprint = footprintOf(name);
	if (!footprint)
	{
		return vertices;
	}

	for (size_t v = 0; v < footprint->dist.size(); v++)
	{
		if (footprint->dist[v] <= 0.f)
		{
			vertices.push_back(v);
		}
	}

	return vertices;
}

int Positions::vertexAt(const glm::vec3 &pos) const
{
	if (!_fromMesh)
	{
		return -1;
	}

	glm::vec3 where = pos;
	int idx = -1;
	_fromMesh(pos, where, idx);
	return idx;
}

bool Positions::footprintDistance(const Footprint &footprint, 
                                  const glm::vec3 &pos, int vertex, 
                                  float &dist, glm::vec3 &away) const
{
	if (vertex < 0 || vertex >= (int)footprint.dist.size())
	{
		return false;
	}

	glm::vec3 from = _meshVertices[footprint.nearest[vertex]];
	glm::vec3 diff = pos - from;
	bool on = (footprint.dist[vertex] <= 0.f);

	if (on)
	{
		diff = pos - footprint.centroid;
		dist = 0.f;
	}
	else
	{
		dist = glm::length(diff);
	}

	float length = glm::length(diff);
	away = (length > 1e-6f ? diff / length : glm::vec3(0.f, 0.f, 1.f));
	return true;
}

bool Positions::footprintDistance(const std::string &name, 
                                  const glm::vec3 &pos, float &dist, 
                                  glm::vec3 &away) const
{
	const Footprint *footprint = footprintOf(name);
	return (footprint && footprintDistance(*footprint, pos, vertexAt(pos), dist,
	                                       away));
}

bool Positions::isFixed(const std::string &name) const
{
	return _lookup.at(name)->fixed;
}

std::vector<std::string> Positions::fixedAntibodies() const
{
	std::vector<std::string> names;
	for (const AntibodyPos &ap : _positions)
	{
		if (ap.fid && ap.fixed)
		{
			names.push_back(ap.name);
		}
	}

	return names;
}

void Positions::setFixedAntibodies(const std::vector<std::string> &fixed)
{
	for (const std::string &name : structuralAntibodies())
	{
		setFixed(name, std::find(fixed.begin(), fixed.end(), name) != 
		         fixed.end());
	}
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

Fiducial *Positions::fiducialFor(const std::string &name) const
{
	if (!_lookup.count(name))
	{
		return nullptr;
	}

	return _lookup.at(name)->fid;
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

	// The fiducial's own list of symmetry operators may be shorter than the
	// antigen's (an orientation it did not find), so the copies which the
	// antigen's operators make of the structural position are there too, for
	// the nearest to be nearest of all. Those already found are not repeated.
	Symmetry antigenSym = _antigen.sym;
	glm::vec3 other = sym.reference();
	antigenSym.reset();
	for (const glm::mat4x4 &transform : antigenSym.transforms())
	{
		antigenSym.applyTransform(transform, Symmetry::next_pointer(&other));

		glm::vec3 where = other;
		int idx = -1;
		if (snapped && _fromMesh)
		{
			_fromMesh(other, where, idx);
		}

		bool known = false;
		for (const glm::vec3 &copy : copies)
		{
			known |= (glm::length(copy - where) < 0.5f);
		}

		if (!known)
		{
			copies.push_back(where);
		}
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
