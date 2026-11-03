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

#ifndef __vagabond__Positions__
#define __vagabond__Positions__

#include <vagabond/utils/glm_import.h>
#include "Symmetry.h"
#include "AbWatch.h"
#include <functional>
#include <mutex>
#include <map>
#include <deque>

struct Competition;
struct Fiducial;
struct Antigen;
struct Mab;

class Symmetry;
class HasRenderables;

class Positions
{
public:
	typedef std::function<void(const glm::vec3 &, glm::vec3 &, int &)> FromMesh;
	typedef std::function<glm::vec3()> RandomFromMesh;

	Positions(Antigen &antigen, const Competition &comp, Mab &mab,
	          const FromMesh &fm, const RandomFromMesh &rm);
	~Positions();
	
	bool isFixed(const std::string &name) const;

	// Antibodies in the naughty corner are left alone entirely: not refined,
	// not randomised, kept out of every snapshot and out of every comparison,
	// clustering and average of results (older results may still hold them).
	// Shared by everything which looks at results, so set from the one place
	// the corner changes; Positions sets it from the competition when made.
	static void setIgnored(const std::vector<std::string> &names);
	static bool ignored(const std::string &name);

	// antibodies which have structural information (a fiducial with a
	// contact point), and so can be fixed at that position
	std::vector<std::string> structuralAntibodies() const;
	// the fiducial behind a structural antibody, or null
	Fiducial *fiducialFor(const std::string &name) const;
	// where each symmetry copy of an antibody sits in its structure; empty
	// without structural information
	// (optionally snapped to the nearest mesh vertex, as a fixed antibody is)
	std::vector<glm::vec3> structuralCopies(const std::string &name,
	                                        bool snapped = false) const;

	// What chance alone would give for the distance between two positions:
	// the mean over random pairs of mesh vertices of the distance from one to
	// the nearest symmetry copy of the other (as every antibody is compared
	// with its symmetry copies). Sampled, so it varies a little each time.
	float randomDistance(int samples = 20000) const;
	// for this session only: fixes an antibody at its structural position, or
	// lets it be refined like any other. No effect without structural info.
	void setFixed(const std::string &name, bool fixed);

	// whether an antibody has positions here
	bool has(const std::string &name) const
	{
		return _lookup.count(name) > 0;
	}

	/** Gives an antibody which was left out (in the naughty corner when this
	 *  was made) positions, to start from a random one. Room was kept for
	 *  these at the start, so that nothing in use is moved; false if there
	 *  is none left. */
	bool addAntibody(const std::string &name);
	// the watches for one antibody, as loadAntibodiesInto() makes for all
	void loadAntibodyInto(HasRenderables *bucket, 
	                      std::vector<AbWatch *> &watches,
	                      const std::string &name);

	// the curvature z-score of each mesh vertex (see Mesh::curvatureZ()), for
	// asking about it where an antibody sits
	void setCurvatureZ(const std::vector<float> &z)
	{
		_curvZ = z;
	}

	bool hasCurvature() const
	{
		return _curvZ.size() > 0 && _fromMesh;
	}

	// the z-score of the mesh vertex nearest to a position
	float curvatureZ(const glm::vec3 &pos) const;
	// the antibodies with structural information which are fixed now, and
	// the setting of that from such a list (those with structure not on it
	// are freed)
	std::vector<std::string> fixedAntibodies() const;
	void setFixedAntibodies(const std::vector<std::string> &fixed);
	void setPosition(const std::string &name, glm::vec3 pos, 
	                 float snappiness = 0);
	const glm::vec3 &operator()(const std::string &name, 
	                            const glm::vec3 *closest = nullptr,
	                            bool shown = false) const;
	void loadAntibodiesInto(HasRenderables *bucket,
	                        std::vector<AbWatch *> &watches);
	
	int paramCount();
	void randomise();

	// Bulk reading for the refinement loop, avoiding a name lookup and lock
	// per position: every symmetry copy of every antibody in one vector,
	// with extent() saying which run of it belongs to an antibody. Not
	// locked, so only for the thread which is changing the positions, or
	// for when none is.
	const std::vector<glm::vec3> &copies(bool shown = false) const
	{
		return (shown ? _shown : _raw);
	}

	// false if there is no such antibody
	bool extent(const std::string &name, int &start, int &num) const;

	// where one antibody (and all its symmetry copies) currently sits
	struct Placement
	{
		std::string name;
		bool fixed{};
		int meshIdx{-1};
		std::vector<glm::vec3> copies;
	};

	std::vector<Placement> snapshot() const;
	// puts every non-fixed antibody back where a snapshot() had it
	void apply(const std::vector<Placement> &placements);
	// Viewing: the antibodies on screen can show a Result or an average of
	// Results instead of the live positions, which go on changing under
	// refinement without being seen. The live positions are still what
	// the model is made from, unless `shown` is asked for above.
	bool viewing() const
	{
		return _viewing;
	}

	void view(const std::vector<Placement> &placements);
	void viewAverage(const std::vector<const std::vector<Placement> *> &all);
	// back to watching the live positions
	void resumeLive();

	// places each symmetry copy of an antibody individually (snapped to mesh)
	void setCopies(const std::string &name, const std::vector<glm::vec3> &copies);
	struct Clusters
	{
		std::vector<glm::vec3> centres; // one per symmetry operator
		std::vector<glm::vec3> points; // every copy in every snapshot
		std::vector<size_t> member; // the cluster (operator) of each point
		std::vector<size_t> resultOf; // which snapshot each point came from
	};

