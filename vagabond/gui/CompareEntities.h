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

#ifndef __vagabond__CompareEntities__
#define __vagabond__CompareEntities__

#include <vagabond/gui/elements/Scene.h>
#include <vagabond/core/Responder.h>
#include <functional>
#include <set>

class PickAtomFromSequence;
class ChooseHeader;
class Instance;
class Residue;
class Model;
class Atom;

class CompareEntities : public Scene, public Responder<ChooseHeader>,
public Responder<PickAtomFromSequence>
{
public:
	CompareEntities(Scene *scene);

	virtual void setup();
	virtual void refresh();
	virtual void buttonPressed(std::string tag, Button *button = nullptr);
private:
	bool showEntityOptions();
	void showScoreOptions();
	bool allEntriesIdentical();
	std::string makeHeader();

	void chooseHeaders(int idx);
	void chooseAtom(int idx);
	void calculate();

	virtual void sendObject(std::string tag, void *object);

	bool proofAndShowEntity(TextButton *button, std::string &id, int idx);

	std::set<std::string> acceptableEntityList();

	enum Mode
	{
		Distance,
		Angle,
		None,
	};

	Mode _measureMode = None;
	size_t requiredCount();

	enum ChooseMode
	{
		ChooseEntity,
		ChooseAtom,
	};

	ChooseMode _chooseMode;

	struct Entry
	{
		typedef std::function<std::vector<std::pair<Atom *, Instance *>>
		(Model *model, Instance *reference)> FindAtom;

		std::string atomName;
		Residue *master;
		std::string id;
		std::string raw_id;
		
		std::string atom_desc();
		
		FindAtom as_find_function();
	};

	Entry _entries[3] = {Entry{}, Entry{}, Entry{}};
	int _active = -1;
	float _target = -1;

	bool proofAndShowAtom(TextButton *button, Entry &entry);
};

#endif
