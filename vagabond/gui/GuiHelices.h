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

#ifndef __vagabond__GuiHelices__
#define __vagabond__GuiHelices__

#include "GuiRepresentation.h"
#include <set>

class GuiHelices : public GuiRepresentation
{
public:
	GuiHelices(GuiAtom *parent);
	virtual ~GuiHelices();

	virtual void watchAtom(Atom *a);
	virtual void watchAtomGroup(AtomGroup *ag);

	virtual void updateSinglePosition(Atom *a, glm::vec3 &p);
	virtual void updateMultiPositions(Atom *a, Atom::WithPos &wp);
	
	virtual void finishUpdate();
protected:
	virtual void extraUniforms();
private:
	size_t verticesPerAtom();
	size_t indicesPerAtom();
	
	struct NOPair
	{
		Atom *n;
		Atom *o;
		
		const bool operator<(const NOPair &other) const
		{
			return n->residueNumber() < other.n->residueNumber();
		}
	};
	
	typedef std::vector<NOPair> Run;
	
	struct Helix
	{
		Run run;
		std::vector<Atom *> cAlphas;
	};

	void extendToCarbonyls(Run &run);
	void extendNitrogensToCarbonylPairs(std::vector<Run> &runs);
	Atom *jumpResidueOtherName(Atom *atom, std::string other, int jump);

	void populateHelices();
	Run sweepCandidates();
	void pruneCandidates(Run &pairs);
	void ensureDirection(std::vector<Run> &runs, bool asc);
	std::vector<Helix> convertToHelices(std::vector<Run> &runs);
	void populateMapFromHelices();
	void drawHelix(Helix &h, std::vector<Snow::Vertex> &verts,
	               std::vector<GLuint> &indices);

	void cAlphasToWireFrame(Helix &h, std::vector<Snow::Vertex> &vs,
	                        std::vector<glm::vec3> &helix_dirs);

	std::vector<Run> findNitrogenRuns(std::vector<NOPair> &pairs);
	void extendRun(Run &run, std::vector<NOPair> &pairs);
	void mergeHelices();
	void purgeHelices();

	void makeHelixSlabs(std::vector<Snow::Vertex> &bez,
	                    std::vector<glm::vec3> &dirs,
	                    std::vector<Snow::Vertex> &verts,
	                    std::vector<GLuint> &indices);
	void makeHelixSlab(Snow::Vertex &origin, Snow::Vertex &previous,
	                   glm::vec3 dir,
	                   std::vector<Snow::Vertex> &verts,
	                   std::vector<GLuint> &indices, float taper);

	std::map<int, int> _helixToVertex;
	std::vector<Helix> _helices;
	std::vector<Atom *> _ns;
	std::vector<Atom *> _os;
	std::map<Atom *, int> _cAlphaMap;
	std::set<int> _helicesToUpdate;
};

#endif
