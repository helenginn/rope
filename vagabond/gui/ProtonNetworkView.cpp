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

#include "ProbeAtom.h"
#include "ProbeBond.h"
#include "ProtonNetworkView.h"
#include <vagabond/core/protonic/Probe.h>
#include <vagabond/core/PositionShifter.h>
#include <vagabond/gui/elements/FloatingText.h>
#include <vagabond/gui/elements/Menu.h>

ProtonNetworkView::ProtonNetworkView(Scene *scene, Network &network) 
: Scene(scene), Mouse3D(scene), IndexResponseView(scene), _network(network)
{
	_translation.z -= 50;
	_farSlab = 40;
	_slabbing = true;
}

ProtonNetworkView::~ProtonNetworkView()
{
	delete _shifter;

}

void ProtonNetworkView::findAtomProbes()
{
	for (AtomProbe *const &probe : _network.atomProbes())
	{
		ProbeAtom *text = new ProbeAtom(this, probe);
		addObject((FloatingText *)text);
		_textProbes[probe] = text;
		_allProbes.insert(probe);
		probe->setResponder(this);
		addIndexResponder(text);
	}

	for (HydrogenProbe *const &probe : _network.hydrogenProbes())
	{
		ProbeAtom *text = new ProbeAtom(this, probe);
		addObject((FloatingText *)text);
		_textProbes[probe] = text;
		_allProbes.insert(probe);
		probe->setResponder(this);
		addIndexResponder(text);
	}

	for (BondProbe *const &probe : _network.bondProbes())
	{
		ProbeBond *bond = new ProbeBond(this, probe);
		if (probe && probe->left().atom() && probe->right().atom() &&
		    (probe->left().atom()->elementSymbol() == "H" ||
		    probe->right().atom()->elementSymbol() == "H"))
		{
			continue;
		}
		addObject((Image *)bond);
		_bondProbes[probe] = bond;
		_allProbes.insert(probe);
		probe->setResponder(this);
		addIndexResponder(bond);
	}

	shiftToCentre(_network.centre(), 50);
	setMakesSelections();
	IndexResponseView::setup();
}

template <class Container>
OpSet<Probe *> selected_probes(const Container &container)
{
	OpSet<Probe *> done;
	for (auto it = container.begin(); it != container.end(); it++)
	{
		if (it->second->isSelected())
		{
			done.insert(it->first);
		}
	}

	return done;
}

void ProtonNetworkView::interactedWithNothing(bool left, bool hover)
{
	if (_active)
	{
		_active->setHighlighted(false);
		_active = nullptr;
	}

	if (_shiftPressed && left && !_moving)
	{
		// deselect everything
		deselect();
	}
	if (!_shiftPressed && !left && !_moving)
	{
		Menu *menu = new Menu(this);
		if (!_2D)
		{
			menu->addOption("arrange figure", [this]() { arrangeFigure(); });
		}
		else
		{
			menu->addOption("add to figure", [this]() { arrangeFigure(); });
		}

		menu->addOption("add neighbours", 
		                [this]() { expandSelectionToNeighbours(); });
		menu->addOption("complete residues", 
		                [this]() { completeResidues(false); });
		menu->addOption("complete to C-alpha", 
		                [this]() { completeResidues(true); });
		
		OpSet<Probe *> selected = selected_probes(_textProbes);
		selected += selected_probes(_bondProbes);
		if (selected.size() > 0)
		{
			// add option to remove
			auto hide_selected = [this, selected]()
			{
				if (_shifter) { _shifter->pause(); }
				for (Probe *probe : selected)
				{
					probe->setHide(-1, false);
					
					if (_textProbes.count(probe))
					{
						if (_shifter)
						{
							_shifter->removePointer(_textProbes[probe]);
						}
						_textProbes[probe]->selected(0, true);
					}
					else if (_bondProbes.count(probe))
					{
						if (_shifter)
						{
							_shifter->removePointer(_bondProbes[probe]);
						}
						_bondProbes[probe]->selected(0, true);
					}
				}
				if (_shifter) { _shifter->unpause(); }
			};

			menu->addOption("hide selection", hide_selected);
		}

		setMenu(menu);
	}

	setInformation("");
}

