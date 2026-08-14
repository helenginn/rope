#ifndef __vagabond__Atom__
#define __vagabond__Atom__

#include <vagabond/utils/compatibility.h>
#include <math.h>
#include "../utils/glm_import.h"
#include "HasBondstraints.h"
#include "AtomPosMap.h"
#include "ResidueId.h"
#include <mutex>
#include <vagabond/utils/OpSet.h>
#include <vector>
#include "BackboneType.h"

struct BondNum
{
	Atom *atom;
	int num;
};

/** identifies a specific crystallographic symmetry transform (space
 *  group operator + lattice translation) - used as a map key so a
 *  symmetry-mate Atom can be resolved from its mother, and so two
 *  mates generated under the same transform (e.g. covalently bonded
 *  atoms of the same mate residue) can find each other via their
 *  shared mother atoms. Comparable/orderable rather than the free-text
 *  debug note (_symNote) this sits alongside, which stays purely for
 *  display. */
struct SymOp
{
	int op = 0;
	int i = 0;
	int j = 0;
	int k = 0;

	bool operator<(const SymOp &other) const
	{
		if (op != other.op) return op < other.op;
		if (i != other.i) return i < other.i;
		if (j != other.j) return j < other.j;
		return k < other.k;
	}

	bool operator==(const SymOp &other) const
	{
		return op == other.op && i == other.i && j == other.j &&
		       k == other.k;
	}
};

class Cyclic;
class BondLength;

class Atom : public HasBondstraints
{
public:
	Atom();
	Atom(const Atom &other);
	Atom(std::string code, std::string name);

	/** summary (average) of atom placement */
	struct AtomPlacement
	{
		glm::mat3x3 tensor;
		float b;
		float occ;
		WithPos pos;
	}; 
	
	typedef std::map<std::string, AtomPlacement> ConformerInfo;
	
	/** @param pos 3D coordinate in real space
	 * @param b B factor in Angstroms squared
	 * @param tensor anisotropic tensor; values in PDB divided by 10000 */
	void setInitialPosition(glm::vec3 pos, float b = -1, 
	                        glm::mat3x3 tensor = glm::mat3(1.f),
	                        float occupancy = 1.f);
	
	/** @returns initial B factor, usually as found in the PDB/mmCIF file */
	const float &initialBFactor() const
	{
		return _initial.b;
	}
	
	/** @returns initial position, usually as found in the PDB/mmCIF file */
	ConformerInfo &conformerPositions()
	{
		return _conformers;
	}
	
	OpSet<std::string> conformerList();
	
	/** @returns initial position, usually as found in the PDB/mmCIF file */
	const glm::vec3 &initialPosition() const
	{
		return _initial.pos.ave;
	}
	
	/** @returns updated derived B factor from vagabond model if in use,
	 * otherwise initial B factor from PDB/mmCIF file */
	const float &derivedBFactor() const
	{
		return _derived.b;
	}
	
	/** @returns updated derived position from vagabond model if in use,
	 * otherwise initial position from PDB/mmCIF file */
	glm::vec3 &derivedPosition() 
	{
		return _derived.pos.ave;
	}
	
	/** @returns updated derived position from vagabond model if in use,
	 * otherwise initial position from PDB/mmCIF file */
	const glm::vec3 &derivedPosition() const
	{
		return _derived.pos.ave;
	}
	
	const void addOtherPosition(std::string tag, glm::vec3 val)
	{
		_others[tag].pos.samples.push_back(val);
	}

	const WithPos &otherPositions(std::string tag)
	{
		return _others[tag].pos;
	}
	
	const void setOtherPosition(std::string tag, glm::vec3 val)
	{
		_others[tag].pos.ave = val;
	}

	bool hasOtherPosition(std::string tag)
	{
		return _others.count(tag);
	}
	
	void removeOtherPosition(std::string tag)
	{
		_others.erase(tag);
	}
	
	const glm::vec3 otherPosition(std::string tag)
	{
		return _others[tag].pos.ave;
	}
	
	void setDerivedPosition(const glm::vec3 &pos);
	void setDerivedPositions(WithPos &pos);
	
	void setDerivedBFactor(double b)
	{
		_derived.b = b;
	}
	
