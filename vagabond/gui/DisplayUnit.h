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

#ifndef __vagabond__DisplayUnit__
#define __vagabond__DisplayUnit__

class Display;
class GuiAtom;
class GuiRefls;
class GuiDensity;
class AtomGroup;
class Diffraction;
class ArbitraryMap;
class Entity;
class Model;
class AtomMap;

struct DisplayUnit
{
public:
	DisplayUnit(Display *disp)
	{
		_parent = disp;
	}
	
	~DisplayUnit();
	
	void setOwnsAtoms()
	{
		_atoms_are_mine = true;
	}
	
	AtomGroup *atoms()
	{
		return _atoms;
	}
	
	GuiDensity *density()
	{
		return _guiDensity;
	}

	void tear(AtomMap *map);
	void refreshDensity(AtomMap *map, bool differences);

	void loadModel(Model *model);
	void loadAtoms(AtomGroup *group, Entity *entity = nullptr);
	void loadReflections(Diffraction *diff);

	void displayAtoms(bool hide_balls = false, bool hide_ribbon = false);

	void setMultiBondMode(bool mode);
	
	void displayDensityFromAtoms();
	void densityFromMap(ArbitraryMap *map);
	void makeMapFromDiffraction();
private:
	void resetDensityMap();
	void setVisuals();

	bool _atoms_are_mine = false;
	bool _map_is_mine = false;

	Model *_model = nullptr;
	AtomGroup *_atoms = nullptr;
	GuiAtom *_guiAtoms = nullptr;
	GuiRefls *_guiRefls = nullptr;
	GuiDensity *_guiDensity = nullptr;
	Diffraction *_diffraction = nullptr;
	Entity *_entity = nullptr;
	ArbitraryMap *_map = nullptr;
	Display *_parent = nullptr;
	
};

#endif
