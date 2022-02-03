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
	Cif2Geometry(std::string filename = "");
	void changeFilename(std::string filename);

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
	
	void setAutomaticKnot(bool knot)
	{
		_knot = true;
	}
	
	/** Warning: passes ownership of the AtomGroup onto the caller.
	 * @returns AtomGroup containing all molecular component atoms found 
	 * in a geometry file */
	AtomGroup *compAtoms() 
	{
		_accessedCompAtoms = true;
		return _compAtoms;
	}
	
	AtomGroup *atoms();
	
	const size_t compAtomCount() const;
	const size_t atomCount() const;
	
	/** Warning: passes ownership of the Table onto the caller.
	 * @returns GeometryTable containing all geometry found in file */
	GeometryTable *geometryTable()
	{
		_accessedTable = true;
		return _table;
	}
private:
	void processLoop(gemmi::cif::Loop &loop);
	bool processLoopAsCompAtoms(gemmi::cif::Loop &loop);
	bool processLoopAsLengths(gemmi::cif::Loop &loop);
	bool processLoopAsAngles(gemmi::cif::Loop &loop);
	bool processLoopAsTorsions(gemmi::cif::Loop &loop);
	bool processLoopAsChirals(gemmi::cif::Loop &loop);

	bool getHeaders(gemmi::cif::Loop &loop, std::string *headers, 
	                int *indices, int n);

	void parseFileContents(std::string filename);

	std::string _filename;
	std::string _code;

	AtomGroup *_compAtoms;
	AtomGroup *_macroAtoms;
	GeometryTable *_table;
	
	bool _accessedCompAtoms;
	bool _accessedMacroAtoms = false;
	bool _accessedTable;
	bool _knot;
};

#endif
