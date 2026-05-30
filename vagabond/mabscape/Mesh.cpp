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

#include "Mesh.h"
#include "Mab.h"
#include <vagabond/gui/GuiDensity.h>
#include <vagabond/core/GroupBounds.h>
#include <vagabond/core/AtomGroup.h>
#include <vagabond/core/matrix_functions.h>
#include <vagabond/utils/KdTree.h>

Mesh::Mesh(Antigen &antigen) : _antigen(antigen)
{
	setUsesProjection(true);
	setVertexShaderFile("assets/shaders/with_matrix.vsh");
	setFragmentShaderFile("assets/shaders/color_only.fsh");
	marchingCubes();
}

AtomGroup *Mesh::atoms()
{
	_antigen.model.load();
	if (!_atoms)
	{
		std::vector<Instance *> antigen_only = _antigen.instances();

		_atoms = new AtomGroup();
		for (Instance *const &inst : antigen_only)
		{
			_atoms->add(inst->currentAtoms());
		}
	}

	_antigen.model.unload();
	return _atoms;
}

ArbitraryMap Mesh::mappedAtoms()
{
	GroupBounds bounds(atoms());
	bounds.min -= glm::vec3(5);
	bounds.max += glm::vec3(5);
	glm::vec3 size = bounds.max - bounds.min;
	size *= 1.5;

	ArbitraryMap map;
	glm::mat3x3 mat = glm::mat3x3(1.f);
	map.setDimensions(size.x, size.y, size.z, true);
	mat[0][0] = map.nx();
	mat[1][1] = map.ny();
	mat[2][2] = map.nz();
	map.setOrigin(bounds.min);
	map.setRealMatrix(mat);
	map.clear();

	auto add_one = [&map](glm::vec3 real)
	{
		map.real2Voxel(real);
		
		for (int k = 0; k < 1; k++)
		for (int j = 0; j < 1; j++)
		for (int i = 0; i < 1; i++)
		{
			long idx = map.index(real.x + i, real.y + j, real.z + k);
			map.setReal(idx, 1);
		}
	};

	atoms()->do_op
	([add_one](Atom *const &atom)
	 {
		glm::vec3 me = atom->initialPosition();
		if (atom->elementSymbol() == "H") { return; }
		add_one(me);
		for (int i = 0; i < atom->bondLengthCount(); i++)
		{
			Atom *other = atom->connectedAtom(i);
			if (other->elementSymbol() == "H") { return; }
			glm::vec3 you = other->initialPosition();
			add_one((me + you) / 2.f);

		}
	});

	return map;
}

int Mesh::removeHollows(ArbitraryMap &map)
{
	// e.g. (1, 1, 0) will loop through xy plane
	auto pass_through_section = [&map]<class Func>
	(int xbeg, int ybeg, int zbeg,
	 int xstep, int ystep, int zstep, Func &job)
	{
		int nx = map.nx(); int ny = map.ny(); int nz = map.nz();
		int zend = zstep * nz + 1 + zbeg;
		int yend = ystep * ny + 1 + ybeg;
		int xend = xstep * nx + 1 + xbeg;
		if (zend > nz) zend = nz;
		if (yend > ny) yend = ny;
		if (xend > nx) xend = nx;
		zstep = 1; ystep = 1; xstep = 1;

		for (int k = zbeg; k < zend; k += zstep)
		{
			for (int j = ybeg; j < yend; j += ystep)
			{
				for (int i = xbeg; i < xend; i += xstep)
				{
					job(i, j, k);
				}
			}
		}
	};

	auto pass_through_plane = [&pass_through_section]<class Func>
	(int x, int y, int z, Func &job)
	{
		pass_through_section(0, 0, 0, x, y, z, job);
	};

	auto pass_through_line = [&pass_through_section]<class Func>
	(int i, int j, int k, int x, int y, int z, Func &job)
	{
		pass_through_section(i, j, k, x, y, z, job);
		job(-1, -1, -1);
	};
	
	auto plane_line_pass = [&pass_through_plane, 
	                        &pass_through_line](int x, int y, int z)
	{
		return [&pass_through_line, &pass_through_plane, x, y, z]
		<class Func>(Func &job)
		{
			auto line_for = [&pass_through_line]
			(int x, int y, int z, Func &job)
			{
				return [&pass_through_line, x, y, z, &job]
				(int i, int j, int k)
				{
					pass_through_line(i, j, k, 1-x, 1-y, 1-z, job);
				};
			};

			auto line = line_for(x, y, z, job);
			pass_through_plane(x, y, z, line);
		};
	};
	
	struct remove_runs
	{
		remove_runs(ArbitraryMap &map) : _map(map) {}
		
		std::vector<long int> list;

		int N = 4;
		int zero_run = -1;
		ArbitraryMap &_map;
		int changed = 0;

		void operator()(int x, int y, int z)
		{
			if (x < 0 && y < 0 && z < 0)
			{
				zero_run = -1; list.clear();
				return;
			}

			long idx = _map.index(x, y, z);
			float val = _map.elementValue(idx);

			if (val > 0.5 && zero_run < 0)
			{
				zero_run = 0;
			}
			else if (val <= 0.5 && zero_run >= 0)
			{
				list.push_back(idx);
				zero_run++;
			}
			else if (val > 0.5 && zero_run >= 0 && zero_run <= N)
			{
				for (const long int &idx : list)
				{
					_map.setReal(idx, 1);
					changed++;
				}
				zero_run = 0;
				list.clear();
			}
			else if (val > 0.5 && zero_run > N)
			{
				zero_run = 0;
				list.clear();
			}
		}
	};
	
	remove_runs remove_runs_from_map(map);
	
	plane_line_pass(1, 1, 0)(remove_runs_from_map);
	plane_line_pass(0, 1, 1)(remove_runs_from_map);
	plane_line_pass(1, 0, 1)(remove_runs_from_map);

	std::cout << "Hollows removed: " <<  remove_runs_from_map.changed
	<< std::endl;
	return remove_runs_from_map.changed;
}