	/** @param name identifier for atom within monomer, e.g. CG2 in valine */
	void setAtomName(std::string name);
	
	/** @returns identifier for atom within monomer, e.g. CG2 in valine */
	const std::string &atomName() const
	{
		return _atomName;
	}
	
	/** set atom number as sequentially numbered in PDB files.
	 *  throws exception if number is negative. */
	void setAtomNum(int num);
	
	/** @returns unique atom number e.g. sequential ordering in PDB files */
	const int &atomNum()
	{
		return _atomNum;
	}
	
	/** @returns residue number within chain */
	const ResidueId &residueId() const
	{
		return _residueId;
	}
	
	const std::string desc() const;

	/** "A-Asn65" - chain, title-cased three-letter code, residue number -
	 *  short enough to read cleanly as an axis/legend label, unlike the
	 *  full desc() ("A-ASN65:CA,A" - includes the atom name and any
	 *  symmetry note). */
	const std::string shortResidueName() const;

	/** @param num residue number within chain */
	void setResidueId(ResidueId num)
	{
		_residueId = num;
	}

	/** @param string consisting of concatenated number and insertion code */
	void setResidueId(std::string num)
	{
		_residueId = ResidueId(num);
	}
	
	const int residueNumber() const
	{
		return _residueId.as_num();
	}
	
	/** @param hetatm if atom originally specified as HETATM in PDB/mmCIF file */
	void setHetatm(bool hetatm)
	{
		_hetatm = hetatm;
	}
	
	/** @returns if atom originally specified as HETATM in PDB/mmCIF file */
	const bool &hetatm()
	{
		return _hetatm;
	}
	
	/** @param float occupancy for atom, usually between 0. and 1.f */
	void setOccupancy(float occ)
	{
		_occupancy = occ;
		if (_occupancy < 0)
		{
			_occupancy = 0; 
		}
	}

	/** @returns occupancy for atom, usually between 0. and 1.f */
	const float &occupancy() const
	{
		return _occupancy;
	}

	/** @returns sum of occupancies of all alternate conformers */
	float occupancy_sum() const;

	/** sets element symbol; forces upper case. 
	 * 	@param ele Element symbol e.g. K. Throws exception if not alphabetical
	 *	or empty. */
	void setElementSymbol(std::string ele);
	
	void setChain(std::string id)
	{
		_chain = id;
	}
	
	const std::string &chain() const
	{
		return _chain;
	}
	
	/** @returns upper case element symbol e.g. K */
	const std::string &elementSymbol() const
	{
		return _ele;
	}
	
	/** locks mutex when atom position data should not be accessed from other
	 * threads. */
	void lockMutex()
	{
		_mutex.lock();
	}
	
	/** unlocks mutex when atom position data may be accessed from other
	 * threads. */
	void unlockMutex()
	{
		_mutex.unlock();
	}
	
	/** @returns true if derived position has been changed before
	 * 	Atom::fishPosition has been subsequently called */
	bool positionChanged();
	
	/** get derived position, conditional on immediate access by mutex lock
	 *  if inaccessible, pointer contents remain unchanged.
	 * 	@param pos pointer to glm::vec3 storage for derived position.
	 * 	@returns true if position was successfully accessed, otherwise false. */
	bool fishPosition(glm::vec3 *pos, bool *hidden);
	
	/** get all derived positions of ensemble, conditional on immediate access 
	 * 	 by mutex lock. If inaccessible, pointer contents remain unchanged.
	 * 	@param pos pointer to Atom::WithPos storage for derived position.
	 * 	@returns true if position was successfully accessed, otherwise false. */
	bool fishPositions(WithPos *wp, bool *hidden);
	
	void setCode(std::string code);
	
	void disconnectAtom(Atom *);
	Atom *connectedAtom(int i) const;
	bool isConnectedToAtom(Atom *a) const;
	
	const std::string &code() const
	{
		return _code;
	}

	virtual Atom *atomIdentity()
	{
		return this;
	}
	
	bool isMainChain() const;
	bool isCoreMainChain() const;
	static bool isMainChain(std::string name);
	static bool isCoreMainChain(std::string atomName);
	
	void setOverrideType(const BackboneType &type)
	{
		_backboneType = type;
	}

