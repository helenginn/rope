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

#ifndef __File__File__
#define __File__File__

#include "../AtomGroup.h"
#include "../GeometryTable.h"
#include "../Reflection.h"
#include <vagabond/utils/FileReader.h>

class Atom;
class AtomContent;
class AtomGroup;
class Metadata;
class GeometryTable;
class Diffraction;
class RefList;

/** \class File
 * abstract class (due to need to implement File::parse() and 
 *  File::cursoryLook()) for any relevant file type in structural biology.
 * CifFile is one subclass. Apr 2022 plans for following MtzFile and PdbFile.
 * Subclasses need to assign at least some of the following:
 * - component atoms (for ligand/geometry cif files)
 * - macromolecular atoms (e.g. ATOM or HETATM records from PDB)
 * - populate a geometry table (for ligand/geometry cif files)
 * - unit cell
 * - space group
 * - reflection list (just as a RefList rather than Diffraction object)
 */

class File
{
public:
	File(std::string filename);
	virtual ~File();

	enum Type
	{
		Nothing = 0, /**< did not find any useful information */
		Geometry = 1 << 0, /**< found geometry restraints */
		CompAtoms = 1 << 1, /**< found ligand component atoms */
		MacroAtoms = 1 << 2, /**< found macromolecular atoms */
		UnitCell = 1 << 3, /**< found unit cell information */
		Symmetry = 1 << 4, /**< found space group/symmetry information */
		Reflections = 1 << 5, /**< found diffraction reflection data */
		Meta = 1 << 6, /**< contains metadata on file or model names */
		Json = 1 << 7, /**< json (environment?) file */
		Sequence = 1 << 8, /**< sequence-containing file */
	};

	enum Flavour
	{
		None = 0,
		Mtz,
		Pdb,
		Cif,
		Csv,
		Jsn,
		Fasta
	};
	
	static File *loadUnknown(std::string filename);
	static File *openUnknown(std::string filename);
	static Type typeUnknown(std::string filename, Flavour flav = None);

	/** determine bitwise contents of file */
	virtual Type cursoryLook() = 0;

	/** commence parsing and pull out atoms and geometry information 
	 * from file */
	virtual void parse() = 0;

	
	/** Warning: passes ownership of the AtomGroup onto the caller.
	 * @return AtomGroup containing all molecular component atoms found 
	 * in a geometry file */
	AtomContent *compAtoms();
	
	/** Warning: passes ownership of the AtomGroup onto the caller.
	 *  @return either macromolecular atoms or component atoms depending on
	 * type of file loaded */
	AtomContent *atoms();

	/** contains unit cell information
	 * 	@return true if unit cell assigned, false otherwise */
	bool hasUnitCell() const;

	/** returns unit cell information, in Angstroms and degrees.
	 * 	@return array of 6 doubles: a b c alpha beta gamma */
	std::array<double, 6> unitCell() const;
	
	/** returns assigned space group 
	 * 	@return CCP4 space group number or -1 if not assigned */
	int spaceGroupNum() const;
	
	/** returns assigned space group name as string
	 * 	@return CCP4 space group number or "" if not assigned */
	std::string spaceGroupName() const;

	/** returns any atom count assigned.
	 * @return number of macromolecular atoms if available; if not, number of 
	 *component atoms available; if not, 0 */
	const size_t atomCount() const;
	
	/** returns number of component atoms
	 * @return component atom count or 0 if not available */
	const size_t compAtomCount() const;
	
	/** returns list of reflections */
	RefList *reflectionList() const;

	/** returns list of reflections turned into Diffraction object */
	Diffraction *diffractionData() const;
	
	/** returns number of reflections assigned */
	const size_t reflectionCount() const
	{
		return _reflections.size();
	}
	
	/** @returns GeometryTable containing all geometry found in file */
	GeometryTable *geometryTable();
	
	void setGeometryTable(GeometryTable *other)
	{
		_table = other;
		_accessedTable = true;
	}
	
	/** Warning: passes ownership of the Metadata onto the caller.
	 * @returns Metadata containing data relating to model filename or 
	 * model id */
	Metadata *metadata();
	
	/** to only pull out a single monomer from a larger collection.
	 *	@param code three letter code */
	void pullOutCode(std::string code)
	{
		_code = code;
		to_upper(code);
	}
	
	enum KnotLevel
	{
		KnotNone,
		KnotLengths,
		KnotAngles,
		KnotTorsions
	};

	/** set whether File automatically assigns geometry to atoms found in
	 * file using standard geometry (proteins, nucleic acids) or additional 
	 * geometry in file.
	 * @param knot true (default) to tie atoms up */
	void setAutomaticKnot(KnotLevel kl)
	{
		_knot = kl;
	}
	
	const std::map<std::string, std::string> &namedSequences() const
	{
		return _sequences;
	}
	
	virtual void write(std::string filename) {}
protected:
	static Type checkUnknownType(std::string filename, Flavour needed);

	static bool compare_file_ending(const std::string &filename, 
	                                const std::string &comp, File::Flavour result);

	static Flavour flavour(std::string filename);
	static std::string toFilename(std::string filename);
	std::string _code;

	void changeFilename(std::string filename);

	std::string _filename;

	AtomGroup *_compAtoms = new AtomGroup();
	AtomGroup *_macroAtoms = new AtomGroup();
	GeometryTable *_table = nullptr;
	Metadata *_metadata = nullptr;
	
	std::map<std::string, std::string> _values;
	std::map<std::string, std::string> _sequences;
	std::vector<Reflection> _reflections;
	
	bool _accessedTable = false;
	bool _accessedMetadata = false;
	KnotLevel _knot = KnotTorsions;

};

#endif
