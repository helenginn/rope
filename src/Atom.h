#ifndef __vagabond__Atom__
#define __vagabond__Atom__

#include "glm_import.h"
#include <mutex>

class Atom
{
public:
	Atom();

	struct AtomPlacement
	{
		glm::mat3x3 tensor;
		float b;
		glm::vec3 pos;
	}; 
	
	void setInitialPosition(glm::vec3 pos, float b, 
	                        glm::mat3x3 tensor = glm::mat3(1.f));
	
	const float &initialBFactor()
	{
		return _initial.b;
	}
	
	const glm::vec3 &initialPosition()
	{
		return _initial.pos;
	}
	
	const float &derivedBFactor()
	{
		return _derived.b;
	}
	
	const glm::vec3 &derivedPosition()
	{
		return _derived.pos;
	}
	
	void setAtomName(std::string name);
	
	std::string atomName()
	{
		return _atomName;
	}
	
	/** set atom number as sequentially numbered in PDB files.
	 *  throws exception if number is negative. */
	void setAtomNum(int num);
	
	const int &atomNum()
	{
		return _atomNum;
	}
	
	const int &residueNumber()
	{
		return _residueNumber;
	}
	
	void setResidueNumber(int num)
	{
		_residueNumber = num;
	}
	
	void setHetatm(bool hetatm)
	{
		_hetatm = hetatm;
	}
	
	void setOccupancy(float occ)
	{
		_occupancy = occ;
	}
	
	const bool &hetatm()
	{
		return _hetatm;
	}
	
	const float &occupancy()
	{
		return _occupancy;
	}

	/** sets element symbol; forces upper case. Throws exception
	 * 	if not alphabetical or empty. */
	void setElementSymbol(std::string ele);
	
	std::string elementSymbol()
	{
		return _ele;
	}
	
	void lockMutex()
	{
		_mutex.lock();
	}
	
	void unlockMutex()
	{
		_mutex.unlock();
	}
	
	bool positionChanged();
	bool fishPosition(glm::vec3 *pos);
private:
	void changedPosition();

	AtomPlacement _initial;
	AtomPlacement _derived;

	bool _setupInitial;
	bool _changedPosition;

	bool _hetatm;
	float _occupancy;
	int _residueNumber;
	int _atomNum;

	std::string _ele;
	std::string _atomName;
	std::mutex _mutex;
};

#endif