void ProtonNetworkView::arrangeFigure()
{
	auto make_getter = [](Probe *probe)
	{
		return [probe]() -> glm::vec3
		{
			return probe->position();
		};
	};

	auto make_init = [](Probe *probe)
	{
		return [probe]() -> glm::vec3
		{
			return probe->_init;
		};
	};

	auto make_setter = [](Probe *probe)
	{
		return [probe](const glm::vec3 &vec)
		{
			probe->setPosition(vec);
		};
	};

	// template this one
	auto make_tidy = []<class ProbeType> (ProbeType *probe)
	{
		return [probe]()
		{
			probe->updatePosition();
		};
	};
	
	_2D = true;
	
	if (!_shifter)
	{
		_shifter = new PositionShifter(getModel());
		_shifter->run();
	}
	
	std::function<void(Probe *, OpSet<void *> &)> get_probes_as_set;
	get_probes_as_set = [this, &get_probes_as_set]
	(Probe *base, OpSet<void *> &growing)
	{
		for (Probe *other : base->others())
		{
			if (_bondProbes.count(other))
			{
				Probe *deeper = _bondProbes[other]->probe();
				get_probes_as_set(deeper, growing);
			}
			if (_textProbes.count(other))
			{
				if (!other->is_absent())
				{
					growing.insert(_textProbes[other]);
				}
			}
		}
	};
	
	std::vector<std::function<void()>> tidyJobs;

	_shifter->pause();
	for (auto it = _textProbes.begin(); it != _textProbes.end(); it++)
	{
		ProbeAtom *probe = it->second;
		if (!probe->isSelected() && !_shifter->hasPointer(probe))
		{
			it->first->setHide(-1.f, false);
		}
		else if (!_shifter->hasPointer(probe))
		{
			it->first->setHide(0.f, false);
			_shifter->addPosition(probe, make_init(probe->probe()),
			                      make_getter(probe->probe()), 
			                      make_setter(probe->probe()));

			OpSet<void *> set;
			get_probes_as_set(probe->probe(), set);
			_shifter->limitSensitivity(probe, set);

			_shifter->addTidy(make_tidy(probe));
		}
		probe->selected(0, true);
	}
	_shifter->unpause();

	for (auto it = _bondProbes.begin(); it != _bondProbes.end(); it++)
	{
		ProbeBond *probe = it->second;
		if (!probe->isSelected() && !_shifter->hasPointer(probe))
		{
			it->first->setHide(-1.f, false);
		}
		else if (!_shifter->hasPointer(probe))
		{
			it->first->setHide(0.f, false);
			_shifter->includePointer(probe);
			_shifter->addTidy(make_tidy(probe));
		}
		probe->selected(0, true);
	}
	
	_shifter->tidy();
}

void ProtonNetworkView::sendObject(std::string tag, void *object)
{
	auto main_job = [this, &object]()
	{
		Probe *p = static_cast<Probe *>(object);
		if (_textProbes.count(p))
		{
			_textProbes[p]->FloatingText::setAlpha(p->alpha());
		}

		if (_bondProbes.count(p))
		{
			_bondProbes[p]->updateProbe();
		}
	};

	main_job();
}

void ProtonNetworkView::setup()
{
	addTitle("Proton network");

	findAtomProbes();
}

void ProtonNetworkView::setMenu(Menu *menu)
{
	float x; float y;
	getFractionalPos(x, y);
	menu->setup(x, y);

	setModal(menu);
}

void ProtonNetworkView::keyReleaseEvent(SDL_Keycode pressed)
{
	if (_controlPressed && !_shiftPressed && pressed == SDLK_z)
	{
		network().undoStack().undo();
	}

	if ((_controlPressed && _shiftPressed && pressed == SDLK_z) ||
	    (_controlPressed && pressed == SDLK_y))
	{
		network().undoStack().redo();
	}

	Scene::keyReleaseEvent(pressed);
}

void ProtonNetworkView::completeResidues(bool stop_at_alpha)
{
	typedef std::pair<std::string, ResidueId> ChainRes;
	OpSet<ChainRes> residues;
	
	// go through covalently bound atoms and extend to include all 
	// existing residue IDs
	
	auto chain_res = [](Atom *const &atom) -> ChainRes
	{
		return {atom->chain(), atom->residueId()};
	};
	
	auto initial_assessment = [&residues, chain_res](Probe *probe)
	{
		residues.insert(chain_res(probe->atom()));
	};
	
	auto check_probe = [&residues, chain_res, stop_at_alpha]
	(Probe *other, Probe *prev) -> bool
	{
		if (other->is_bond())
		{
			BondProbe *bp = static_cast<BondProbe *>(other);
			// exclude those which bridge a hydrogen (nullptr atom)
			if (!(bp->left().atom() && bp->right().atom()))
			{
				return false;
			}
			if (!(residues.count(chain_res(bp->left().atom())) &&
			      residues.count(chain_res(bp->right().atom()))))
			{
				return false;
			}
			
			if (stop_at_alpha && prev->atom() && prev->atom()->isReporterAtom())
			{
				return false;
			}
		}

		if (other->atom() && 
		    !residues.count(chain_res(other->atom())))
		{
			return false;
		}

		return true;
	};
	
	completeOnCondition(initial_assessment, check_probe);
}

