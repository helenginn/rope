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
#include <vagabond/gui/ColourScheme.h>
#include <vagabond/core/GroupBounds.h>
#include <vagabond/core/AtomGroup.h>
#include <vagabond/core/matrix_functions.h>
#include <vagabond/utils/KdTree.h>
#include <vagabond/utils/FileReader.h>
#include <vagabond/utils/glm_json.h>
#include <fstream>
#include <unordered_map>
#include <cmath>
#include <array>
#include <cfloat>
#include <algorithm>

Mesh::Mesh(Antigen &antigen) : _antigen(antigen)
{
	setUsesProjection(true);
	setVertexShaderFile("assets/shaders/with_matrix.vsh");
	setFragmentShaderFile("assets/shaders/color_only.fsh");

	if (!loadCache())
	{
		marchingCubes();
	}
}

std::string Mesh::cachePath()
{
	std::string source = _antigen.model.filename();
	if (source.length() == 0)
	{
		return "";
	}

	return source + ".mesh_cache.json";
}

bool Mesh::loadCache()
{
	std::string source = _antigen.model.filename();
	std::string path = cachePath();
	if (path.length() == 0 || !file_exists(path))
	{
		return false;
	}

	std::ifstream file(path);
	if (!file)
	{
		return false;
	}

	json data;
	try
	{
		file >> data;
	}
	catch (const std::exception &e)
	{
		std::cout << "Could not parse mesh cache " << path << ": "
		<< e.what() << std::endl;
		return false;
	}

	std::string cached_source = data.value("source_filename", "");
	time_t cached_mtime = data.value("source_mtime", (time_t)0);

	if (cached_source != source || cached_mtime != file_mod_time(source))
	{
		std::cout << "Mesh cache for " << source
		<< " is out of date, recalculating" << std::endl;
		return false;
	}

	std::vector<Vertex> verts;
	verts.reserve(data["vertices"].size());
	for (const json &v : data["vertices"])
	{
		Vertex vertex{};
		vertex.pos = v.at("pos").get<glm::vec3>();
		vertex.normal = v.at("normal").get<glm::vec3>();
		vertex.color = glm::vec4(0.5, 0.5, 0.8, 1.0);
		verts.push_back(vertex);
	}

	_vertices = verts;
	_indices = data["indices"].get<std::vector<GLuint>>();

	// marchingCubes() leaves the live (uncached) mesh in GL_LINES mode -
	// refine()'s triangulate() call is a no-op in that mode (it bails
	// out immediately if _renderType == GL_LINES), so the cached
	// _indices are still line pairs, not a triangle list. Without this,
	// _renderType defaults to GL_TRIANGLES and those line pairs get
	// misinterpreted as (garbled, opaque) triangles.
	_renderType = GL_LINES;

	forceRender(true, true);
	_loadedFromCache = true;

	std::cout << "Loaded cached mesh for " << source << std::endl;
	return true;
}

void Mesh::saveCache()
{
	std::string source = _antigen.model.filename();
	std::string path = cachePath();
	if (path.length() == 0)
	{
		return;
	}

	json data;
	data["source_filename"] = source;
	data["source_mtime"] = file_mod_time(source);

	json verts = json::array();
	for (const Vertex &v : _vertices)
	{
		json vj;
		vj["pos"] = v.pos;
		vj["normal"] = v.normal;
		verts.push_back(vj);
	}
	data["vertices"] = verts;
	data["indices"] = _indices;

	std::ofstream file;
	file.open(path);
	if (!file)
	{
		std::cout << "Could not write mesh cache to " << path << std::endl;
		return;
	}
	file << data.dump(1);
	file << std::endl;
	file.close();
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

void Mesh::buildDensity(GuiDensity &density)
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
	
	density.setThreshold(0.5);
	density.setStep(2.0);
	density.populateFromMap(&map);
}

void Mesh::marchingCubes()
{
	GuiDensity density;
	buildDensity(density);
	copyFrom(&density);
	calculateNormals();

	_triangleIdxs = indices();
	changeToLines();
	_lineIdxs = indices();
}

void Mesh::reset()
{
	// the slow part first, while the old mesh stays as it was (and drawn)
	GuiDensity density;
	buildDensity(density);

	std::string path = cachePath();
	if (path.length() && file_exists(path))
	{
		std::remove(path.c_str());
	}

	{
		std::unique_lock<std::mutex> lock(_excludeMutex);
		_excluded.clear();
	}
	_selected.clear();
	_baseColours.clear();
	_connections.clear();
	_outward = 0.f;
	delete _tree;
	_tree = nullptr;
	_loadedFromCache = false;
	_refined = false;

	// then the swap, which is quick: the new mesh arrives as triangles, as it
	// did to begin with, and is put into lines
	{
		std::unique_lock<std::mutex> lock(_vertLock);
		_renderType = GL_TRIANGLES;
		copyFrom(&density);
		calculateNormals(true);
		_triangleIdxs = indices();
		changeToLines();
		_lineIdxs = indices();
		forceRender(true, true);
	}

	invalidateExclusions();
	invalidateCurvature();
}

void Mesh::refine(bool force)
{
	if (!force && refined())
	{
		return;
	}

	// a mesh from the cache has only its lines, which hold the triangles
	if (_triangleIdxs.empty())
	{
		_triangleIdxs = triangles();
	}

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
	triangulate();

	saveCache();
	_refined = true;
	invalidateCurvature();
}

