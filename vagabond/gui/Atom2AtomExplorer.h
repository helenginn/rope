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

#ifndef __vagabond__Atom2AtomExplorer__
#define __vagabond__Atom2AtomExplorer__

#include <vagabond/gui/elements/Scene.h>
#include "CompareDistances.h"
#include <vagabond/utils/svd/PCA.h>
#include "AtomGroup.h"
#include "RAMovement.h"

struct prepare_atom_list;
struct Posular;

class Slider;
class Instance;
class MatrixPlot;
class ColourLegend;
class PositionalCluster;

class Atom2AtomExplorer : public Scene, public DragResponder
{
public:
	Atom2AtomExplorer(Scene *scene, Instance *instance,
	                  const RAMovement &movements);

	virtual void setup();

	virtual void finishedDragging(std::string tag, double x, double y);
	virtual void mousePressEvent(double x, double y, SDL_MouseButtonEvent button);
private:
	void sampleFromPlot(double x, double y);

	void update();

	Instance *_instance = nullptr;
	AtomGroup *_atoms = nullptr;

	void slider();
	void addPlot();


	const RAMovement _movement;
	float _motionScale = 1;
	float _colourScale = 1;
	
	prepare_atom_list *_atom2Vec;
	
	CompareDistances _cd;
	AtomFilter _filter{};
	
	PCA::Matrix _matrix;
	MatrixPlot *_plot = nullptr;
	std::mutex _mutex;
	
	Slider *_colourSlide = nullptr;
	ColourLegend *_legend = nullptr;
};

#endif
