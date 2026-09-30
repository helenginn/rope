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

#ifndef __vagabond__Mesh__
#define __vagabond__Mesh__

#include <nlohmann/json.hpp>
#include <vagabond/utils/OpSet.h>
#include <vagabond/gui/elements/SimplePolygon.h>
#include <vagabond/core/grids/ArbitraryMap.h>
using nlohmann::json;

struct Antigen;

class KdTree;
class AtomGroup;

/** translucent black faces laid over the mesh (see Mesh::setHeat()). Every
 *  triangle has its own vertices, so each can have its own opacity, and only
 *  the frontmost layer is ever blended whichever way it is viewed. */
class MeshFaces : public SimplePolygon
{
public:
	MeshFaces()
	{
		setUsesProjection(true);
		setVertexShaderFile("assets/shaders/with_matrix.vsh");
		setFragmentShaderFile("assets/shaders/color_only.fsh");
		setDisabled(true);
	}

	// shows just these triangles (three indices each) of `base`, the strong
	// ones and the faint ones, in black at these opacities
	void show(const std::vector<Vertex> &base, 
	          const std::vector<GLuint> &strong, float strongAlpha,
	          const std::vector<GLuint> &faint, float faintAlpha);

	virtual void render(GLView *gl);
};

class Mesh : public SimplePolygon
{
public:
	Mesh(Antigen &antigen);

	AtomGroup *atoms();

	void refine();
	void kdTree();

	// lays translucent black over every face which touches one of the
	// vertices of `main` (strongly) or, failing that, of `others` (faintly)
	void setHeat(const std::map<int, int> &main, 
	             const std::map<int, int> &others = {});
	void clearHeat();

	int nearestVertex(const glm::vec3 &v);

	std::function<void(const glm::vec3 &, glm::vec3 &, int &)> vertexFinder();
	std::function<glm::vec3()> random();
private:
	ArbitraryMap mappedAtoms();
	int removeHollows(ArbitraryMap &map);
	void growOrShrinkBorder(ArbitraryMap &map, int dir);
	void marchingCubes();
	void normalsFromLines();

	std::string cachePath();
	bool loadCache();
	void saveCache();
	bool _loadedFromCache = false;

	Antigen &_antigen;

	AtomGroup *_atoms{};
	std::map<GLuint, OpSet<GLuint>> _connections;
	void calculateNormalsFrom(const std::vector<GLuint> &indices);
	float adjustVertices(bool planar);
	
	std::vector<GLuint> _triangleIdxs;
	std::vector<GLuint> _lineIdxs;
	
	// the mesh's triangles, three indices each, whatever its render mode
	std::vector<GLuint> triangles() const;

	MeshFaces *_faces{};
	KdTree *_tree{};
};

#endif