namespace
{
	/** The protein as the union of its heavy atoms at their van der Waals
	 *  radii. Atoms are held in a hash grid of cells two Angstroms wide, so
	 *  that nothing further than that is looked at. */
	class ProteinField
	{
	public:
		ProteinField(Antigen &antigen)
		{
			antigen.model.load();
			std::vector<Instance *> antigen_only = antigen.instances();
			AtomGroup group;
			for (Instance *const &inst : antigen_only)
			{
				group.add(inst->currentAtoms());
			}

			group.do_op([this](Atom *const &atom)
			{
				std::string element = atom->elementSymbol();
				if (element == "H") { return; }
				float radius = 1.7f;
				if (element == "N") { radius = 1.55f; }
				else if (element == "O") { radius = 1.52f; }
				else if (element == "S") { radius = 1.8f; }
				Cell c = cell_of(atom->initialPosition());
				_grid[key_of(c[0], c[1], c[2])].push_back
				({atom->initialPosition(), radius});
			});
			antigen.model.unload();
		}

		// how far this point is outside the nearest atom's surface (negative
		// if inside it), reported as no more than cellSize
		float clearance(const glm::vec3 &p) const
		{
			Cell c = cell_of(p);
			float best = cellSize;
			for (int i = -1; i <= 1; i++)
			for (int j = -1; j <= 1; j++)
			for (int k = -1; k <= 1; k++)
			{
				auto it = _grid.find(key_of(c[0] + i, c[1] + j, c[2] + k));
				if (it == _grid.end()) { continue; }
				for (const Sphere &sphere : it->second)
				{
					best = std::min(best, 
					                glm::length(sphere.pos - p) - sphere.radius);
				}
			}
			return best;
		}

		/** moves from here to there, as far as it can go (a full move, half,
		 *  a quarter, or not at all) without going into the protein or
		 *  deeper into it than here already was. */
		glm::vec3 allowedMove(const glm::vec3 &from, const glm::vec3 &move,
		                      float &fraction_out) const
		{
			float allowed = std::min(0.f, clearance(from)) - 1e-4f;
			float fraction = 1.f;
			for (int attempt = 0; attempt < 3; attempt++, fraction *= 0.5f)
			{
				glm::vec3 target = from + move * fraction;
				if (clearance(target) >= allowed)
				{
					fraction_out = fraction;
					return target;
				}
			}

			fraction_out = 0.f;
			return from;
		}

	private:
		static constexpr float cellSize = 2.f;
		struct Sphere { glm::vec3 pos; float radius; };
		typedef std::array<int, 3> Cell;

		static Cell cell_of(const glm::vec3 &p)
		{
			return Cell{(int)std::floor(p.x / cellSize),
			            (int)std::floor(p.y / cellSize),
			            (int)std::floor(p.z / cellSize)};
		}

		static int64_t key_of(int x, int y, int z)
		{
			return ((int64_t)(x & 0x1FFFFF) << 42) 
			| ((int64_t)(y & 0x1FFFFF) << 21) | (int64_t)(z & 0x1FFFFF);
		}

		std::unordered_map<int64_t, std::vector<Sphere>> _grid;
	};
}

void Mesh::shrinkWrap(float volumeWeight)
{
	const int maxIterations = 10000;
	const float maxStep = 0.2f;
	const float tolerance = 1e-3f;

	if (_vertices.empty())
	{
		return;
	}

	ProteinField field(_antigen);
	auto clearance = [&field](const glm::vec3 &p)
	{
		return field.clearance(p);
	};

	std::vector<GLuint> tris = triangles();
	size_t n = _vertices.size();
	std::vector<glm::vec3> pos(n);
	for (size_t i = 0; i < n; i++)
	{
		pos[i] = _vertices[i].pos;
	}

	// a typical edge length sets how big a step the surface tension allows
	double edge_sum = 0; size_t edge_count = 0;
	for (size_t t = 0; t + 3 <= tris.size(); t += 3)
	{
		for (int j = 0; j < 3; j++)
		{
			edge_sum += glm::length(pos[tris[t + j]] - pos[tris[t + (j + 1) % 3]]);
			edge_count++;
		}
	}
	double h = (edge_count ? edge_sum / edge_count : 1.);
	// the surface tension sets how big a step is stable
	double w = volumeWeight;
	double dt = 0.1 * h * h;

	std::vector<glm::dvec3> gradArea(n), gradVol(n);
	std::vector<double> vertArea(n);
	int iterations = 0;

	for (; iterations < maxIterations && !_stop; iterations++)
	{
		// positions relative to the middle keep the volume sums accurate
		glm::dvec3 middle{};
		for (const glm::vec3 &p : pos) { middle += glm::dvec3(p); }
		middle /= (double)n;

		std::fill(gradArea.begin(), gradArea.end(), glm::dvec3(0.));
		std::fill(gradVol.begin(), gradVol.end(), glm::dvec3(0.));
		std::fill(vertArea.begin(), vertArea.end(), 0.);
		double volume = 0;

		for (size_t t = 0; t + 3 <= tris.size(); t += 3)
		{
			GLuint ia = tris[t], ib = tris[t + 1], ic = tris[t + 2];
			glm::dvec3 a = glm::dvec3(pos[ia]) - middle;
			glm::dvec3 b = glm::dvec3(pos[ib]) - middle;
			glm::dvec3 c = glm::dvec3(pos[ic]) - middle;

			volume += glm::dot(a, glm::cross(b, c)) / 6.;
			gradVol[ia] += glm::cross(b, c) / 6.;
			gradVol[ib] += glm::cross(c, a) / 6.;
			gradVol[ic] += glm::cross(a, b) / 6.;

			glm::dvec3 normal = glm::cross(b - a, c - a);
			double twice = glm::length(normal);
			if (twice < 1e-12) { continue; }
			normal /= twice;

			gradArea[ia] += 0.5 * glm::cross(normal, c - b);
			gradArea[ib] += 0.5 * glm::cross(normal, a - c);
			gradArea[ic] += 0.5 * glm::cross(normal, b - a);
			vertArea[ia] += twice / 6.;
			vertArea[ib] += twice / 6.;
			vertArea[ic] += twice / 6.;
		}

		// whichever way the triangles wind, shrinking is to reduce the volume
		double sign = (volume >= 0 ? 1. : -1.);
		double biggest = 0;
		std::vector<glm::vec3> next = pos;

		for (size_t i = 0; i < n; i++)
		{
			if (vertArea[i] < 1e-9) { continue; }
			
			glm::dvec3 grad = gradArea[i] + w * sign * gradVol[i];
			glm::dvec3 move = -grad * (dt / vertArea[i]);
			double length = glm::length(move);
			if (length > maxStep) { move *= maxStep / length; }
			if (!(length == length)) { continue; }

			// the hard stop: never into the protein, nor deeper into it than
			// it started
			float allowed = std::min(0.f, clearance(pos[i])) - 1e-4f;
			float fraction = 1.f;
			for (int attempt = 0; attempt < 3; attempt++, fraction *= 0.5f)
			{
				glm::vec3 target = pos[i] + glm::vec3(move) * fraction;
				if (clearance(target) >= allowed)
				{
					next[i] = target;
					biggest = std::max(biggest, 
					                   (double)glm::length(move) * fraction);
					break;
				}
			}
		}

		pos = next;

		if (iterations % 10 == 0 || biggest < tolerance)
		{
			std::unique_lock<std::mutex> lock(_vertLock);
			for (size_t i = 0; i < n; i++)
			{
				_vertices[i].pos = pos[i];
			}
			forceRender(true, true);
		}

		if (biggest < tolerance)
		{
			break;
		}
	}

	// (stopped early or not, what there is goes on show)
	{
		std::unique_lock<std::mutex> lock(_vertLock);
		for (size_t i = 0; i < n; i++)
		{
			_vertices[i].pos = pos[i];
		}
		forceRender(true, true);
	}

	std::cout << "Shrink wrap (volume weight " << volumeWeight << ") stopped after "
	<< iterations << " iterations" << std::endl;

	_triangleIdxs = tris;
	calculateNormalsFrom(_triangleIdxs);
	_connections.clear();
	delete _tree;
	_tree = nullptr;
	invalidateExclusions();
	invalidateCurvature();

	saveCache();
	_refined = true;
}