	const BackboneType &backboneType() const
	{
		return _backboneType;
	}
	
	bool isReporterAtom() const;
	static bool isReporterAtom(std::string name);
	
	/** coordination matrix for BondSequence.
	    @param isAnchor specifies if atom is first in sequence */
	 /* @returns matrix describing all connected partners */
	glm::mat4x4 coordinationMatrix(Atom *children[4], int count, 
	                               Atom *prev = nullptr);

	void checkChirality(glm::mat4x4 &ret, Atom *prev, 
	                    Atom *children[4], const int count);
	
	/** set matrix used for initial placement of anchor point */
	void setTransformation(glm::mat4x4 transform);
	void setAbsoluteTransformation(glm::mat4x4 transform);
	
	int bondsBetween(Atom *end, int maxBonds, bool long_way = false);

	const glm::mat4x4 &transformation() const
	{
		return _transform;
	}
	
	const bool &isTransformed() const
	{
		return _transformed;
	}
	
	float addedColour()
	{
		if (_count == 0) return 0;
		return _colour / (float)_count;
	}
	
	void setAddedColour(float col)
	{
		_colour = col;
		_count = 1;
		changedPosition();
	}
	
	void clearColour()
	{
		_colour = 0;
		_count = 0;
	}
	
	bool isHidden()
	{
		return _hidden;
	}
	
	void setHidden(const bool &hidden)
	{
		bool same = (_hidden == hidden);
		_hidden = hidden;
		
		if (!same)
		{
			changedPosition();
		}
	}
	
	void addToColour(float add)
	{
		_colour += add;
		_count++;
	}

	void setCyclic(Cyclic *cyc)
	{
		_cyclic = cyc;
	}
	
	Cyclic *cyclic()
	{
		return _cyclic;
	}
	
	void setSymmetryCopyOf(Atom *const &other, const SymOp &op,
	                       const std::string &note)
	{
		_symAtom = other;
		_symOp = op;
		_symNote = note;
		other->registerSymmetryEquivalent(op, this);
	}

	const std::string &symNote() const
	{
		return _symNote;
	}

	const SymOp &symOp() const
	{
		return _symOp;
	}

	/** records that `equivalent` is this atom's own copy under `op` -
	 *  called automatically by setSymmetryCopyOf() on the mother atom,
	 *  so this only needs calling directly for an atom that isn't itself
	 *  a mate (e.g. an asymmetric-unit atom registering its own mates). */
	void registerSymmetryEquivalent(const SymOp &op, Atom *const &equivalent)
	{
		_symEquivalents[op] = equivalent;
	}

	/** this atom's own copy under `op`, or nullptr if none is currently
	 *  loaded (e.g. outside whatever radius pulled symmetry mates in). */
	Atom *symmetryEquivalent(const SymOp &op) const
	{
		auto it = _symEquivalents.find(op);
		return (it == _symEquivalents.end()) ? nullptr : it->second;
	}

	float charge();
	
	void setSelected(const bool &sel)
	{
		_selected = sel;
	}
	
	const bool &isSelected() const
	{
		return _selected;
	}
	
	Atom *const &symmetryCopyOf() const
	{
		return _symAtom;
	}
private:
	void changedPosition();

	AtomPlacement _initial{};
	AtomPlacement _derived{};
	ConformerInfo _conformers{};
	
	std::map<std::string, AtomPlacement> _others{};

	bool _setupInitial = false;
	bool _changedPosition = false;

	bool _selected = false;
	bool _transformed = false;
	bool _hetatm = false;
	float _occupancy = 1.;
	ResidueId _residueId = {1};
	int _atomNum = 1.;

	std::string _chain;
	std::string _ele;
	std::string _atomName;
	std::string _code;

	std::mutex _mutex;

	float _colour = 0;
	int _count = 0;
	bool _hidden = false;
	glm::mat4x4 _transform = glm::mat4(1.f);
	Cyclic *_cyclic = nullptr;

	BackboneType _backboneType = NoOverride;

	Atom *_symAtom = nullptr;
	SymOp _symOp;
	std::string _symNote;
	std::map<SymOp, Atom *> _symEquivalents;
};


#endif
