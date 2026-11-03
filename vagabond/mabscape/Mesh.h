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
#include <mutex>
#include <atomic>
#include <set>
#include <vagabond/gui/elements/SimplePolygon.h>
#include <vagabond/core/grids/ArbitraryMap.h>
using nlohmann::json;

struct Antigen;

class KdTree;
class GuiDensity;
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

	// Over a solid mesh the faces lie on its surface: they are drawn where they
	// are level with it or nearer, and its depth is kept, so that the
	// faces on the far side of the antigen are hidden.
	void setOverSolid(bool over)
	{
		_overSolid = over;
	}

	virtual void render(GLView *gl);
private:
	bool _overSolid{false};
};

/** the mesh as solid faces, lit like the antibodies are */
class MeshSolid : public SimplePolygon
{
public:
	MeshSolid()
	{
		setUsesProjection(true);
		setVertexShaderFile("assets/shaders/with_matrix.vsh");
		setFragmentShaderFile("assets/shaders/lighting.fsh");
		setDisabled(true);
	}

	// these triangles (three indices each) of `base`, with its normals.
	// marks, if given, has a colour for each vertex of `base` which is
	// marked (alpha above 0) in place of the usual one.
	void show(const std::vector<Vertex> &base, const std::vector<GLuint> &tris,
	          const std::vector<glm::vec4> *marks = nullptr);

	virtual void render(GLView *gl);
};

class Mesh : public SimplePolygon
{
public:
	Mesh(Antigen &antigen);

	AtomGroup *atoms();

	// force: go through the smoothing cycles again, even if it is settled
	void refine(bool force = false);

	// settled: refined already (or loaded from the cache, which was)
	bool refined() const
	{
		return _loadedFromCache || _refined;
	}
	/** Moves the vertices downhill on: (surface area) + volumeWeight * (volume
	 *  enclosed by the mesh), both of which are better smaller. With no volume
	 *  weight the surface is simply pulled taut: dents are drawn out and bumps
	 *  smoothed, so it bridges hollows. A volume weight adds an even inward
	 *  pressure (in Angstroms^-1), so that the surface hugs the hollows more.
	 *  Each vertex moves along the gradient of that sum; it may never move into
	 *  the protein (the van der Waals spheres of its heavy atoms): a move which
	 *  would put it inside them, or deeper than it started, is cut back or
	 *  refused. Settles the mesh as refine() does.
	 */
	void shrinkWrap(float volumeWeight);

	/** Evens out the spacing of the vertices along the surface as it is now:
	 *  each moves only across the way the surface faces, towards the middle of
	 *  its neighbours, and never into the protein. The shape is left as it is. */
	void evenOut();

	/** A grow-shrink cycle with a rolling ball of this radius (Angstroms): the
	 *  solid the mesh encloses is grown by the radius and shrunk by it again
	 *  (a morphological closing), which fills every hollow too narrow for the
	 *  ball to roll into. A vertex lying deeper in a hollow than that is moved
	 *  out onto the closed surface; the others, and the triangles joining
	 *  them, stay as they are. Quick enough to follow a slider. */
	void closeCycle(float radius);

	// asks the shrink wrapping or evening out to finish early (keeping what it
	// has done so far); clearStop() before starting something
	void requestStop() { _stop = true; }
	void clearStop() { _stop = false; }

	/** How curved the surface is at each vertex, as a z-score: the mean
	 *  curvature (positive where the surface is convex, outward-bulging, and
	 *  negative where it is concave), smoothed a little over the neighbouring
	 *  vertices, in standard deviations from the mean over the whole mesh.
	 *  Worked out when first asked for, and again after the mesh has changed. */
	std::vector<float> curvatureZ();

	// colours the vertices by that z-score with the Cluster4x scheme (and
	// back to the usual colour); kept up to date while the mesh changes
	void setCurvatureColours(bool on);

	// back to the initial calculation: the cache is thrown away and the mesh
	// made again from the atoms (not refined, so not settled)
	void reset();
	void kdTree();

	// lays translucent black over every face which touches one of the
	// vertices of `main` (strongly) or, failing that, of `others` (faintly)
	void setHeat(const std::map<int, int> &main, 
	             const std::map<int, int> &others = {});
	void clearHeat();

	// Solid faces, lit like the antibodies and hiding what is behind, in place
	// of the wire of lines (the translucent black faces go on working over
	// either).
	void setOpaque(bool opaque);

	bool opaque() const
	{
		return _opaque;
	}

	virtual void render(GLView *gl);

	int nearestVertex(const glm::vec3 &v);

	// where every vertex is
	std::vector<glm::vec3> vertexPositions() const;

	// the direction away from the inside of the antigen at the point of the
	// mesh nearest to this one: a unit vector (or nothing, if the mesh has
	// no normal there). The normals are taken to point outwards if, on the
	// whole, they point away from the middle of the mesh.
	glm::vec3 outwardAt(const glm::vec3 &p);

	std::function<void(const glm::vec3 &, glm::vec3 &, int &)> vertexFinder();

	// random vertices, to start from: never an excluded one (unless they all
	// are). May be called from the thread which refines.
	std::function<glm::vec3()> random();

	// The vertices on the side of the mesh facing the viewer whose positions
	// lie within this box, in GL coordinates (either way up)
	std::vector<int> verticesInBox(float top, float left, float bottom, 
	                               float right);

	// shades the faces on these vertices in translucent dark grey, as the
	// faces of an antibody's positions are (the footprints of the antibodies
	// shown); none to clear them
	void setFootprintMarks(const std::set<int> &marked);

	// vertices picked out, shown highlighted
	void setSelection(const std::set<int> &selection);
	const std::set<int> &selection() const
	{
		return _selected;
	}

	// the selection becomes excluded from random starts, shown dark blue
	void excludeSelection();
	void clearExclusions();
	// puts back the exclusions kept with the antigen (see Antigen::
	// excluded_start): each to the vertex at that position
	void loadExclusions();
	bool hasExclusions();
private:
	ArbitraryMap mappedAtoms();
	int removeHollows(ArbitraryMap &map);
	void growOrShrinkBorder(ArbitraryMap &map, int dir);
	void marchingCubes();
	// the slow part of that: the density the mesh is made from, which does
	// not touch the mesh itself (so is fine to wait for while it is on show)
	void buildDensity(GuiDensity &density);
	void normalsFromLines();

	std::string cachePath();
	bool loadCache();
	void saveCache();
	bool _loadedFromCache = false;
	bool _refined = false;
	std::atomic<bool> _stop{false};

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
	MeshFaces *_footprintFaces{};
	MeshSolid *_solid{};
	float _outward{0.f}; // 1 if the normals point outwards, else -1; 0 unknown
	void findOutward();

	// selected and excluded vertices are coloured over the usual colour
	void applyMarks();
	void keepExclusions();
	// the curvature is out of date (the vertices moved), and so are any
	// colours made from it
	void invalidateCurvature();
	void computeCurvature();
	std::vector<float> _curvZ;
	std::mutex _curvMutex;
	bool _colourCurv{false};
	// the vertices have moved or been remade: the excluded starts (kept as
	// positions with the antigen too) no longer mean anything, so they go
	void invalidateExclusions();
	std::set<int> _selected;
	std::set<int> _footprintMarks;
	std::set<int> _excluded;
	std::vector<glm::vec4> _baseColours;
	std::mutex _excludeMutex;
	bool _opaque{false};
	KdTree *_tree{};
};

#endif