	// divides one antibody's positions into a cluster per symmetry operator
	// (see applyAverage()); false if it is fixed or absent
	static bool cluster(const std::vector<const std::vector<Placement> *> &all,
	                    const std::string &name, Clusters &out);

	// How sharply the clusters are separated, from 0 to 1: the silhouette of
	// every position, (b - a) / max(a, b) with a its distance to its own
	// cluster centre and b to the nearest other one, averaged and with
	// anything negative made 0. (Distances to the centres stand in for the
	// usual mean distances to every member, which would be quadratic in the
	// number of positions.) With a single cluster there is nothing to tell
	// apart, which counts as 1.
	static float silhouette(const Clusters &clusters);

	// the median of each cluster: the mesh vertex with the least total
	// distance to the vertices its members occupied (across all the results),
	// which need not be one of them. Falls back to the cluster's centre if it
	// has no members, and to the best of the occupied vertices if the mesh
	// vertices are not known (see setMeshVertices()).
	std::vector<glm::vec3> medoids(const Clusters &clusters) const;

	/** The footprint of each fixable fiducial on the mesh: the vertices within
	 *  5 Angstroms of an atom of the antibody making the interface, in every
	 *  orientation (see ContactPoint::computeFootprint()). For every vertex
	 *  of the mesh it keeps the nearest vertex of the footprint, which is
	 *  itself where it is on it. Needs the mesh vertices first. The models
	 *  of any fiducial without its footprint kept are loaded to make it
	 *  (and the json saved), the once. */
	void buildFootprints();

	struct Footprint
	{
		std::vector<float> dist; // from each mesh vertex to the footprint
		std::vector<int> nearest; // its nearest footprint vertex
		glm::vec3 centroid{};
		size_t count{};
	};

	const Footprint *footprintOf(const std::string &name) const
	{
		auto it = _footprints.find(name);
		return (it == _footprints.end() ? nullptr : &it->second);
	}

	// the vertices on the footprint of an antibody (none without one)
	std::vector<int> footprintVertices(const std::string &name) const;

	/** How far a position is from the footprint of an antibody (straight
	 *  line, to the nearest vertex of it; 0 on it), and the unit direction
	 *  away from it (from the middle of it if on it). False without one. */
	bool footprintDistance(const std::string &name, const glm::vec3 &pos,
	                       float &dist, glm::vec3 &away) const;

	// The same for lookups made over and over for one position: its nearest
	// mesh vertex worked out once (-1 if there is none), and a footprint
	// already found.
	int vertexAt(const glm::vec3 &pos) const;
	float curvatureZAtVertex(int vertex) const
	{
		return (vertex >= 0 && vertex < (int)_curvZ.size() ? _curvZ[vertex] : 0.f);
	}
	bool footprintDistance(const Footprint &footprint, const glm::vec3 &pos,
	                       int vertex, float &dist, glm::vec3 &away) const;

	// every vertex of the mesh, for medoids() to choose from
	void setMeshVertices(const std::vector<glm::vec3> &vertices)
	{
		_meshVertices = vertices;
	}

	// For each non-fixed antibody, all its positions in these snapshots (every
	// symmetry copy of every result) are divided into as many clusters as
	// there are symmetry operators, starting from the first snapshot's
	// copies, so that cluster k belongs to operator k. Each copy is then
	// placed at the median occupied vertex of its cluster (see medoids()).
	void applyAverage(const std::vector<const std::vector<Placement> *> &all);
private:
	Antigen &_antigen;
	Mab &_mab;
	
	struct AntibodyPos
	{
		void setPosition(std::vector<glm::vec3> &raw, glm::vec3 ref, 
		                 const FromMesh &fromMesh);
		
		const glm::vec3 &closest_to(const std::vector<glm::vec3> &raw,
		                            const glm::vec3 *other) const;
		void setShown(std::vector<glm::vec3> &shown, 
		              const std::vector<glm::vec3> &copies, 
		              const FromMesh &fromMesh);
		
		~AntibodyPos()
		{

		}
		
		AbWatch *icoAbWatch(const AbWatch::Source &source, int offset) const;

		Fiducial *fid{};
		std::string name{};
		int start_idx = -1; // for accessing _raw
		int num = 1;
		Symmetry sym{};
		unsigned int version = 0;
		std::mutex *mut{};
		int meshIdx = -1;
		bool fixed = false; // this session; needs fid to be true
		unsigned int shownVersion = 0;
	};

	// a deque, so that what points to one stays valid as antibodies are added
	std::deque<AntibodyPos> _positions;
	std::vector<glm::vec3> _raw;
	// what is on show while viewing; same size as _raw
	std::vector<glm::vec3> _shown;
	bool _viewing = false;

	void copyLiveToShown();
	void loadOne(const AntibodyPos &ap, HasRenderables *bucket,
	             std::vector<AbWatch *> &watches);
	std::map<std::string, AntibodyPos *> _lookup;
	std::vector<glm::vec3> _meshVertices;
	FromMesh _fromMesh;
	std::vector<float> _curvZ;
	std::map<std::string, Footprint> _footprints;
	RandomFromMesh _random;
};

#endif
