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

#ifndef __vagabond__HBondDataView__
#define __vagabond__HBondDataView__


#include <vagabond/core/Responder.h>
#include <vagabond/gui/elements/Scene.h>

class HBondData;
class ChooseHeader;

class HBondDataView : public Scene, public Responder<ChooseHeader>
{
public:
	HBondDataView(Scene *prev, HBondData *hbd);
	~HBondDataView();

	virtual void sendObject(std::string tag, void *object);
	virtual void setup();
	virtual void buttonPressed(std::string tag, Button *button = nullptr);

protected:

private:
	HBondData *_hbd = nullptr;

};

#endif