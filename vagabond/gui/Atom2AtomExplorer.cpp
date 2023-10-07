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

#include <vagabond/gui/elements/Slider.h>
#include <vagabond/gui/ColourLegend.h>
#include "Atom2AtomExplorer.h"
#include "Atom3DPosition.h"
#include "RAMovement.h"
#include "AtomPosMap.h"
#include "MatrixPlot.h"
#include "Sequence.h"
#include "Polymer.h"
#include "Residue.h"

template <typename Obj>
struct multiply_by
{
	multiply_by(const float &scale) : _scale(scale) {}

	Obj operator()(const Obj &other) const
	{
		return other * _scale;
	}

	const float &_scale;
};

struct atompos
{
	atompos(Atom *atom, const RAMovement &movements,
	        Instance *instance) : _atom(atom)
	{
		int idx = -1;
		std::vector<Atom3DPosition> headers = movements.headers_only();
		for (const Atom3DPosition &pos : headers)
		{
			idx++;
			if (pos.fitsAtom(_atom, instance))
			{
				_idx = idx;
				break;
			}
		}
	}

	AtomWithPos operator()()
	{
		AtomWithPos awp;
		awp.atom = _atom;
		awp.wp.ave = _atom->initialPosition();
		awp.wp.target = _atom->initialPosition();
		return awp;
	}

	template <class F>
	AtomWithPos operator()(const std::vector<Posular> &vals, const F &op)
	{
		AtomWithPos awp = (*this)();
		awp.wp.ave += op(vals[_idx]);
		return awp;
	}

	Atom *_atom;
	int _idx = -1;
};

template <typename T>
struct fillable : public std::vector<T>
{
	fillable() { }
	
	template <class F>
	std::vector<AtomWithPos> operator()(const std::vector<Posular> &vals,
	                                    const F &op)
	{
		std::vector<AtomWithPos> ret;
		for (atompos &entry : *this)
		{
			ret.push_back(entry(vals, op));
		}
		return ret;
	}
};

struct prepare_atom_list
{
	prepare_atom_list(AtomGroup *const &group, 
	                  const RAMovement &movement,
	                  CompareDistances::AtomFilter &filter,
	                  Instance *const &instance)
	: _group(group), _movement(movement), _filter(filter), _instance(instance)
	{

	}
	
	fillable<atompos> operator()()
	{
		if (_filled.size())
		{
			return _filled;
		}

		fillable<atompos> atoms;
		std::cout << "here" << std::endl;

		for (size_t i = 0; i < _group->size(); i++)
		{
			Atom *atom = (*_group)[i];
			
			if (!_filter || _filter(atom))
			{
				atoms.push_back(atompos(atom, _movement, _instance));
			}
		}

		_filled = atoms;
		return atoms;
	}

	fillable<atompos> _filled;

	AtomGroup *const &_group;
	const RAMovement &_movement;
	CompareDistances::AtomFilter _filter;
	Instance *const _instance;
};

Atom2AtomExplorer::Atom2AtomExplorer(Scene *scene, Instance *instance,
                                     const RAMovement &movements)
: Scene(scene), _movement(movements), _cd(false)
{
	_instance = instance;
	_instance->load();
	_atoms = _instance->currentAtoms();
	
	_filter = [](Atom *const &atom) -> bool
	{
		return atom->atomName() == "CA";
	};
	
	if (_instance->hasSequence())
	{
		Polymer *pol = static_cast<Polymer *>(_instance);
		_atom2Vec = new prepare_atom_list(_atoms, _movement, _filter, _instance);
	}

}

void Atom2AtomExplorer::update()
{
	prepare_atom_list &prep = *_atom2Vec;
	fillable<atompos> positions = prep();

	_cd.clearMatrix();
	std::vector<Posular> vals = _movement.storage_only();
	AtomPosList posList = positions(vals, multiply_by<glm::vec3>(_motionScale));

	_cd.process(posList);

	const float &scale = _colourScale;
	_matrix = _cd.matrix();
	auto func = [scale](const float &f) -> float
	{
		return f * scale + 0.5;
	};

	PCA::do_op(_matrix, func);
	_plot->update();

	_legend->setCentre(0.0, 0.0);
	_legend->setLimits(-1.5 / _colourScale, 1.5 / _colourScale);
	_legend->setCentre(0.75, 0.5);
}

void Atom2AtomExplorer::addPlot()
{
	_plot = new MatrixPlot(_matrix, _mutex);
	addObject(_plot);

	ColourLegend *legend = new ColourLegend(_plot->legend()->scheme(), 
	                                        true, this);
	legend->disableButtons();
	legend->setTitle("Distance change, A");
	legend->setLimits(-1.5, 1.5);
	legend->setCentre(0.75, 0.5);
	addTempObject(legend);
	_legend = legend;
}

void Atom2AtomExplorer::setup()
{
	if (!_plot)
	{
		addPlot();
		slider();
	}
	
	update();
}

void Atom2AtomExplorer::slider()
{
	Slider *s = new Slider();
	s->setVertical(true);
	s->setDragResponder(this);
	s->resize(0.5);
	s->setup("Colour scale", 0, 100, 1);
	s->setStart(0.0, 0.9);
	s->setCentre(0.9, 0.5);
	s->setReturnTag("colour");
	_colourSlide = s;
	addObject(s);
}

void Atom2AtomExplorer::mousePressEvent(double x, double y, 
                                        SDL_MouseButtonEvent button)
{
	sampleFromPlot(x, y);
	Scene::mousePressEvent(x, y, button);
}

void Atom2AtomExplorer::finishedDragging(std::string tag, double x, double y)
{
	_colourScale = (100 - x) / 10;
	update();
}

void Atom2AtomExplorer::sampleFromPlot(double x, double y)
{
	double tx = x; double ty = y;
	convertToGLCoords(&tx, &ty);

	glm::vec3 v = glm::vec3(tx, ty, 0);
	glm::vec3 min, max;
	_plot->boundaries(&min, &max);

	v -= min;
	v /= (max - min);
	v.z = 0;

	if ((v.x < 0 || v.x > 1) || (v.y < 0 || v.y > 1))
	{
		return;
	}

	fillable<atompos> positions = (*_atom2Vec)();

	int x_idx = v.x * positions.size();
	int y_idx = v.y * positions.size();

	Atom *a = positions[x_idx]().atom;
	Atom *b = positions[y_idx]().atom;
	
	setInformation(a->desc() + ", " + b->desc());
}

