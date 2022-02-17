#ifndef __vagabond__CifFile__
#define __vagabond__CifFile__

#include <string>
#include <gemmi/cif.hpp>
#include "../utils/FileReader.h"
#include "Reflection.h"

class Atom;
class AtomGroup;
class GeometryTable;
class RefList;

class CifFile
{
public:

	enum Type
	{
		Nothing = 0,
		Geometry = 1 << 0,
		CompAtoms = 1 << 1,
		MacroAtoms = 1 << 2,
		UnitCell = 1 << 3,
		Symmetry = 1 << 4,
		Reflections = 1 << 5,
	};

	CifFile(std::string filename = "");
	void changeFilename(std::string filename);

	~CifFile();
	
	/** to only pull out a single monomer from a larger collection.
	 *	@param code three letter code */
	void pullOutCode(std::string code)
	{
		_code = code;
		to_upper(code);
	}

	/** determine bitwise contents of file */
	Type cursoryLook();

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
	
	bool hasUnitCell() const;
	std::array<double, 6> unitCell() const;
	int spaceGroupNum() const;
	
	const size_t compAtomCount() const;
	const size_t atomCount() const;
	
	RefList *reflectionList() const;
	
	const size_t reflectionCount() const
	{
		return _reflections.size();
	}
	
	/** Warning: passes ownership of the Table onto the caller.
	 * @returns GeometryTable containing all geometry found in file */
	GeometryTable *geometryTable()
	{
		_accessedTable = true;
		return _table;
	}
private:
	void processPair(gemmi::cif::Pair &pair);
	void processLoop(gemmi::cif::Loop &loop);
	bool processLoopAsCompAtoms(gemmi::cif::Loop &loop);
	bool processLoopAsLengths(gemmi::cif::Loop &loop);
	bool processLoopAsLengthLinks(gemmi::cif::Loop &loop);
	bool processLoopAsAngleLinks(gemmi::cif::Loop &loop);
	bool processLoopAsAngles(gemmi::cif::Loop &loop);
	bool processLoopAsTorsions(gemmi::cif::Loop &loop);
	bool processLoopAsChirals(gemmi::cif::Loop &loop);
	bool processLoopAsMacroAtoms(gemmi::cif::Loop &loop);
	bool processLoopAsReflections(gemmi::cif::Loop &loop);

	bool getHeaders(gemmi::cif::Loop &loop, std::string *headers, 
	                int *indices, int n);
	bool identifyHeader(std::string *headers);

	void parseFileContents(std::string filename);

	std::string _filename;
	std::string _code;

	AtomGroup *_compAtoms;
	AtomGroup *_macroAtoms;
	GeometryTable *_table;
	
	std::map<std::string, std::string> _values;
	std::vector<Reflection> _reflections;
	
	bool _accessedCompAtoms;
	bool _accessedMacroAtoms = false;
	bool _accessedTable;
	bool _knot;
	
	static std::string macroHeaders[];
	bool identifyHeader(gemmi::cif::Document &doc, std::string headers[]);
	bool identifyHeader(gemmi::cif::Loop &loop, std::string headers[]);
};

#endif
