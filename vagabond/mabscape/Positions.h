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

	// antibodies which have structural information (a fiducial with a
	// contact point), and so can be fixed at that position
	std::vector<std::string> structuralAntibodies() const;
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
	void setPosition(const std::string &name, glm::vec3 pos, 
	                 float snappiness = 0);
	const glm::vec3 &operator()(const std::string &name, 
	                            const glm::vec3 *closest = nullptr,
	                            bool shown = false) const;
	void loadAntibodiesInto(HasRenderables *bucket,
	                        std::vector<AbWatch *> &watches);
	
	int paramCount();
	void randomise();

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

	// For each non-fixed antibody, all its positions in these snapshots (every
	// symmetry copy of every result) are divided into as many clusters as
	// there are symmetry operators, starting from the first snapshot's
	// copies, so that cluster k belongs to operator k. Each copy is then
	// placed at the mean of its cluster, snapped to the nearest mesh vertex.
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

	std::vector<AntibodyPos> _positions;
	std::vector<glm::vec3> _raw;
	// what is on show while viewing; same size as _raw
	std::vector<glm::vec3> _shown;
	bool _viewing = false;

	void copyLiveToShown();
	std::map<std::string, AntibodyPos *> _lookup;
	FromMesh _fromMesh;
	RandomFromMesh _random;
};

#endif
