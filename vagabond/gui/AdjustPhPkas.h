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

#ifndef __vagabond__AdjustPhPkas__
#define __vagabond__AdjustPhPkas__

#include <vagabond/gui/elements/Scene.h>
#include <vagabond/core/Item.h>

class Network;
class Slider;
class Text;
class LineGroup;

// left-hand list of pH/pKa entries (Item/LineGroup, same idiom as
// CommunicationAnalysis::prepareGroups()) linked to Model::
// protonSettings() (CustomProtonSettings.h) - "Recalculate proton
// network" itself lives one level up, on EditModel, since it applies to
// every value here at once rather than to a single selected entry.
class AdjustPhPkas : public Scene
{
public:
	AdjustPhPkas(Scene *prev, Network &network);

	virtual void setup();
private:
	// residue code ("ARG" etc.) for a side chain entry, or empty for pH
	// itself - selects which of CustomProtonSettings' values the shared
	// slider (see AdjustPhPkas.cpp's own comment) currently edits.
	void selectItem(Item *item, const std::string &code,
	                const std::string &label);

	Network &_network;

	Item _parent;
	LineGroup *_lg = nullptr;
	Item *_selectedItem = nullptr;

	std::string _currentCode;
	Text *_label = nullptr;
	Text *_pkaCaption = nullptr;
	Slider *_slider = nullptr;
};

#endif