void Mesh::evenOut()
{
	const int maxIterations = 500;
	const float tolerance = 1e-3f;
	const float factor = 0.5f;

	if (_vertices.empty())
	{
		return;
	}

	ProteinField field(_antigen);
	std::vector<GLuint> tris = triangles();
	size_t n = _vertices.size();

	std::vector<std::vector<GLuint>> neighbours(n);
	for (size_t t = 0; t + 3 <= tris.size(); t += 3)
	{
		for (int j = 0; j < 3; j++)
		{
			GLuint a = tris[t + j], b = tris[t + (j + 1) % 3];
			neighbours[a].push_back(b);
			neighbours[b].push_back(a);
		}
	}
	for (std::vector<GLuint> &list : neighbours)
	{
		std::sort(list.begin(), list.end());
		list.erase(std::unique(list.begin(), list.end()), list.end());
	}

	std::vector<glm::vec3> pos(n);
	for (size_t i = 0; i < n; i++)
	{
		pos[i] = _vertices[i].pos;
	}

	int iterations = 0;
	for (; iterations < maxIterations && !_stop; iterations++)
	{
		// the way the surface faces at each vertex (whichever way up)
		std::vector<glm::vec3> normals(n, glm::vec3(0.f));
		for (size_t t = 0; t + 3 <= tris.size(); t += 3)
		{
			glm::vec3 cross = glm::cross(pos[tris[t + 1]] - pos[tris[t]],
			                             pos[tris[t + 2]] - pos[tris[t]]);
			for (int j = 0; j < 3; j++)
			{
				normals[tris[t + j]] += cross;
			}
		}

		float biggest = 0;
		std::vector<glm::vec3> next = pos;
		for (size_t i = 0; i < n; i++)
		{
			if (neighbours[i].empty()) { continue; }

			glm::vec3 average{};
			for (GLuint other : neighbours[i]) { average += pos[other]; }
			average /= (float)neighbours[i].size();

			// only along the surface: what lies across it is left alone
			glm::vec3 move = (average - pos[i]) * factor;
			float length = glm::length(normals[i]);
			if (length > 1e-12f)
			{
				glm::vec3 normal = normals[i] / length;
				move -= normal * glm::dot(move, normal);
			}
			if (!is_glm_vec_sane(move)) { continue; }

			float fraction = 0;
			next[i] = field.allowedMove(pos[i], move, fraction);
			biggest = std::max(biggest, glm::length(move) * fraction);
		}

		pos = next;

		if (iterations % 10 == 0)
		{
			std::unique_lock<std::mutex> lock(_vertLock);
			for (size_t i = 0; i < n; i++)
			{
				_vertices[i].pos = pos[i];
			}
			forceRender(true, true);
		}

		if (biggest < tolerance)
		{
			break;
		}
	}

	{
		std::unique_lock<std::mutex> lock(_vertLock);
		for (size_t i = 0; i < n; i++)
		{
			_vertices[i].pos = pos[i];
		}
		forceRender(true, true);
	}

	std::cout << "Even out stopped after " << iterations << " iterations" 
	<< std::endl;

	_triangleIdxs = tris;
	calculateNormalsFrom(_triangleIdxs);
	_connections.clear();
	delete _tree;
	_tree = nullptr;
	invalidateExclusions();
	invalidateCurvature();

	saveCache();
	_refined = true;
}

