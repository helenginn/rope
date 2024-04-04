#ifndef __vagabond__Atom__
#define __vagabond__Atom__

#define _USE_MATH_DEFINES
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
	bool fishPosition(glm::vec3 *pos);
	
	/** get all derived positions of ensemble, conditional on immediate access 
	 * 	 by mutex lock. If inaccessible, pointer contents remain unchanged.
	 * 	@param pos pointer to Atom::WithPos storage for derived position.
	 * 	@returns true if position was successfully accessed, otherwise false. */
	bool fishPositions(WithPos *wp);
	
	void setCode(std::string code);
	
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
		return _colour / (float)_count;
	}
	
	void setAddedColour(float col)
	{
		_colour = col;
		_count = 1;
		changedPosition();
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
	
	void setSymmetryCopyOf(Atom *const &other, const std::string &note)
	{
		_symAtom = other;
		_symNote = note;
	}
	
	float charge();
	
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
	glm::mat4x4 _transform = glm::mat4(1.f);
	Cyclic *_cyclic = nullptr;

	BackboneType _backboneType = NoOverride;

	Atom *_symAtom = nullptr;
	std::string _symNote;
};


#endif
