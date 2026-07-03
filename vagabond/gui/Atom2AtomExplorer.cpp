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
#include <vagabond/gui/elements/Image.h>
#include <vagabond/core/FlexibilityCache.h>
#include <vagabond/core/Flexibility.h>
#include "Atom2AtomExplorer.h"
#include "Atom3DPosition.h"
#include "RAMovement.h"
#include "AtomPosMap.h"
#include "MatrixPlot.h"
#include "Sequence.h"
#include "Polymer.h"
#include "Residue.h"
#include "HBondSelectorView.h"
#include "VdWSelectorView.h"


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
	                  AtomFilter &filter,
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
	AtomFilter _filter;
	Instance *const _instance;
};

Atom2AtomExplorer::Atom2AtomExplorer(Scene *scene, Instance *instance,
                                     const RAMovement &movements, std::string polymerTitle, bool bondFlag)
: Scene(scene), _movement(movements), _cd(false), _bondFlag(bondFlag)
{
	addTitle(polymerTitle);
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

	_cd.process(posList, true);

	const float &scale = _colourScale;
	_matrix = _cd.matrix();
	auto func = [scale](const float &f) -> float
	{
		return f * scale + 0.5;
	};

	PCA::do_op(_matrix, func);
	_plot->update();
	_plot->setCentre(0.5, 0.5);

	_legend->setCentre(0.0, 0.0);
	_legend->setLimits(-5 / _colourScale, 5 / _colourScale);
	_legend->setCentre(0.75, 0.5);
}

void Atom2AtomExplorer::addPlot()
{
	_plot = new MatrixPlot(_matrix, _mutex);
	// _plot->setRowLabels(labels) --> this will be done and preapre while figuring out the hbond overlay 
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
	addObject(_plot);

	if (_bondFlag)
    {
    	std::cout << "[DEBUG] bondFlag is true, looking up instance: " 
              << _instance << std::endl;
        Flexibility *flex = FlexibilityCache::instance().get(_instance);
    	std::cout << "[DEBUG] flex from cache: " 
              << (flex ? "FOUND" : "NULL") << std::endl;
        if (flex)
        {
            _flex = flex;
            setupBondTickBoxes();
            markerSlider();
        }
        else
        {
            std::cout << "[WARNING] bondFlag set but no Flexibility found "
                      << "in cache for this instance" << std::endl;
        }
    }
}

