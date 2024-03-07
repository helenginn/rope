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

#include <fstream>
#include "GuiDensity.h"
#include <vagabond/core/refine/Sampler.h>
#include <vagabond/gui/elements/SnowGL.h>
#include <vagabond/core/AtomGroup.h>
#include <vagabond/core/BondCalculator.h>
#include <vagabond/core/grids/AtomMap.h>
#include <vagabond/core/grids/ArbitraryMap.h>
#include "Result.h"
#define MC_IMPLEM_ENABLE
#include "MC.h"

GuiDensity::GuiDensity() : CullablePrimitives()
{
	setUsesProjection(true);
	setVertexShaderFile("assets/shaders/density.vsh");
	setFragmentShaderFile("assets/shaders/density.fsh");
	setDisabled(true);
	setName("Gui density");
}

void GuiDensity::objectFromMesh(MC::mcMesh &mesh, bool diff)
{
	_indices.clear();
	_vertices.clear();

	for (size_t i = 0; i < mesh.vertices.size(); i++)
	{
		MC::mcVec3f &mp = mesh.vertices[i];
		MC::mcVec3f &mn = mesh.normals[i];
		Vertex v{};
		v.pos = glm::vec3(mp.x, mp.y, mp.z);
		v.normal = glm::vec3(mn.x, mn.y, mn.z);
		v.color = glm::vec4(0.5, 0.5, 0.8, 1.0);
		
		if (diff && mesh.values[i] < 0)
		{
			v.color = glm::vec4(1.0, 0.8, 0.2, 1.0);
		}
		
		_vertices.push_back(v);
	}

	for (size_t i = 0; i < mesh.indices.size(); i++)
	{
		addIndex(mesh.indices[i]);
	}
	
}

void GuiDensity::sampleFromOtherMap(OriginGrid<fftwf_complex> *ref, 
                                    OriginGrid<fftwf_complex> *map,
                                    bool diff)
{
	if (ref == nullptr || map == nullptr)
	{
		return;
	}

	glm::vec3 min = map->minBound();
	glm::vec3 max = map->maxBound();
	float step = _step;
	
	if (step < 0)
	{
		step = FLT_MAX;
		for (size_t i = 0; i < 3; i++)
		{
			glm::vec3 test = glm::vec3({i % 3 == 0, i % 3 == 1, i % 3 == 2});
			map->voxel2Real(test);
			test -= min;
			float l = glm::length(test) / 2;
			if (step > l)
			{
				step = l;
			}
		}
	}

	glm::vec3 limits = max - min;

	float num_per_ang_cubed = 1 / (step * step * step);
	float vol = limits.x * limits.y * limits.z;

	std::vector<float> vals;
	vals.reserve((int)(num_per_ang_cubed * vol));

	for (float z = 0; z < limits.z - step / 2; z += step)
	{
		for (float y = 0; y < limits.y - step / 2; y += step)
		{
			for (float x = 0; x < limits.x - step / 2; x += step)
			{
				glm::vec3 real = glm::vec3(x, y, z) + min;
				float val = ref->realValue(real);
				bool report = fabs(x) > 20 && fabs(x) < 22;
				if (_tear)
				{
					val -= _tear->realValue(real);
				}

				vals.push_back(val);
			}
		}
	}
	
	unsigned int nx = (int)lrint(limits.x / step);
	unsigned int ny = (int)lrint(limits.y / step);
	unsigned int nz = (int)lrint(limits.z / step);

	const float *ptr = &vals[0];
	float mean = _tear ? _tear->mean() : ref->mean();
	if (diff)
	{
		mean = 0;
	}

	float sigma = _tear ? _tear->sigma() : ref->sigma();
	float thresh = mean + sigma * _threshold * 3;
	std::cout << "Mean/sigma: " << mean << " " << sigma << std::endl;
	
	_ref = ref;
	_map = map;

	MC::mcMesh mesh;
	MC::marching_cube(ptr, nx, ny, nz, thresh, mesh, diff);
	
	std::unique_lock<std::mutex> lock(_vertLock);
	objectFromMesh(mesh, diff);
	rotateByMatrix(glm::mat3(step));
	addToVertices(min);
	setDisabled(false);
	lock.unlock();
	rebufferVertexData();
	rebufferIndexData();
}

void GuiDensity::tear(AtomMap *map)
{
	_tear = map;
}

void GuiDensity::fromAtomMap(AtomMap *map, bool differences)
{
	_hasUC = false;
	sampleFromOtherMap(map, map, differences);
}

void GuiDensity::populateFromMap(OriginGrid<fftwf_complex> *map)
{
	sampleFromOtherMap(map, map);
}

void GuiDensity::render(SnowGL *gl)
{
	glEnable(GL_DEPTH_TEST);
	
	_model = gl->getModel();
//	reorderIndices();
	Renderable::render(gl);

	glDisable(GL_DEPTH_TEST);
}


void GuiDensity::extraUniforms()
{
	glm::vec3 centre = glm::vec3(0.);
	if (_gl)
	{
		centre = _gl->getCentre();
	}
	
	GLuint uTrack = glGetUniformLocation(_program, "track");
	glUniform1i(uTrack, _tracking);
	uTrack = glGetUniformLocation(_program, "track_frag");
	glUniform1i(uTrack, _tracking);

	GLuint uCentre = glGetUniformLocation(_program, "centre");
	glUniform3f(uCentre, centre[0], centre[1], centre[2]);

	GLuint uSlice = glGetUniformLocation(_program, "slice");
	glUniform1f(uSlice, _slice);
	
	glm::mat3x3 uc = glm::mat3(1000.f);
	
	if (_hasUC && _map)
	{
		uc = _map->frac2Real();
	}

	GLuint uCell = glGetUniformLocation(_program, "unit_cell");
	glUniformMatrix3fv(uCell, 1, GL_FALSE, &uc[0][0]);
}