void ProtonNetworkView::
completeOnCondition(std::function<void(Probe *probe)> initial_assessment,
                    std::function<bool(Probe *probe, Probe *prev)> check_probe)
{
	// get the initial selected probes into a set.
	OpSet<Probe *> done = selected_probes(_textProbes);

	for (Probe *probe : done)
	{
		if (probe->atom())
		{
			initial_assessment(probe);
		}
	}

	OpSet<Probe *> fresh = done;
	do
	{
		OpSet<Probe *> tmp = fresh;
		fresh = {};
		for (Probe *probe : tmp)
		{
			for (Probe *other : probe->others())
			{
				if (done.count(other) > 0)
				{
					continue;
				}
				
				if (!check_probe(other, probe))
				{
					continue;
				}

				done.insert(other);
				fresh.insert(other);

				if (other->atom() && _textProbes.count(other))
				{
					_textProbes[other]->selected(0, 0);
				}
				else if (_bondProbes.count(other))
				{
					_bondProbes[other]->selected(0, 0);
				}
			}
		}

	}
	while (fresh.size() > 0);
}

void ProtonNetworkView::expandSelectionToNeighbours()
{
	// get the initial selected probes into a set.
	OpSet<Probe *> done = selected_probes(_textProbes);

	// go through selected probes and add neighbours if they are in the set.
	// continue until neighbours are exhausted
	OpSet<Probe *> fresh = done;
	
	do
	{
		OpSet<Probe *> tmp = fresh;
		fresh = {};
		for (Probe *probe : tmp)
		{
			for (Probe *other : probe->others())
			{
				if (done.count(other) > 0)
				{
					continue;
				}

				if (other->is_covalent())
				{
					continue;
				}
				
				if (other->is_bond())
				{
					BondProbe *bp = static_cast<BondProbe *>(other);
					if (bp->_obj.value() == hnet::Bond::Broken)
					{
						continue;
					}
				}

				std::cout << "inserting " << other->display() << std::endl;
				done.insert(other);
				fresh.insert(other);

				if (_textProbes.count(other))
				{
					_textProbes[other]->selected(0, 0);
				}
				else if (_bondProbes.count(other))
				{
					_bondProbes[other]->selected(0, 0);
				}
			}
		}

	}
	while (fresh.size() > 0);
}

void ProtonNetworkView::sendSelection(float t, float l, float b, float r,
                                      bool inverse)
{
	IndexResponseView::sendSelection(t, l, b, r, inverse);
}

void ProtonNetworkView::setManualAdjust(Probe *probe)
{
	if (!_textProbes.count(probe))
	{
		return;
	}

	setManualAdjust(_textProbes.at(probe));
}

void ProtonNetworkView::setManualAdjust(ProbeAtom *probe)
{
	if (_shifter)
	{
		_shifter->setSkip(probe);
		_manual = probe;
	}

}

void ProtonNetworkView::mouseReleaseEvent(double x, double y,
                                  SDL_MouseButtonEvent button)
{
	if (_shifter)
	{
		_shifter->setSkip(nullptr);
		_manual = nullptr;
	}

	Mouse3D::mouseReleaseEvent(x, y, button);
}

void ProtonNetworkView::mouseMoveEvent(double x, double y)
{
	if (_left && !_shiftPressed && !_controlPressed && !_altPressed && 
	    _manual)
	{
		double dx = x - _lastX;
		double dy = y - _lastY;

		dx /= width();
		dy /= height();
		
		glm::vec4 start = _proj * glm::vec4(0, 0, -1, 1);
		glm::vec4 end = _proj * glm::vec4(dx, -dy, -1, 1);
		glm::vec3 move = end - start;

		glm::vec3 curr = _shifter->getPosition(_manual);
		curr += move * 12.f;
		_shifter->setPosition(_manual, curr);
		_manual->updatePosition();
		_lastX = x;
		_lastY = y;
	}
	else
	{
		Mouse3D::mouseMoveEvent(x, y);
	}
	
}
