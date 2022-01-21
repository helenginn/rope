#ifndef __vagabond__Cif2Geometry__
#define __vagabond__Cif2Geometry__

#include <string>
#include <gemmi/cif.hpp>
#include "FileReader.h"

class Atom;
class AtomGroup;
class GeometryTable;

class Cif2Geometry
{
public:
	Cif2Geometry(std::string filename);
	~Cif2Geometry();
	
	/** to only pull out a single monomer from a larger collection.
	 *	@param code three letter code */
	void pullOutCode(std::string code)
	{
		_code = code;
		to_upper(code);
	}

	/** commence parsing and pull out atoms and geometry information 
	 * from file */
	void parse();
	
	/** Warning: passes ownership of the AtomGroup onto the caller.
	 * @returns AtomGroup containing all atoms found in geometry file */
	AtomGroup *atoms() 
	{
		_accessedAtoms = true;
		return _atoms;
	}
	
	/** Warning: passes ownership of the Table onto the caller.
	 * @returns GeometryTable containing all geometry found in file */
	GeometryTable *geometryTable()
	{
		_accessedTable = true;
		return _table;
	}
private:
	void processLoop(gemmi::cif::Loop &loop);

	std::string _filename;
	std::string _code;

	AtomGroup *_atoms;
	GeometryTable *_table;
	
	bool _accessedAtoms;
	bool _accessedTable;
};

#endif