namespace
{
	// squared distance (in voxels) to the nearest voxel where f is 0 (the
	// others holding a big number), along each axis in turn
	// (Felzenszwalb & Huttenlocher)
	void edt_line(const float *f, int n, float *d, int *v, float *z)
	{
		const float inf = 1e12f;
		int k = 0;
		v[0] = 0;
		z[0] = -inf;
		z[1] = inf;
		for (int q = 1; q < n; q++)
		{
			auto cross = [&](int from)
			{
				return ((f[q] + (float)q * q) - (f[from] + (float)from * from)) /
				(2.f * q - 2.f * from);
			};

			float s = cross(v[k]);
			while (s <= z[k])
			{
				k--;
				s = cross(v[k]);
			}
			k++;
			v[k] = q;
			z[k] = s;
			z[k + 1] = inf;
		}

		k = 0;
		for (int q = 0; q < n; q++)
		{
			while (z[k + 1] < q) { k++; }
			d[q] = ((float)q - v[k]) * ((float)q - v[k]) + f[v[k]];
		}
	}

	void edt3d(std::vector<float> &g, int nx, int ny, int nz)
	{
		int m = std::max(nx, std::max(ny, nz));
		std::vector<float> line(m), out(m), z(m + 1);
		std::vector<int> v(m);

		for (int axis = 0; axis < 3; axis++)
		{
			int n = (axis == 0 ? nx : (axis == 1 ? ny : nz));
			int na = (axis == 0 ? ny : nx);
			int nb = (axis == 2 ? ny : nz);
			for (int a = 0; a < na; a++)
			for (int b = 0; b < nb; b++)
			{
				auto index = [&](int i)
				{
					int x = (axis == 0 ? i : a);
					int y = (axis == 1 ? i : (axis == 0 ? a : b));
					int zz = (axis == 2 ? i : b);
					return ((size_t)zz * ny + y) * nx + x;
				};

				for (int i = 0; i < n; i++) { line[i] = g[index(i)]; }
				edt_line(line.data(), n, out.data(), v.data(), z.data());
				for (int i = 0; i < n; i++) { g[index(i)] = out[i]; }
			}
		}
	}
}

