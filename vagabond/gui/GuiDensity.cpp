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
#include "../core/AtomGroup.h"
#include "../core/BondCalculator.h"
#include "../core/AtomMap.h"
#define MC_IMPLEM_ENABLE
#include "MC.h"

GuiDensity::GuiDensity() : Renderable()
{
	setUsesProjection(true);
	setVertexShaderFile("assets/shaders/density.vsh");
	setFragmentShaderFile("assets/shaders/density.fsh");
	setDisabled(true);
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

void GuiDensity::populateFromMap(AtomMap *map)
{
	const float *ptr = map->arrayPtr();
	float thresh = 0.1;
	unsigned int nx = map->nx();
	unsigned int ny = map->ny();
	unsigned int nz = map->nz();
	
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

void GuiDensity::render(SnowGL *gl)
{
	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT);
	
	Renderable::render(gl);

	glDisable(GL_DEPTH_TEST);
}

void GuiDensity::recalculate()
{
	int dims = 1;
	Sampler sampler(30, dims);
	int num = sampler.pointCount();

	BondCalculator calculator;
	calculator.setPipelineType(BondCalculator::PipelineCalculatedMaps);
	calculator.setMaxSimultaneousThreads(1);
	calculator.setTorsionBasisType(TorsionBasis::TypeConcerted);
	calculator.setSampler(&sampler);

	std::vector<AtomGroup *> subgroups = _atoms->connectedGroups();

	for (size_t i = 0; i < subgroups.size(); i++)
	{
		Atom *anchor = subgroups[i]->chosenAnchor();
		
		if (anchor)
		{
			calculator.addAnchorExtension(anchor);
		}
	}

	calculator.setup();

	calculator.start();

	Job job{};
	job.custom.allocate_vectors(1, dims, num);
	job.requests = JobCalculateMapSegment;
	calculator.submitJob(job);

	Result *result = calculator.acquireResult();
	AtomMap *map = result->map;
	populateFromMap(map);
	result->destroy();

	calculator.finish();

	std::cout << "HERE" << std::endl;
}

void GuiDensity::extraUniforms()
{
	GLuint uSlice = glGetUniformLocation(_usingProgram, "slice");
	glUniform1f(uSlice, _slice);
	checkErrors("binding slice");
}
