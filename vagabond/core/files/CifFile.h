#ifndef __vagabond__CifFile__
#define __vagabond__CifFile__

#include "../../utils/os.h"
#ifdef OS_WINDOWS
	#define NOMINMAX  // avoid conflict with std::min and std::max in gemmi
#endif

#include <map>
#include <gemmi/cifdoc.hpp>
#include <vagabond/utils/FileReader.h>
#include "File.h"

/** \class CifFile
 * this is a subclass of File and will be able to handle any CIF or mmCIF
 * which has reflections or atoms or ligands or geometry information.
 * Will not handle MTZ or PDB */

class CifFile : public File
{
public:
	CifFile(std::string filename = "");

	virtual File::Type cursoryLook();
	virtual void parse();
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
	                int *indices, int req, int n = -1);
	bool identifyHeader(std::string *headers);
	bool identifyPairs(gemmi::cif::Document &doc, std::string keys[]);

	void parseFileContents(std::string filename);
	
	static std::string macroHeaders[];
	static std::string lengthHeaders[];
	static std::string angleHeaders[];
	static std::string torsionHeaders[];
	static std::string compHeaders[];
	static std::string compCoreHeaders[];
	static std::string reflHeaders[];
	static std::string unitCellKeys[];
	static std::string intTableKeys[];
	static std::string groupSymbolKeys[];

	static std::string ignoreChirals[];
	static std::string sideChain[];
	static std::string mainChain[];

	bool identifyHeader(gemmi::cif::Document &doc, std::string headers[]);
	bool identifyHeader(gemmi::cif::Loop &loop, std::string headers[]);
	
	int _atomNum = 1; /* for chemical components */
};

#endif
