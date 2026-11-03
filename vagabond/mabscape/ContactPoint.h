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

#ifndef __vagabond__ContactPoint__
#define __vagabond__ContactPoint__

#include <list>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <vagabond/utils/glm_import.h>
#include "Symmetry.h"

struct Fiducial;
class Antigens;
class Instance;
class AtomGroup;
struct Antigen;

class ContactPoint : public Symmetry
{
public:
	// orientations aligning worse than this (in Angstroms) are poor
	static constexpr float MAX_RMSD = 4.f;

	// How close, in Angstroms, an atom has to be to the other side for it to
	// be part of the interface, both for the midpoint of the interface and for
	// the footprint of the antibody on the antigen
	static constexpr float INTERFACE_REACH = 6.f;

	// The atoms making up an interface: the antibody atoms within {reach} of
	// the antigen and the antigen atoms within {reach} of the antibody. Both
	// groups are new, and for the caller to delete.
	static void interfaceAtoms(AtomGroup *antibody, AtomGroup *antigen,
	                           AtomGroup *&antibodyFace, AtomGroup *&antigenFace,
	                           float reach = INTERFACE_REACH);

	// The midpoint of an interface: the antibody atoms within {reach} of the
	// antigen and the antigen atoms within {reach} of the antibody are each
	// reduced to the one atom nearest the centre of its group, and the
	// midpoint is halfway between these two. False if either group is empty.
	static bool interfaceMidpoint(AtomGroup *antibody, AtomGroup *antigen,
	                              glm::vec3 &midpoint, 
	                              float reach = INTERFACE_REACH);

	/** The antibody which makes an interface, of the chains of a model which
	 *  may hold it more than once: the chain nearest to a point (in the model's
	 *  own frame) and those which touch it, such as the other chain of the
	 *  Fab. */
	static std::vector<Instance *> 
	antibodyAtInterface(const std::vector<Instance *> &chains,
	                    const glm::vec3 &reference);

	// find_instances is false to only make room for the mapping to be put
	// back from a saved one (restore()), which needs none of the models
	ContactPoint(Fiducial &fiducial, Antigens &antigens, 
	             bool find_instances = true);

	/** everything worked out by findMapping(), to be saved */
	nlohmann::json alignmentJson() const;
	/** puts back a mapping saved by alignmentJson(). False, and nothing
	 * changed, if it was made from different models or entities. */
	bool restore(const nlohmann::json &saved);

	void applyTransform(const glm::mat4x4 &which);
	void findMapping();

	/** The footprint of the antibody on the antigen: the positions, in the
	 *  antibody model as it was loaded, of the atoms of the antibody which
	 *  make the interface (those within {reach} of the antigen, of the one
	 *  antibody which makes the interface the orientations are of). Put through
	 *  an orientation's transform they are where the antibody's atoms are on
	 *  the mesh. Kept with the alignment. Loads the model; false if there is
	 *  nothing to work it out from. */
	bool computeFootprint(float reach = INTERFACE_REACH);
	bool hasFootprint() const
	{
		return _footprint.size() > 0;
	}
	const std::vector<glm::vec3> &footprint() const
	{
		return _footprint;
	}
private:
	void establishMidpoint();
	// what the mapping depends on: if these differ, it has to be redone
	nlohmann::json inputs() const;
public:
	// a problem found while mapping: an orientation which aligns poorly
	struct Warning
	{
		std::string text;
		// the matrix which took the antibody model onto the antigen model,
		// as it came out of the superposition
		glm::mat4x4 transform;
		float rmsd{};
	};

	// every orientation found, good or poor, as the matrix which took the
	// antibody model onto the antigen model.
	struct Orientation
	{
		glm::mat4x4 transform;
		float rmsd{};
	};

	const std::vector<Orientation> &orientations() const
	{
		return _orientations;
	}

	const std::vector<Warning> &warnings() const
	{
		return _warnings;
	}
private:

	Fiducial &_fiducial;
	Antigens &_antigens;
	Antigen *_chosen{};

	// instances belonging to fiducial for antibody
	std::vector<Instance *> _iFiducials;

	// instances belonging to fiducial mirroring antigen
	std::vector<Instance *> _iFidAntigens;

	// instances belonging to antigen
	std::vector<Instance *> _iAntigens;
	
	struct Entry
	{
		glm::mat4x4 transform;
		std::list<Instance *> fids;
		std::list<Instance *> ants;
	};

	std::vector<Entry> _entries;
	std::vector<Warning> _warnings;
	std::vector<Orientation> _orientations;
	std::vector<glm::mat4x4> _raw;
	std::vector<glm::vec3> _footprint;
	
};

#endif