void Mesh::growOrShrinkBorder(ArbitraryMap &map, int dir)
{
	for (int i = 0; i < map.nn(); i++)
	{
		map.element(i)[1] = map.element(i)[0];
	}
	
	auto check_neighbours = [&map, dir](int x, int y, int z)
	{
		long idx = map.index(x, y, z);
		for (int k = -1; k <= 1; k++)
		{
			for (int j = -1; j <= 1; j++)
			{
				for (int i = -1; i <= 1; i++)
				{
					long check = map.index(x + i, y + j, z + k);
					float val = map.element(check)[1];
					if (dir ? (val > 0.5) : (val < 0.5))
					{
						map.setReal(idx, 1 - dir);
						return;
					}
				}
			}
		}
	};

	for (int k = 0; k < map.nz(); k++)
	{
		for (int j = 0; j < map.ny(); j++)
		{
			for (int i = 0; i < map.nx(); i++)
			{
				long check = map.index(i, j, k);
				if (dir ? (map.elementValue(check) > 0.5) :
				    (map.elementValue(check) < 0.5)) continue;
				check_neighbours(i, j, k);
			}
		}
	}

	for (int i = 0; i < map.nn(); i++)
	{
		map.element(i)[1] = 0;
	}
}

float Mesh::adjustVertices(bool planar)
{
	// must be in lines
	for (int i = 0; i < indices().size(); i += 2) 
	{
		_connections[indices()[i]] += indices()[i + 1];
		_connections[indices()[i + 1]] += indices()[i];
	}

	std::vector<Vertex> &copy = vertices();
	std::vector<Vertex> edit = copy;
	
	auto average = [&copy](const OpSet<GLuint> &neighbours)
	{
		glm::vec3 sum{};
		for (const GLuint &g : neighbours)
		{
			sum += copy[g].pos;
		}
		sum /= (float)neighbours.size();
		return sum;
	};

	auto nudged = [&copy](const OpSet<GLuint> &neighbours,
	                      const glm::vec3 &orig)
	{
		std::vector<glm::vec3> bits; bits.reserve(neighbours.size());
		std::vector<float> radii;
		float ave_radius{};
		glm::vec3 ave_bit{};

		for (const GLuint &idx : neighbours)
		{
			const glm::vec3 &pos = copy[idx].pos;
			glm::vec3 diff = pos - orig;
			float length = glm::length(diff);
			radii.push_back(length);
			ave_radius += length;
			float multiplier = 0.5 / length;
			glm::vec3 contrib{};
			for (int i = 0; i < 3; i++)
			{
				contrib[i] = -2 * diff[i] * orig[i];
			}
			contrib *= multiplier;
			if (contrib != contrib)
			{
				contrib = {};
			}
			bits.push_back(contrib);
			ave_bit += contrib;
		}
		ave_radius /= (float)neighbours.size();
		ave_bit /= (float)neighbours.size();

		glm::vec3 grad{};
		
		for (int j = 0; j < 3; j++)
		{
			for (int i = 0; i < radii.size(); i++)
			{
				float first = radii[i] - ave_radius;
				float second = bits[i][j] - ave_bit[j];
				grad[j] += -2 * first * second / (float)radii.size();
			}
		}
		
		return orig + grad * 0.001f;
	};
	
	float all_changes = {};
	for (int i = 0; i < copy.size(); i++)
	{
		const OpSet<GLuint> &neighbours = _connections[i];
		glm::vec3 orig = copy[i].pos;
		glm::vec3 target = (planar ? nudged(neighbours, orig) 
		                    : average(neighbours));
		glm::vec3 diff = target - orig;
		glm::vec3 &norm = copy[i].normal;
		float curvature = glm::dot(diff, norm);
		glm::vec3 plane = diff - norm * curvature;
		glm::vec3 mod = {};
		if (!planar && curvature > 0)
		{
			mod += diff;
		}
		else if (planar)
		{
			mod += plane;
		}
		mod *= (planar ? 0.03f : 0.01f);
		edit[i].pos += mod;
		all_changes += glm::dot(mod, mod);
	}
	all_changes = sqrt(all_changes / (float)copy.size());

	std::unique_lock<std::mutex> lock(_vertLock);
	vertices() = edit;
	forceRender(true, true);
	return all_changes;
}

