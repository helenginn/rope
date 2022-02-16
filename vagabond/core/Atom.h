#ifndef __vagabond__Atom__
#define __vagabond__Atom__

#include "../utils/glm_import.h"
#include "HasBondstraints.h"
#include <mutex>
#include <vector>

class BondLength;

class Atom : public HasBondstraints
{
public:
	Atom();
	Atom(std::string code, std::string name);
	
	struct WithPos
	{
		std::vector<glm::vec3> samples;
		glm::vec3 ave;
	};

	/** summary (average) of atom placement */
	struct AtomPlacement
	{
		glm::mat3x3 tensor;
		float b;
		WithPos pos;
	}; 
	
	/** @param pos 3D coordinate in real space
	 * @param b B factor in Angstroms squared
	 * @param tensor anisotropic tensor; values in PDB divided by 10000 */
	void setInitialPosition(glm::vec3 pos, float b = -1, 
	                        glm::mat3x3 tensor = glm::mat3(1.f));
	
	/** @returns initial B factor, usually as found in the PDB/mmCIF file */
	const float &initialBFactor() const
	{
		return _initial.b;
	}
	
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
	
	void setDerivedPosition(glm::vec3 &pos);
	void setDerivedPositions(WithPos &pos);
	
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
	const int &residueNumber()
	{
		return _residueNumber;
	}
	
	const std::string desc() const;
	
	/** @param num residue number within chain */
	void setResidueNumber(int num)
	{
		_residueNumber = num;
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
	const float &occupancy()
	{
		return _occupancy;
	}

	/** sets element symbol; forces upper case. 
	 * 	@param ele Element symbol e.g. K. Throws exception if not alphabetical
	 *	or empty. */
	void setElementSymbol(std::string ele);
	
	/** @returns upper case element symbol e.g. K */
	std::string elementSymbol()
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
	
	void setCode(std::string code);
	
	Atom *connectedAtom(int i);
	
	const std::string &code() const
	{
		return _code;
	}

	virtual Atom *atomIdentity()
	{
		return this;
	}
	
	/** coordination matrix for BondSequence.
	    @param isAnchor specifies if atom is first in sequence */
	 /* @returns matrix describing all connected partners */
	glm::mat4x4 coordinationMatrix(Atom *children[4], int count, 
	                               Atom *prev = NULL);

	void checkChirality(glm::mat4x4 &ret, Atom *prev, 
	                    Atom *children[4], const int count);
	
	/** set matrix used for initial placement of anchor point */
	void setTransformation(glm::mat4x4 transform);
	
	const glm::mat4x4 &transformation() const
	{
		return _transform;
	}
private:
	void changedPosition();

	AtomPlacement _initial{};
	AtomPlacement _derived{};

	bool _setupInitial = false;
	bool _changedPosition = false;

	bool _hetatm = false;
	float _occupancy = 1.;
	int _residueNumber = 1.;
	int _atomNum = 1.;

	std::string _ele;
	std::string _atomName;
	std::string _code;

	std::mutex _mutex;

	glm::mat4x4 _transform = glm::mat4(1.f);
};

#endif
