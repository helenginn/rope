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
#include "../core/Sampler.h"
#include <vagabond/gui/elements/SnowGL.h>
#include "../core/AtomGroup.h"
#include "../core/BondCalculator.h"
#include <vagabond/core/AtomMap.h>
#include "../core/ArbitraryMap.h"
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

void GuiDensity::objectFromMesh(MC::mcMesh &mesh)
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
		
		_vertices.push_back(v);
	}

	for (size_t i = 0; i < mesh.indices.size(); i++)
	{
		addIndex(mesh.indices[i]);
	}
	
}

void GuiDensity::sampleFromOtherMap(OriginGrid<fftwf_complex> *ref, 
                                    OriginGrid<fftwf_complex> *map)
{
	if (ref == nullptr || map == nullptr)
	{
		return;
	}

	glm::vec3 min = map->minBound();
	glm::vec3 max = map->maxBound();
	float step = 0.5;
	float num_per_ang_cubed = 1 / (step * step * step);
	float lx = (max.x - min.x);
	float ly = (max.y - min.y);
	float lz = (max.z - min.z);
	float vol = lx * ly * lz;

	std::vector<float> vals;
	vals.reserve((int)(num_per_ang_cubed * vol));

	for (float z = min.z; z < max.z - step / 2; z += step)
	{
		for (float y = min.y; y < max.y - step / 2; y += step)
		{
			for (float x = min.x; x < max.x - step / 2; x += step)
			{
				glm::vec3 real(x, y, z);
				float val = ref->realValue(real);
				vals.push_back(val);
			}
		}
	}
	
	unsigned int ns[3] = {0, 0, 0};
	
	for (size_t i = 0; i < 3; i++)
	{
		for (float j = min[i]; j < max[i] - step / 2; j += step)
		{
			ns[i]++;
		}
	}
	
	unsigned int nx = (int)lrint(lx / step);
	unsigned int ny = (int)lrint(ly / step);
	unsigned int nz = (int)lrint(lz / step);

	const float *ptr = &vals[0];
	float mean = ref->mean();
	float sigma = ref->sigma();
	float thresh = mean + sigma;
	
	_ref = ref;
	_map = map;

	MC::mcMesh mesh;
	MC::marching_cube(ptr, nx, ny, nz, thresh, mesh);
	
	lockMutex();
	objectFromMesh(mesh);
	rotateByMatrix(glm::mat3(step));
	addToVertices(min);
	setDisabled(false);
	rebufferVertexData();
	rebufferIndexData();
	unlockMutex();
}

void GuiDensity::fromMap(AtomMap *map)
{
	const float *ptr = map->arrayPtr();

	float mean = map->mean();
	float sigma = map->sigma();
	float thresh = mean + sigma;
	unsigned int nx = map->nx();
	unsigned int ny = map->ny();
	unsigned int nz = map->nz();
	
	for (size_t i = 0; i < 10; i++)
	{
		std::cout << ptr[i] << " ";
	}
	std::cout << std::endl;

	float real = map->realDim();
	glm::vec3 origin = map->origin();

	MC::mcMesh mesh;
	MC::marching_cube(ptr, nx, ny, nz, thresh, mesh);

	lockMutex();
	objectFromMesh(mesh);
	rotateByMatrix(glm::mat3(real));
	addToVertices(origin);
	setDisabled(false);
	rebufferVertexData();
	rebufferIndexData();
	unlockMutex();

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

void GuiDensity::recalculate()
{
	int dims = 2;
	Sampler sampler(50, dims);

	BondCalculator calculator;
	if (_ref == nullptr)
	{
		calculator.setPipelineType(BondCalculator::PipelineCalculatedMaps);
	}
	else
	{
		calculator.setPipelineType(BondCalculator::PipelineCorrelation);
	}

	calculator.setMaxSimultaneousThreads(1);
	calculator.setTorsionBasisType(TorsionBasis::TypeSimple);
	calculator.setSampler(&sampler);
	int num = sampler.pointCount();

	std::vector<AtomGroup *> subgroups = _atoms->connectedGroups();

	for (size_t i = 0; i < subgroups.size(); i++)
	{
		Atom *anchor = subgroups[i]->chosenAnchor();
		
		if (anchor)
		{
			calculator.addAnchorExtension(anchor);
			break;
		}
	}

	if (_ref != nullptr)
	{
		calculator.setReferenceDensity(_ref);
	}

	calculator.setup();

	calculator.start();

	Job job{};
	job.custom.allocate_vectors(1, dims, num);
	job.requests = static_cast<JobType>(JobExtractPositions | 
	                                    JobCalculateMapSegment);
	if (_ref != nullptr)
	{
		job.requests = (JobType)(job.requests | JobMapCorrelation);
	}

	calculator.submitJob(job);

	Result *result = calculator.acquireResult();
	result->transplantPositions();
	AtomMap *map = result->map;

	if (_ref != nullptr)
	{
		sampleFromOtherMap(_ref, map);
		std::cout << "Correlation: " << result->correlation << std::endl;
	}
	else
	{
		populateFromMap(map);
	}

	result->destroy();

	calculator.finish();
}

void GuiDensity::extraUniforms()
{
	glm::vec3 centre = glm::vec3(0.);
	if (_gl)
	{
		centre = _gl->getCentre();
	}

	GLuint uCentre = glGetUniformLocation(_program, "centre");
	glUniform3f(uCentre, centre[0], centre[1], centre[2]);

	GLuint uSlice = glGetUniformLocation(_program, "slice");
	glUniform1f(uSlice, _slice);
	
	glm::mat3x3 uc = glm::mat3(1.f);
	
	if (_map)
	{
		uc = _map->frac2Real();
	}

	GLuint uCell = glGetUniformLocation(_program, "unit_cell");
	glUniformMatrix3fv(uCell, 1, GL_FALSE, &uc[0][0]);
}