void Mesh::closeCycle(float radius)
{
	if (_vertices.empty() || radius < 0.05f)
	{
		return;
	}

	std::vector<GLuint> tris = triangles();
	size_t n = _vertices.size();
	std::vector<glm::vec3> pos(n);
	glm::vec3 lo(FLT_MAX), hi(-FLT_MAX);
	for (size_t i = 0; i < n; i++)
	{
		pos[i] = _vertices[i].pos;
		lo = glm::min(lo, pos[i]);
		hi = glm::max(hi, pos[i]);
	}

	// a grid of voxels over the mesh, with room for the ball all round
	float h = 0.5f;
	glm::vec3 pad(radius + 4.f);
	glm::vec3 origin = lo - pad;
	glm::vec3 size = (hi + pad) - origin;
	while ((size.x / h) * (size.y / h) * (size.z / h) > 5e6f) { h *= 1.1f; }
	int nx = (int)std::ceil(size.x / h), ny = (int)std::ceil(size.y / h);
	int nz = (int)std::ceil(size.z / h);
	size_t total = (size_t)nx * ny * nz;

	// what the mesh encloses: along each line in x, the voxels between an odd
	// and an even crossing of a triangle (a hair off the grid lines, so that
	// no ray goes exactly through an edge)
	std::vector<std::vector<float>> crossings((size_t)ny * nz);
	const float jitter = 1.3e-3f * h;
	for (size_t t = 0; t + 3 <= tris.size(); t += 3)
	{
		glm::vec3 a = pos[tris[t]], b = pos[tris[t + 1]], c = pos[tris[t + 2]];
		float minY = std::min(a.y, std::min(b.y, c.y)), 
		maxY = std::max(a.y, std::max(b.y, c.y));
		float minZ = std::min(a.z, std::min(b.z, c.z)), 
		maxZ = std::max(a.z, std::max(b.z, c.z));
		int y0 = std::max(0, (int)std::ceil((minY - jitter - origin.y) / h - 0.5f));
		int y1 = std::min(ny - 1, (int)std::floor((maxY - jitter - origin.y) / h - 0.5f));
		int z0 = std::max(0, (int)std::ceil((minZ - jitter * 1.7f - origin.z) / h - 0.5f));
		int z1 = std::min(nz - 1, (int)std::floor((maxZ - jitter * 1.7f - origin.z) / h - 0.5f));

		double det = (double)(b.y - a.y) * (c.z - a.z) - (double)(c.y - a.y) * (b.z - a.z);
		if (std::fabs(det) < 1e-12) { continue; }

		for (int iz = z0; iz <= z1; iz++)
		for (int iy = y0; iy <= y1; iy++)
		{
			double py = origin.y + (iy + 0.5) * h + jitter;
			double pz = origin.z + (iz + 0.5) * h + jitter * 1.7f;
			double u = ((py - a.y) * (c.z - a.z) - (c.y - a.y) * (pz - a.z)) / det;
			double w = ((b.y - a.y) * (pz - a.z) - (py - a.y) * (b.z - a.z)) / det;
			if (u < 0 || w < 0 || u + w > 1) { continue; }
			crossings[(size_t)iz * ny + iy].push_back
			((float)(a.x + u * (b.x - a.x) + w * (c.x - a.x)));
		}
	}

	const float far = 1e12f;
	std::vector<float> solid(total, far);
	for (int iz = 0; iz < nz; iz++)
	for (int iy = 0; iy < ny; iy++)
	{
		std::vector<float> &list = crossings[(size_t)iz * ny + iy];
		std::sort(list.begin(), list.end());
		for (size_t k = 0; k + 1 < list.size(); k += 2)
		{
			int x0 = std::max(0, (int)std::ceil((list[k] - origin.x) / h - 0.5f));
			int x1 = std::min(nx - 1, (int)std::floor((list[k + 1] - origin.x) / h - 0.5f));
			for (int ix = x0; ix <= x1; ix++)
			{
				solid[((size_t)iz * ny + iy) * nx + ix] = 0.f;
			}
		}
	}
	crossings.clear();

	// grow: all that is within the radius of the solid. Then shrink: the
	// distance of those from the rest. What is still further than the radius
	// from the rest is the solid, closed.
	edt3d(solid, nx, ny, nz);
	float radiusSq = (radius / h) * (radius / h);
	for (size_t i = 0; i < total; i++)
	{
		solid[i] = (solid[i] <= radiusSq ? far : 0.f);
	}
	edt3d(solid, nx, ny, nz);
	std::vector<float> &depth = solid;
	for (size_t i = 0; i < total; i++)
	{
		depth[i] = std::sqrt(depth[i]) * h;
	}

	auto sample = [&](const glm::vec3 &p)
	{
		float fx = (p.x - origin.x) / h - 0.5f;
		float fy = (p.y - origin.y) / h - 0.5f;
		float fz = (p.z - origin.z) / h - 0.5f;
		int ix = std::max(0, std::min(nx - 2, (int)std::floor(fx)));
		int iy = std::max(0, std::min(ny - 2, (int)std::floor(fy)));
		int iz = std::max(0, std::min(nz - 2, (int)std::floor(fz)));
		float tx = std::max(0.f, std::min(1.f, fx - ix));
		float ty = std::max(0.f, std::min(1.f, fy - iy));
		float tz = std::max(0.f, std::min(1.f, fz - iz));

		float sum = 0;
		for (int k = 0; k < 2; k++)
		for (int j = 0; j < 2; j++)
		for (int i = 0; i < 2; i++)
		{
			float weight = (i ? tx : 1 - tx) * (j ? ty : 1 - ty) * 
			(k ? tz : 1 - tz);
			sum += weight * depth[((size_t)(iz + k) * ny + iy + j) * nx + ix + i];
		}
		return sum;
	};

	// The mesh lies on the edge of the solid, so a vertex is no more than the
	// radius from the rest: further than that, it is in a hollow which the
	// closed surface spans. It goes out, away from the solid, to that surface.
	// A voxel's worth of error is let go, so that vertices on the surface do
	// not creep outwards with every cycle.
	float tolerance = 1.5f * h;
	int moved = 0;
	std::vector<glm::vec3> next = pos;
	for (size_t i = 0; i < n; i++)
	{
		glm::vec3 p = pos[i];
		if (sample(p) - radius <= tolerance) { continue; }

		for (int attempt = 0; attempt < 6; attempt++)
		{
			float excess = sample(p) - radius;
			if (excess <= 0.2f * h) { break; }

			glm::vec3 grad(sample(p + glm::vec3(h, 0, 0)) - sample(p - glm::vec3(h, 0, 0)),
			               sample(p + glm::vec3(0, h, 0)) - sample(p - glm::vec3(0, h, 0)),
			               sample(p + glm::vec3(0, 0, h)) - sample(p - glm::vec3(0, 0, h)));
			float length = glm::length(grad);
			if (length < 1e-6f) { break; }

			// the depth grows going into the hollow, so out is against it
			p -= (grad / length) * excess;
		}

		next[i] = p;
		moved++;
	}

	if (moved)
	{
		{
			std::unique_lock<std::mutex> lock(_vertLock);
			for (size_t i = 0; i < n; i++)
			{
				_vertices[i].pos = next[i];
			}
			forceRender(true, true);
		}

		_triangleIdxs = tris;
		calculateNormalsFrom(_triangleIdxs);
		_connections.clear();
		delete _tree;
		_tree = nullptr;
		invalidateExclusions();
		invalidateCurvature();
		saveCache();
	}

	_refined = true;
	std::cout << "Grow-shrink cycle (ball of " << radius << " A): moved " 
	<< moved << " of " << n << " vertices" << std::endl;
}

