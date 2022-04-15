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

#ifndef __vagabond__PdbFile__
#define __vagabond__PdbFile__

#include "File.h"

namespace gemmi
{
	struct Model;
	struct Residue;
	struct Chain;
	struct Atom;
}

class PdbFile : public File
{
public:
	PdbFile(std::string filename = "");

	virtual File::Type cursoryLook();
	virtual void parse();
private:
	struct AtomInfo
	{
		std::string chain;
		std::string res;
		std::string seqstr;
		bool isHetatm;

	};

	void processModel(gemmi::Model &m);
	void processResidue(gemmi::Residue &r, AtomInfo &ai);
	void processAtom(gemmi::Atom &a, AtomInfo &ai);

	void getStandardGeometry();
	void parseFileContents();

};

#endif
