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

#ifndef __vagabond__DistanceMaker__
#define __vagabond__DistanceMaker__

#include "SequenceView.h"

class Entity;

class DistanceMaker : public SequenceView
{
public:
	DistanceMaker(Scene *prev, IndexedSequence *sequence);
	
	void setEntity(Entity *ent)
	{
		_entity = ent;
	}

	virtual void addExtras(TextButton *t, Residue *r);
	virtual void handleResidue(Button *button, Residue *r);
	virtual void buttonPressed(std::string tag, Button *button = nullptr);
private:
	void handleAtomName(std::string name);
	void calculateDistance();
	void confirmAtom();

	enum Stage
	{
		Nothing,
		ChosenFirst,
	};

	Stage _stage = Nothing;
	Residue *_curr;
	std::string _candidate;
	
	Entity *_entity = nullptr;
	std::string _first;
	std::string _second;
	Residue *_aRes = nullptr;
	Residue *_bRes = nullptr;
};

#endif