void Mesh::computeCurvature()
{
	// (_curvMutex is held)
	std::vector<GLuint> tris = triangles();
	size_t n = _vertices.size();
	_curvZ.assign(n, 0.f);
	if (n == 0 || tris.empty())
	{
		return;
	}

	std::vector<glm::dvec3> pos(n);
	glm::dvec3 middle{};
	for (size_t i = 0; i < n; i++)
	{
		pos[i] = glm::dvec3(_vertices[i].pos);
		middle += pos[i];
	}
	middle /= (double)n;

	// The gradient of the surface area at a vertex is the mean curvature
	// (times twice the area there) along the way the surface faces. Which
	// way is out is told by the volume: positive if the triangles wind
	// outwards.
	std::vector<glm::dvec3> gradArea(n, glm::dvec3(0.)), facing(n, glm::dvec3(0.));
	std::vector<double> area(n, 0.);
	double volume = 0;

	for (size_t t = 0; t + 3 <= tris.size(); t += 3)
	{
		GLuint ia = tris[t], ib = tris[t + 1], ic = tris[t + 2];
		glm::dvec3 a = pos[ia] - middle, b = pos[ib] - middle, 
		c = pos[ic] - middle;
		volume += glm::dot(a, glm::cross(b, c)) / 6.;

		glm::dvec3 normal = glm::cross(b - a, c - a);
		double twice = glm::length(normal);
		if (twice < 1e-12) { continue; }

		glm::dvec3 unit = normal / twice;
		gradArea[ia] += 0.5 * glm::cross(unit, c - b);
		gradArea[ib] += 0.5 * glm::cross(unit, a - c);
		gradArea[ic] += 0.5 * glm::cross(unit, b - a);
		for (GLuint i : {ia, ib, ic})
		{
			facing[i] += normal;
			area[i] += twice / 6.;
		}

	}

	double outwards = (volume >= 0 ? 1. : -1.);
	std::vector<double> curvature(n, 0.);
	for (size_t i = 0; i < n; i++)
	{
		double length = glm::length(facing[i]);
		if (area[i] < 1e-9 || length < 1e-12) { continue; }

		glm::dvec3 out = facing[i] * (outwards / length);
		curvature[i] = glm::dot(gradArea[i], out) / (2. * area[i]);
	}

	// Vertices an Angstrom or so apart see only the bumps of single atoms, so
	// the curvature at each is the average over everything within 30 Angstroms
	// (as the crow flies), each vertex counting for the area it covers.
	const float radius = 30.f;
	typedef std::array<int, 3> Cell;
	auto cell_of = [radius](const glm::dvec3 &p)
	{
		return Cell{(int)std::floor(p.x / radius), (int)std::floor(p.y / radius),
		            (int)std::floor(p.z / radius)};
	};
	auto key_of = [](int x, int y, int z)
	{
		return ((int64_t)(x & 0x1FFFFF) << 42) | ((int64_t)(y & 0x1FFFFF) << 21)
		| (int64_t)(z & 0x1FFFFF);
	};

	std::unordered_map<int64_t, std::vector<GLuint>> grid;
	for (size_t i = 0; i < n; i++)
	{
		Cell c = cell_of(pos[i]);
		grid[key_of(c[0], c[1], c[2])].push_back(i);
	}

	std::vector<double> averaged(n, 0.);
	for (size_t i = 0; i < n; i++)
	{
		Cell c = cell_of(pos[i]);
		double sum = 0, weight = 0;

		for (int x = -1; x <= 1; x++)
		for (int y = -1; y <= 1; y++)
		for (int z = -1; z <= 1; z++)
		{
			auto it = grid.find(key_of(c[0] + x, c[1] + y, c[2] + z));
			if (it == grid.end()) { continue; }

			for (GLuint j : it->second)
			{
				glm::dvec3 diff = pos[j] - pos[i];
				if (glm::dot(diff, diff) > (double)radius * radius) { continue; }

				sum += area[j] * curvature[j];
				weight += area[j];
			}
		}

		averaged[i] = (weight > 0 ? sum / weight : curvature[i]);
	}
	curvature = averaged;

	double mean = 0, spread = 0;
	for (double c : curvature) { mean += c; }
	mean /= (double)n;
	for (double c : curvature) { spread += (c - mean) * (c - mean); }
	spread = std::sqrt(spread / (double)n);

	for (size_t i = 0; i < n; i++)
	{
		_curvZ[i] = (spread > 1e-12 ? (float)((curvature[i] - mean) / spread) : 0.f);
	}
}

std::vector<float> Mesh::curvatureZ()
{
	std::unique_lock<std::mutex> lock(_curvMutex);
	if (_curvZ.size() != _vertices.size())
	{
		computeCurvature();
	}

	return _curvZ;
}

void Mesh::invalidateCurvature()
{
	{
		std::unique_lock<std::mutex> lock(_curvMutex);
		_curvZ.clear();
	}

	if (_colourCurv)
	{
		applyMarks();
	}
}

void Mesh::setFootprintMarks(const std::set<int> &marked)
{
	_footprintMarks = marked;

	if (marked.empty())
	{
		if (_footprintFaces)
		{
			_footprintFaces->setDisabled(true);
		}
		return;
	}

	// every face which touches one of the vertices, shaded as the faces of an
	// antibody's positions are
	std::vector<GLuint> tris = triangles();
	std::vector<GLuint> shaded;
	for (size_t i = 0; i + 3 <= tris.size(); i += 3)
	{
		if (marked.count(tris[i]) || marked.count(tris[i + 1]) || 
		    marked.count(tris[i + 2]))
		{
			shaded.insert(shaded.end(), tris.begin() + i, tris.begin() + i + 3);
		}
	}

	if (!_footprintFaces)
	{
		_footprintFaces = new MeshFaces();
		_footprintFaces->setOverSolid(_opaque);
		addObject(_footprintFaces);
	}

	_footprintFaces->show(_vertices, shaded, 0.45f, {}, 0.f);
}

