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

#ifndef __vagabond__EntitySequenceView__
#define __vagabond__EntitySequenceView__

#include <thread>

#include "PickAtomFromSequence.h"
#include <thread>

class Entity;
class Metadata;
class ImageButton;

class EntitySequenceView : public PickAtomFromSequence
{
public:
	EntitySequenceView(Scene *prev, IndexedSequence *sequence);
	
	void setEntity(Entity *ent);
	virtual void setup();

	virtual void buttonPressed(std::string tag, Button *button = nullptr);
private:
	void handleAtomName(std::string name);
	void queueMetadataForShow(Metadata *md);
	void calculateDistance();
	void calculateAngle();
	void distanceButton();
	void angleButton();
	void confirmAtom();
	void wipe();

	void stop();
	void prepareDistances();
	void prepareAngles();

	enum Stage
	{
		Nothing,
		ChosenFirst,
		ChosenSecond,
	};

	Stage _stage = Nothing;
	std::string _candidate;
	
	Entity *_entity = nullptr;
	std::string _first;
	std::string _second;
	std::string _third;
	Residue *_aRes = nullptr;
	Residue *_bRes = nullptr;
	Residue *_cRes = nullptr;
	ImageButton *_modeButton = nullptr;
	
	std::thread *_worker = nullptr;
	
	enum Mode
	{
		Ruler,
		Angle,
	};

	Mode _mode = Ruler;
};

#endif