void Mesh::calculateNormalsFrom(const std::vector<GLuint> &indices)
{
	std::unique_lock<std::mutex> lock(_vertLock);
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		_vertices[i].normal = glm::vec3(0.);
	}
	
	for (size_t i = 0; i < indices.size(); i += 3)
	{
		glm::vec3 &pos1 = _vertices[indices[i+0]].pos;
		glm::vec3 &pos2 = _vertices[indices[i+1]].pos;
		glm::vec3 &pos3 = _vertices[indices[i+2]].pos;

		glm::vec3 diff31 = pos3 - pos1;
		glm::vec3 diff21 = pos2 - pos1;

		glm::vec3 cross = glm::cross(diff31, diff21);
		cross = glm::normalize(cross);
		
		if (!is_glm_vec_sane(cross))
		{
			continue;
		}
		
		/* Normals */					
		for (int j = 0; j < 3; j++)
		{
			_vertices[indices[i + j]].normal += cross;
		}
	}

	for (size_t i = 0; i < _vertices.size(); i++)
	{
		glm::vec3 &norm = _vertices[i].normal;
		norm = glm::normalize(norm);
	}
}

void Mesh::normalsFromLines()
{
	calculateNormalsFrom(_triangleIdxs);
}

void Mesh::marchingCubes()
{
	ArbitraryMap map = mappedAtoms();
	for (int i = 0; i < 2; i++)
	{
		growOrShrinkBorder(map, 1);
	}
		growOrShrinkBorder(map, -1);
		growOrShrinkBorder(map, -1);
	growOrShrinkBorder(map, 1);
	
	// loop until no more hollows
	while (removeHollows(map) > 0) {};
	
	GuiDensity density;
	density.setThreshold(0.5);
	density.setStep(2.0);
	density.populateFromMap(&map);
	copyFrom(&density);
	calculateNormals();

	_triangleIdxs = indices();
	changeToLines();
	_lineIdxs = indices();
}

void Mesh::refine()
{
	auto loop = [this](bool planar)
	{
		while (true)
		{
			float result = adjustVertices(planar);
			if (result < 5e-04)
			{
				break;
			}
			normalsFromLines();
		}
	};

	loop(true);
	loop(false);
}

void Mesh::kdTree()
{
	if (_tree)
	{
		return;
	}

	struct ReturnPos
	{
		ReturnPos(std::vector<Vertex> &list)
		: _list(list) {}
		
		bool operator()(size_t &idx, glm::vec3 &v)
		{
			if (n >= _list.size()) return false;
			idx = n;
			v = _list[n].pos;
			n++;
			return true;
		};

		std::vector<Vertex> &_list;
		int n = 0;
	};

	_tree = new KdTree(ReturnPos(_vertices));
}

int Mesh::nearestVertex(const glm::vec3 &v)
{
	kdTree();

	return _tree->nearestVertex(v);
}

std::function<void(const glm::vec3 &, glm::vec3 &, int &)> 
Mesh::vertexFinder()
{
	return [this](const glm::vec3 &t, glm::vec3 &p, int &i)
	{
		i = nearestVertex(t);
		p = _vertices[i].pos;
	};

}