void Mesh::setCurvatureColours(bool on)
{
	_colourCurv = on;
	applyMarks();
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

std::vector<glm::vec3> Mesh::vertexPositions() const
{
	std::vector<glm::vec3> positions;
	positions.reserve(_vertices.size());
	for (const Vertex &v : _vertices)
	{
		positions.push_back(v.pos);
	}

	return positions;
}

std::function<glm::vec3()> Mesh::random()
{
	return [this]()
	{
		std::unique_lock<std::mutex> lock(_excludeMutex);
		int n = vertexCount();
		int i = (rand() % n);

		// not an excluded one: another is drawn (all of them excluded can
		// only be an accident, and then any will do)
		if (_excluded.size() < (size_t)n)
		{
			for (int tries = 0; tries < 1000 && _excluded.count(i); tries++)
			{
				i = (rand() % n);
			}
		}

		return _vertices[i].pos;
	};
}

void Mesh::findOutward()
{
	if (_outward != 0.f || _vertices.empty())
	{
		return;
	}

	glm::vec3 middle = centroid();
	double sum = 0;
	for (const Vertex &v : _vertices)
	{
		sum += glm::dot(v.normal, v.pos - middle);
	}

	_outward = (sum >= 0 ? 1.f : -1.f);
}

std::vector<int> Mesh::verticesInBox(float top, float left, float bottom, 
                                     float right)
{
	std::vector<int> found;
	if (!_gl)
	{
		return found;
	}

	findOutward();
	glm::mat4x4 model = _gl->getModel();
	glm::mat4x4 proj = _gl->getProjection();
	glm::mat3x3 rotation = glm::mat3x3(model);

	float minX = std::min(left, right), maxX = std::max(left, right);
	float minY = std::min(top, bottom), maxY = std::max(top, bottom);

	for (size_t i = 0; i < _vertices.size(); i++)
	{
		// only the side facing the viewer, not what is behind it
		glm::vec3 normal = rotation * (_vertices[i].normal * _outward);
		if (normal.z <= 0)
		{
			continue;
		}

		glm::vec4 pos = proj * model * glm::vec4(_vertices[i].pos, 1.f);
		pos /= pos.w;

		if (pos.x >= minX && pos.x <= maxX && pos.y >= minY && pos.y <= maxY)
		{
			found.push_back(i);
		}
	}

	return found;
}

void Mesh::setSelection(const std::set<int> &selection)
{
	_selected = selection;
	applyMarks();
}

void Mesh::excludeSelection()
{
	{
		std::unique_lock<std::mutex> lock(_excludeMutex);
		_excluded.insert(_selected.begin(), _selected.end());
	}

	_selected.clear();
	keepExclusions();
	applyMarks();
}

// the exclusions go with the antigen, to be saved: as positions
void Mesh::keepExclusions()
{
	std::unique_lock<std::mutex> lock(_excludeMutex);
	_antigen.excluded_start.clear();
	for (int i : _excluded)
	{
		_antigen.excluded_start.push_back(_vertices[i].pos);
	}
}

void Mesh::invalidateExclusions()
{
	if (hasExclusions() || _antigen.excluded_start.size())
	{
		clearExclusions();
	}
}

void Mesh::loadExclusions()
{
	if (_vertices.empty())
	{
		return;
	}

	{
		std::unique_lock<std::mutex> lock(_excludeMutex);
		_excluded.clear();

		for (const glm::vec3 &p : _antigen.excluded_start)
		{
			int idx = nearestVertex(p);

			// the mesh may be a different one to the one it was kept for
			if (idx >= 0 && idx < (int)_vertices.size() &&
			    glm::length(_vertices[idx].pos - p) < 1.f)
			{
				_excluded.insert(idx);
			}
		}
	}

	applyMarks();
}

void Mesh::clearExclusions()
{
	{
		std::unique_lock<std::mutex> lock(_excludeMutex);
		_excluded.clear();
	}

	keepExclusions();
	applyMarks();
}

bool Mesh::hasExclusions()
{
	std::unique_lock<std::mutex> lock(_excludeMutex);
	return !_excluded.empty();
}

// selected vertices are highlighted, excluded ones dark blue; everything
// else is as it was
void Mesh::applyMarks()
{
	if (_baseColours.size() != _vertices.size())
	{
		_baseColours.resize(_vertices.size());
		for (size_t i = 0; i < _vertices.size(); i++)
		{
			_baseColours[i] = _vertices[i].color;
		}
	}

	// the usual colour, or the curvature's: from blue (concave) through to red
	// (convex), over three standard deviations either way
	std::vector<glm::vec4> curved;
	if (_colourCurv)
	{
		static std::vector<glm::vec4> table;
		static std::once_flag once;
		std::call_once(once, []()
		{
			ColourScheme scheme(Cluster4x);
			for (int i = 0; i <= 255; i++)
			{
				table.push_back(scheme.colour((float)i / 255.f));
			}
		});

		std::vector<float> z = curvatureZ();
		curved.resize(_vertices.size());
		for (size_t i = 0; i < curved.size(); i++)
		{
			float p = std::max(0.f, std::min(1.f, 0.5f + z[i] / 6.f));
			curved[i] = table[(int)(p * 255.f + 0.5f)];
			curved[i].a = 1.f;
		}
	}

	std::set<int> excluded;
	{
		std::unique_lock<std::mutex> lock(_excludeMutex);
		excluded = _excluded;
	}

	const glm::vec4 highlight(1.f, 0.85f, 0.1f, 1.f);
	const glm::vec4 dark(0.02f, 0.02f, 0.4f, 1.f);
	std::vector<glm::vec4> marks(_vertices.size(), glm::vec4(0.f));

	for (int i : _selected)
	{
		if (i >= 0 && i < (int)marks.size()) marks[i] = highlight;
	}

	for (int i : excluded)
	{
		if (i >= 0 && i < (int)marks.size()) marks[i] = dark;
	}

	for (size_t i = 0; i < _vertices.size(); i++)
	{
		_vertices[i].color = (marks[i].a > 0 ? marks[i] : 
		                      (_colourCurv ? curved[i] : _baseColours[i]));
	}

	forceRender(true, false);

	if (_solid && _opaque)
	{
		_solid->show(_vertices, triangles(), &marks);
	}
}

void MeshFaces::show(const std::vector<Vertex> &base, 
                     const std::vector<GLuint> &strong, float strongAlpha,
                     const std::vector<GLuint> &faint, float faintAlpha)
{
	_vertices.clear();
	_indices.clear();

	auto add = [this, &base](const std::vector<GLuint> &tris, float alpha)
	{
		for (GLuint idx : tris)
		{
			Vertex v = base[idx];
			v.color = glm::vec4(0.f, 0.f, 0.f, alpha);
			_indices.push_back(_vertices.size());
			_vertices.push_back(v);
		}
	};

	add(faint, faintAlpha);
	add(strong, strongAlpha);

	setDisabled(_indices.empty());
	forceRender(true, true);
}

// Translucent triangles which overlap (the near and far side of the mesh, say)
// would pile up, depending on the order they are drawn in. So the depth of
// the nearest one is found first, then only fragments at that depth are
// blended: one layer, whichever way round the mesh is.
void MeshFaces::render(GLView *gl)
{
	if (isDisabled())
	{
		Renderable::render(gl);
		return;
	}

	if (_overSolid)
	{
		// level with the solid surface, or nearer; the solid has already
		// hidden the far side, so one pass will do, and its depth stays
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(-1.f, -1.f);
		Renderable::render(gl);
		glDisable(GL_POLYGON_OFFSET_FILL);
		glDepthFunc(GL_LESS);
		glDisable(GL_DEPTH_TEST);
		return;
	}

	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	Renderable::render(gl);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	Renderable::render(gl);

	// nothing drawn after this should be hidden by these faces
	glDepthFunc(GL_LESS);
	glDisable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT);
}