void Atom2AtomExplorer::slider()
{
	Slider *s = new Slider();
	s->setVertical(true);
	s->setDragResponder(this);
	s->resize(0.5);
	s->setup("Colour scale", 0, 100, 0.1);
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
	if (tag == "marker_size")
	{
		_markerSize = 0.01f + (x / 100.f) * 0.15f;
		updateMarkerSizes();
		return;
	}
	_colourScale = (100 - x)/0.2;
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


void Atom2AtomExplorer::setupBondTickBoxes()
{
    TickBoxes *tickboxes = new TickBoxes(this, this);

    auto toggle_hbonds = [this, tickboxes]()
    {
        bool ticked = tickboxes->isTicked("H-bonds");
        if (ticked)
        {
            if (_flex->getHBonds().empty())
            {
                std::cout << "[DEBUG] H-bonds ticked but empty" << std::endl;
                return;
            }
            showHBondSelector();
        }
    };

    auto toggle_vdw = [this, tickboxes]()
    {
        bool ticked = tickboxes->isTicked("VdW bonds");
        if (ticked)
        {
            if (_flex->getVdWBonds().empty())
            {
                std::cout << "[DEBUG] VdW ticked but empty" << std::endl;
                return;
            }
           showVdWSelector();
        }
    };

    tickboxes->addOption("H-bonds", toggle_hbonds, false);
    tickboxes->addOption("VdW bonds", toggle_vdw, false);
    tickboxes->setVertical(true);
    tickboxes->setOneOnly(false);
    tickboxes->arrange(0.05, 0.3, 0.25, 0.7);
    tickboxes->hideTickboxes();
    addObject(tickboxes);
}

void Atom2AtomExplorer::showHBondSelector()
{
    const auto &hbonds = _flex->getHBonds();
    if (hbonds.empty())
    {
        std::cout << "[DEBUG] No H-bonds to select from" << std::endl;
        return;
    }
    HBondSelectorView *selector = new HBondSelectorView(this, hbonds, nullptr);
    selector->setApplyCallback([this](const std::vector<int> &selected)
    {
    	for (Image *marker : _hbondMarkers)
    	{
    		removeObject(marker);
    		delete marker;
    	}
    	_hbondMarkers.clear();
    	_hbondMarkerIndices.clear();

		const auto &hbonds = _flex->getHBonds(); 
		for (int i : selected)
		{
				const auto &hbe = hbonds[i];
				int x = residueToMatrixIndex(hbe.Hydrogen->residueId());
				int y = residueToMatrixIndex(hbe.Acceptor->residueId());

				if (x < 0 || y < 0) { continue; }
				// overlay matrix
				placeMarker(x, y, HBondMarker);
		}
    });
	selector->show();
}

int Atom2AtomExplorer::residueToMatrixIndex(const ResidueId &resId)
{
	if (_residueToIndex.empty())
	{
		fillable<atompos> positions = (*_atom2Vec)();
		for (size_t i = 0; i < positions.size(); i++)
		{
			Atom *ca = positions[i]._atom; // _atom always the positions of the CA by default
			_residueToIndex[ca->residueId()] = i;

		}
	}
	if (_residueToIndex.count(resId)) // check if residue exitsts, so for example a ligand or a gap will skip that 
	{
		return _residueToIndex[resId];
	}
	std::cout << "[WARNING] residueToMatrixIndex: residue " << resId << " not found in matrix" << std::endl;
	return -1;
}

void Atom2AtomExplorer::placeMarker(int x_idx, int y_idx, MarkerType type)
{
	glm::vec3 min, max;
	_plot->boundaries(&min, &max);
  
	// convert indixes to normalised [0,1] coord
	// same logic as sampleFromPlot but reverse
	fillable<atompos> positions = (*_atom2Vec)();
	float nx = (float)x_idx / (float)positions.size();
	float ny = (float)y_idx / (float)positions.size();

	glm::vec3 screenPos = min + (max - min) * glm::vec3(nx, ny, 0);

	std::string image = (type == HBondMarker) ? 
        "assets/images/hydrogen_bond.png" : 
        "assets/images/eye.png";  // replace with your actual VdW image

    Image *marker = new Image(image);

	marker->resize(_originalMarkerSize);
	marker->setPosition(screenPos);

	addObject(marker);

	if (type == HBondMarker)
	{
		_hbondMarkers.push_back(marker);
		_hbondMarkerIndices.push_back({x_idx, y_idx});

	}
	else
	{
		_vdwMarkers.push_back(marker);
		_vdwMarkerIndices.push_back({x_idx, y_idx});
	}

	// symmetric marker at y_idx, x_idx
	if (x_idx != y_idx)
	{
	    glm::vec3 screenPosT = min + (max - min) * glm::vec3(ny, nx, 0);
	    Image *markerT = new Image(image);  // use image variable, not hardcoded
	    markerT->resize(_originalMarkerSize);
	    markerT->setPosition(screenPosT);
	    addObject(markerT);
	    if (type == HBondMarker)
	    {
	        _hbondMarkers.push_back(markerT);
	        _hbondMarkerIndices.push_back({y_idx, x_idx});
	    }
	    else
	    {
	        _vdwMarkers.push_back(markerT);
	        _vdwMarkerIndices.push_back({y_idx, x_idx});  // fixed: was {x_idx, y_idx}
	    }
	}

}

void Atom2AtomExplorer::markerSlider()
{

	Slider *s = new Slider();
	s->setVertical(true);
	s->setDragResponder(this);
	s->resize(0.1);
	s->setup("Marker size", 0, 100, 0.1);
	s->setStart(0.0, 0.5);
	s->setCentre(0.2, 0.5);
	s->setReturnTag("marker_size");
	addObject(s);
}

void Atom2AtomExplorer::updateMarkerSizes()
{
	float scale = _markerSize / _originalMarkerSize;
    for (Image *marker : _hbondMarkers)
        marker->resize(scale);
    
    for (Image *marker : _vdwMarkers)
        marker->resize(scale);

	_originalMarkerSize = _markerSize;
}

void Atom2AtomExplorer::showVdWSelector()
{
    const auto &vdw = _flex->getVdWBonds();
    if (vdw.empty())
    {
        std::cout << "[DEBUG] No VdWs to select from" << std::endl;
        return;
    }
    VdWSelectorView *selector = new VdWSelectorView(this, vdw, nullptr);
	selector->setApplyCallback([this](const std::vector<int> &selected)
	{
	    for (Image *marker : _vdwMarkers)
	    {
	        removeObject(marker);
	        delete marker;
	    }
	    _vdwMarkers.clear();
	    _vdwMarkerIndices.clear();

	    const auto &vdwBonds = _flex->getVdWBonds();
	    for (int i : selected)
	    {
	        const auto &vdw = vdwBonds[i];
	        int x = residueToMatrixIndex(vdw.Donor->residueId());
	        int y = residueToMatrixIndex(vdw.Acceptor->residueId());
	        if (x < 0 || y < 0) continue;
	        placeMarker(x, y, VdWMarker);
	    }
	});
    selector->show();
}