// in line mode each triangle a, b, c is stored as a, b, c, a, b, c (see
// Renderable::changeToLines()), which draws its three edges
std::vector<GLuint> Mesh::triangles() const
{
	std::vector<GLuint> tris;
	size_t stride = (_renderType == GL_LINES ? 6 : 3);

	for (size_t i = 0; i + stride <= _indices.size(); i += stride)
	{
		tris.push_back(_indices[i]);
		tris.push_back(_indices[i + 1]);
		tris.push_back(_indices[i + 2]);
	}

	return tris;
}

void Mesh::clearHeat()
{
	if (_faces)
	{
		_faces->setDisabled(true);
	}
}

void Mesh::setHeat(const std::map<int, int> &main, 
                   const std::map<int, int> &others)
{
	auto mark = [this](const std::map<int, int> &counts)
	{
		std::vector<bool> found(_vertices.size(), false);
		for (const auto &entry : counts)
		{
			if (entry.first >= 0 && entry.first < (int)found.size())
			{
				found[entry.first] = true;
			}
		}
		return found;
	};

	std::vector<bool> mainFound = mark(main);
	std::vector<bool> otherFound = mark(others);
	std::vector<GLuint> tris = triangles();
	std::vector<GLuint> strong, faint;

	for (size_t i = 0; i + 3 <= tris.size(); i += 3)
	{
		auto touches = [&](const std::vector<bool> &found)
		{
			return (found[tris[i]] || found[tris[i + 1]] || found[tris[i + 2]]);
		};

		// a face of both is only drawn strongly
		std::vector<GLuint> *dest = touches(mainFound) ? &strong : 
		(touches(otherFound) ? &faint : nullptr);

		if (dest)
		{
			dest->insert(dest->end(), tris.begin() + i, tris.begin() + i + 3);
		}
	}

	if (!_faces)
	{
		_faces = new MeshFaces();
		_faces->setOverSolid(_opaque);
		addObject(_faces);
	}

	_faces->show(_vertices, strong, 0.45f, faint, 0.1f);
}

void MeshSolid::show(const std::vector<Vertex> &base, 
                     const std::vector<GLuint> &tris,
                     const std::vector<glm::vec4> *marks)
{
	_vertices = base;
	for (size_t i = 0; i < _vertices.size(); i++)
	{
		Vertex &v = _vertices[i];
		bool marked = (marks && i < marks->size() && (*marks)[i].a > 0);
		v.color = (marked ? (*marks)[i] : glm::vec4(0.5, 0.5, 0.8, 1.0));
		v.extra = glm::vec4(0.f);
	}

	_indices = tris;
	setDisabled(_indices.empty());
	forceRender(true, true);
}

void MeshSolid::render(GLView *gl)
{
	if (isDisabled())
	{
		Renderable::render(gl);
		return;
	}

	// it writes the depth that hides the far side of the antigen, and what
	// comes after keeps it
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	Renderable::render(gl);
	glDisable(GL_DEPTH_TEST);
}

void Mesh::setOpaque(bool opaque)
{
	_opaque = opaque;

	if (opaque && !_solid)
	{
		_solid = new MeshSolid();
		addObject(_solid);
	}

	if (opaque)
	{
		applyMarks();

		// the solid goes first, the translucent faces over it
		if (_faces)
		{
			removeObject(_faces);
			addObject(_faces);
		}

		if (_footprintFaces)
		{
			removeObject(_footprintFaces);
			addObject(_footprintFaces);
		}
	}
	else if (_solid)
	{
		_solid->setDisabled(true);
	}

	if (_faces)
	{
		_faces->setOverSolid(opaque);
	}

	if (_footprintFaces)
	{
		_footprintFaces->setOverSolid(opaque);
	}
}

void Mesh::render(GLView *gl)
{
	if (!_opaque)
	{
		SimplePolygon::render(gl);
		return;
	}

	// not the lines, just what is laid over the mesh
	_gl = gl;

	if (isDisabled())
	{
		return;
	}

	for (size_t i = 0; i < objectCount(); i++)
	{
		object(i)->render(gl);
	}
}

glm::vec3 Mesh::outwardAt(const glm::vec3 &p)
{
	if (_vertices.empty())
	{
		return glm::vec3(0.f);
	}

	if (_outward == 0.f)
	{
		glm::vec3 middle = centroid();
		double sum = 0;
		for (const Vertex &v : _vertices)
		{
			sum += glm::dot(v.normal, v.pos - middle);
		}

		_outward = (sum >= 0 ? 1.f : -1.f);
	}

	int idx = nearestVertex(p);
	if (idx < 0 || idx >= (int)_vertices.size())
	{
		return glm::vec3(0.f);
	}

	glm::vec3 normal = _vertices[idx].normal;
	float length = glm::length(normal);
	return (length > 1e-6f ? normal * (_outward / length) : glm::vec3(0.f));
}
