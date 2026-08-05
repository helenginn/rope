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
#include "ProbeCharge.h"
#include "FocusResidue.h"
#include "ProtonNetworkView.h"
#include <vagabond/core/protonic/ExhaustiveSearch.h>
#include <vagabond/core/protonic/CliqueFinder.h>
#include <vagabond/core/protonic/CountProbe.h>
#include <vagabond/core/protonic/Subdivide.h>
#include <vagabond/core/PositionShifter.h>
#include <vagabond/core/AtomGroup.h>
#include <vagabond/core/Environment.h>
#include <vagabond/core/ResidueRange.h>
#include <vagabond/utils/DoJob.h>
#include <vagabond/gui/CliqueView.h>
#include <vagabond/gui/ProblemReviewView.h>
#include <vagabond/gui/HBondAnalysisControl.h>
#include <vagabond/gui/elements/AskYesNo.h>
#include <vagabond/gui/elements/AskForText.h>
#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/gui/elements/ChooseRange.h>
#include <vagabond/gui/elements/ImageButton.h>
#include <vagabond/gui/elements/FloatingText.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/Menu.h>
#include <fstream>

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

	// every ProbeAtom/ProbeBond/ProbeCharge/CliqueView added via
	// addObject() holds a raw pointer into _network - they must be torn
	// down before the network (and its Probes/Cliques) is deleted below,
	// otherwise ~HasRenderables() (which runs later, as part of the
	// implicit Scene base-class teardown) deletes them while they still
	// point at freed memory.
	deleteObjects();

	delete &_network;
}

void ProtonNetworkView::linkSymmetricAtomProbes(const hnet::AtomConf &ac)
{
	if (!ac.ptr || !ac.ptr->symmetryCopyOf())
	{
		return;
	}

	AtomProbe *p = _network.probeForAtom(ac);
	AtomProbe *q = _network.probeForAtom({ac.ptr->symmetryCopyOf(), ac.conf});
	if (!(p && q))
	{
		return;
	}
	
	p->register_probe(q);
	q->register_probe(p);
}

void ProtonNetworkView::findAtomProbes()
{
	for (AtomProbe *const &probe : _network.atomProbes())
	{
		if (probe->is_bulk())
		{
			continue;
		}

		ProbeAtom *text = new ProbeAtom(this, probe);
		addObject((FloatingText *)text);
		linkSymmetricAtomProbes(probe->atomConf());
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
		_hProbes.insert(probe);
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

		// _textProbes is fully populated for both AtomProbe and
		// HydrogenProbe by the two loops above, before any BondProbe is
		// constructed, so both lookups are guaranteed to succeed here.
		bond->setEndpoints(_textProbes[&probe->left()],
		                   _textProbes[&probe->right()]);
	}

	for (CountProbe *const &probe : _network.chargeProbes())
	{
		ProbeCharge *charge = new ProbeCharge(this, probe);
		addObject((FloatingImage *)charge);
		_allProbes.insert(probe);
		_countProbes[probe] = charge;
		probe->setResponder(this);
		addIndexResponder(charge);
	}

	shiftToCentre(_network.centre(), 50);
	setMakesSelections();
	IndexResponseView::setup();
//	preparePingPongBuffers();

	auto check_for_collapses = [this]()
	{
		std::vector<std::string> messages = _network.impromptuCollapses();
		
		if (!messages.size())
		{
			return;
		}

		std::string text = "Clashes prematurely collapsed "
		+ std::to_string(messages.size()) + " alternate conformers.\n"\
		"These will need editing of the PDB file to resolve.\n"\
		"Please click the lemon for a complete list.";
		
		auto clicked_lemon = [this, messages]()
		{
			ProblemReviewView *view = new ProblemReviewView(this, messages);
			view->show();
		};
		
		BadChoice *bc = new BadChoice(this, text);
		addMainThreadJob
		([this, bc, clicked_lemon]()
		 {
			ImageButton *b = new ImageButton("assets/images/lemon.png", this);
			b->setReturnJob(clicked_lemon);
			b->resize(0.06);
			b->setRight(0.95, 0.15);
			addObject(b);


			setModal(bc);
		});
	};

	auto reclique_request = [this]<class NextJob>(NextJob job)
	{
		return [this, job] ()
		{
			if (!_network._reclique)
			{
				job();
				return;
			}

			auto recalculate_cliques = [this, job]()
			{
				_network.cliques().clear();
				job();
			};

			std::string text = "Cliques saved in rope environment appear to be\n"\
			"out of date. Delete to allow recalculation?\n"\
			"This will delete any customisations.";

			AskYesNo *ayn = new AskYesNo(this, text);
			ayn->addJob("yes", recalculate_cliques);
			ayn->addJob("no", job);

			addMainThreadJob
			([this, ayn]()
			 {
				setModal(ayn);
			});

			_network._reclique = false;
		};
	};
	
	addMainThreadJob(reclique_request(check_for_collapses));
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
		menu->addOption("expand to clique", 
		                [this]() { expandSelectionToNeighbours(); });
		menu->addOption("expand by four jumps", 
		                [this]() { expandSelectionToNeighbours(4); });
		menu->addOption("complete residues", 
		                [this]() { completeResidues(false); });
		menu->addOption("complete to C-alpha", 
		                [this]() { completeResidues(true); });
		menu->addOption("make new clique",
		                [this]() { ensureCliqueView(); makeNewClique(); });
		menu->addOption("select using plan...",
		                [this]() { askForSelectionPlan(); });
		
		OpSet<Probe *> selected = selected_probes(_textProbes);
		selected += selected_probes(_bondProbes);
		if (selected.size() > 0 && _2D)
		{
			menu->addOption("add to figure", [this]() { arrangeFigure(); });
		}

		setMenu(menu);
	}
}

void ProtonNetworkView::arrangeFigure()
{
	// getter/setter operate on the ProbeAtom (render layer), not the
	// physics Probe - PositionShifter must never write Probe::_pos, since
	// Energy.cpp reads that directly for H-bond angle geometry and must
	// never see 2D layout positions.
	auto make_getter = [](ProbeAtom *probe)
	{
		return [probe]() -> glm::vec3
		{
			return probe->FloatingText::centroid();
		};
	};

	// the layout algorithm still needs real 3D inter-atomic distances as
	// its reference geometry, so this alone stays reading the physics
	// probe - read-only, never written back to.
	auto make_init = [](Probe *probe)
	{
		return [probe]() -> glm::vec3
		{
			return probe->_init;
		};
	};

	auto make_setter = [](ProbeAtom *probe)
	{
		return [probe](const glm::vec3 &vec)
		{
			probe->FloatingText::setPosition(vec);
		};
	};

	// atoms: the setter above already wrote the new render position
	// directly, so all that is needed per step is to flag a redraw - NOT
	// updatePosition(), which would immediately resync from (unchanged)
	// physics and undo the shift. Bonds are unaffected by this - they are
	// never given their own position via addPosition(), only included via
	// includePointer(), and their updatePosition() already follows their
	// endpoints' current render position (see ProbeBond::setEndpoints),
	// physics-derived or not.
	auto make_tidy_atom = [](ProbeAtom *probe)
	{
		return [probe]()
		{
			probe->FloatingText::forceRender(true, false);
		};
	};

	auto make_tidy_bond = [](ProbeBond *probe)
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

		// replaces PositionShifter's own former limitSensitivity()/depth=2
		// relay mechanism: _reach[atom] is precomputed below (1 AND 2
		// covalent bonds away, matching that mechanism's exact former
		// reach) once per atom, the moment it's added to the shifter -
		// this just looks it up. Weight 0 (unrelated/further away) makes
		// the spring term a no-op for that pair.
		_shifter->setWeightFn([this](void *left, void *right) -> float
		{
			auto found = _reach.find(left);
			if (found == _reach.end())
			{
				return 0.f;
			}
			return found->second.count(right) ? 1.f : 0.f;
		});

		// this weightFn is strictly binary (1.0 = within 2 covalent
		// bonds, 0.0 = not) - matches the old depth=2 relay's own
		// "remaining = all - done" repulsion exemption exactly (see
		// setExemptWeightedFromRepulsion()'s own comment for why this
		// matters: without it, two already-spring-held bonded atoms also
		// got an unrelated repulsion kick, which could NaN-poison an
		// atom's momentum and freeze it - most visible as a dragged atom
		// snapping then refusing to move).
		_shifter->setExemptWeightedFromRepulsion(true);

		_shifter->run();
	}

	std::vector<std::function<void()>> tidyJobs;

	for (auto it = _countProbes.begin(); it != _countProbes.end(); it++)
	{
		it->first->setHide(-1.f, false);
		it->second->quickUpdate();
	}

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
			                      make_getter(probe),
			                      make_setter(probe));

			// 1 bond away, via the graph walk shared with HBondDiagram::
			// makeAtoms() (Probe::bondedNeighbours() - see its own
			// comment)...
			OpSet<Probe *> neighbours;
			Probe::bondedNeighbours(probe->probe(), neighbours);

			// ...plus 2 bonds away - matches PositionShifter's former
			// depth=2 relay exactly: a snapshot of the 1-hop set first,
			// since extending `neighbours` while iterating it directly
			// would let newly-inserted 2-hop atoms also get walked as if
			// they were 1-hop, silently reaching 3+ bonds out.
			OpSet<Probe *> hopOne = neighbours;
			for (Probe *other : hopOne)
			{
				Probe::bondedNeighbours(other, neighbours);
			}

			// only atoms actually displayed (this view's own _textProbes)
			// and not logically absent get a real spring weight - the
			// shared walk above returns everything reachable regardless,
			// leaving this filtering to the caller (see its own comment).
			OpSet<void *> reach;
			for (Probe *other : neighbours)
			{
				if (_textProbes.count(other) && !other->is_absent())
				{
					reach.insert(_textProbes[other]);
				}
			}

			_reach[probe] = reach;

			_shifter->addTidy(make_tidy_atom(probe));
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
			_shifter->addTidy(make_tidy_bond(probe));
		}
		probe->selected(0, true);
	}
	
	_shifter->tidy();
}

void ProtonNetworkView::leave2D()
{
	// discard the 2D layout entirely rather than trying to reconcile it -
	// next entry into 2D mode (see arrangeFigure()) always starts a fresh
	// PositionShifter, since _shifter is null again.
	if (_shifter)
	{
		delete _shifter;
		_shifter = nullptr;
	}

	// entries are ProbeAtom* pointers only valid for the 2D session that
	// built them (see the member's own comment) - stale otherwise.
	_reach.clear();

	// resync every render position back from the (untouched throughout)
	// physics position, undoing the 2D placement - atoms first, since
	// ProbeBond::updatePosition() now follows its endpoints' current
	// render position rather than physics directly.
	for (auto it = _textProbes.begin(); it != _textProbes.end(); it++)
	{
		it->first->setHide(0.f, false);
		it->second->updatePosition();
	}

	for (auto it = _bondProbes.begin(); it != _bondProbes.end(); it++)
	{
		it->first->setHide(0.f, false);
		it->second->updatePosition();
	}

	for (auto it = _countProbes.begin(); it != _countProbes.end(); it++)
	{
		it->first->setHide(0.f, false);
		it->second->quickUpdate();
	}

	_2D = false;

	if (_activeClique)
	{
		// re-select so it is obvious where the active clique is once back
		// in 3D, and frame the clique itself rather than the whole
		// protein - that is what we are actually looking at.
		selectProbes(_activeClique->probes());
		shiftToCentre(_activeClique->centroid(), 50);
	}
	else
	{
		shiftToCentre(_network.centre(), 50);
	}
}

void ProtonNetworkView::cancelAnalysis()
{
	if (_2D)
	{
		leave2D();
	}

	_activeClique = nullptr;
	_analysing = false;

	deselect();
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

void ProtonNetworkView::ensureCliqueView()
{
	// constructs _cv if needed, but does not show it - callers that just
	// need somewhere to insert a Clique (e.g. makeNewClique()) shouldn't
	// force the CliqueView on screen as a side effect. See
	// showCliqueView() for construct-and-show.
	if (_cv)
	{
		return;
	}

	_cv = new CliqueView(this, _network, _hProbes);

	auto kill = [this]()
	{
		removeObject(_cv);
	};

	_cv->setKillAndClean(kill);
	highlightCliques();
}

void ProtonNetworkView::showCliqueView()
{
	ensureCliqueView();

	// picks up subdivisions created (Exhaustive search) or whose select
	// job was borrowed and restored (ViewCorrelations) since this cached
	// view was last shown.
	_cv->rewireSubdivisions();

	// addObject() now throws on a double-add, so a CliqueView that is
	// already on screen (still in _objects - it isn't removed until
	// killed) must not be re-added.
	if (!hasObject(_cv))
	{
		addObject(_cv);
	}
}

void ProtonNetworkView::makeMainMenu()
{
	auto browse_cliques = [this]()
	{
		showCliqueView();
	};

	auto reset_cliques = [this]()
	{
		std::string text = "This will delete all cliques and recalculate\n"\
		"them. Are you sure?";

		AskYesNo *ayn = new AskYesNo(this, text);
		ayn->addJob("yes", [this]()
		{
			_network.cliques().clear();
			_network.updateModelCliques();

			// browse_cliques() re-shows the existing _cv rather than
			// rebuilding it if one is already around, so its cached list
			// (populated once at construction from _network.cliques() as
			// it was then) would keep showing the just-deleted cliques.
			// Destroy it so the next "Browse cliques" click reconstructs
			// it fresh and re-discovers from the now-empty network.
			if (_cv)
			{
				deleteLater(_cv);
				_cv = nullptr;
			}
		});

		addMainThreadJob
		([this, ayn]()
		 {
			setModal(ayn);
		});
	};

	TextButton *text = new TextButton("Menu", this);
	auto make_menu = [this, browse_cliques, reset_cliques, text]()
	{
		if (hasObject(_cv))
		{
			_cv->kill();
		}
		glm::vec2 c = text->xy();
		Menu *m = new Menu(this, this, "options");
		if (!_activeClique)
		{
			m->addOption("Browse cliques", browse_cliques);
			m->addOption("Reset cliques", reset_cliques);
		}

		m->addOption("Save", [this]()
		{
			_network.updateModelCliques();
			Environment::environment().save();
		});
		
		m->addOption("Export H-bonds", [this]()
        {
	              exportHBonds();
        });

		auto control_analysis = [this]()
		{
			HBondAnalysisControl *hbac = 
			new HBondAnalysisControl(this, _activeClique, _network);
			hbac->show();
		};

		
		if (_activeClique)
		{
			m->addOption("Analysis overview", control_analysis);
			m->addOption("Cancel analysis", [this]() { cancelAnalysis(); });
		}
		
		auto freeze_positions = [this]()
		{
			if (_shifter)
			{
				_shifter->pause();
			}
		};
		auto unfreeze_positions = [this]()
		{
			if (_shifter)
			{
				_shifter->unpause();
			}
		};
		
		if (_shifter && !_shifter->isPaused())
		{
			m->addOption("Freeze positions", freeze_positions);
		}
		else if (_shifter && _shifter->isPaused())
		{
			m->addOption("Unfreeze positions", unfreeze_positions);
		}

		if (_2D)
		{
			m->addOption("Switch to 3D", [this]() { leave2D(); });
		}
		else if (_activeClique)
		{
			m->addOption("Switch to 2D", [this]()
			{
				// matches what right-click "analyse" did the first time
				// (CliqueView::analyse_bonds) - without this, re-entering
				// 2D after a trip back to 3D only has leave2D()'s
				// re-selected clique probes, not expanded up to CA.
				completeResidues(true);
				arrangeFigure();
			});
		}

		m->setup(c.x, c.y);
		setModal(m);
	};
	
	text->setReturnJob(make_menu);
	text->setRight(0.95, 0.1);
	addObject(text);

}

void ProtonNetworkView::setup()
{
	addTitle("Proton network");

	findAtomProbes();
	makeMainMenu();
}

void ProtonNetworkView::setMenu(Menu *menu)
{
	float x; float y;
	getFractionalPos(x, y);
	menu->setup(x, y);

	setModal(menu);
}

void ProtonNetworkView::focusOnResidue(std::string chain, int res)
{
	AtomGroup *atoms = _network.atoms();
	Atom *chosen = atoms->atomByIdName({res}, "CA", chain);
	if (!chosen)
	{
		chosen = atoms->atomByIdName({res}, "", chain);
	}
	if (!chosen)
	{
		return;
	}

	shiftToCentre(chosen->derivedPosition(), 0);
}

void ProtonNetworkView::keyPressEvent(SDL_Keycode pressed)
{
	if (pressed == SDLK_g && !lastModal())
	{
		FocusResidue::prepareEnter
		(this, [this](std::string chain, int res)
		{
			focusOnResidue(chain, res);
		});
	}

	Mouse2D::keyPressEvent(pressed);

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

void ProtonNetworkView::selectProbes(const OpSet<Probe *> &probes, bool on)
{
	for (Probe *const &other : probes)
	{
		if (_textProbes.count(other))
		{
			_textProbes[other]->selected(0, (on ? 0 : 1));
		}
		else if (_bondProbes.count(other))
		{
			_bondProbes[other]->selected(0, (on ? 0 : 1));
		}
		else
		{
			std::cout << "Could not find probe: " << other->desc() << std::endl;
		}
	}
}

void ProtonNetworkView::makeNewClique()
{
	if (!_cv)
	{
		return;
	}

	OpSet<Probe *> probes = selected_probes(_textProbes);
	probes += selected_probes(_bondProbes);

	while (Subdivide::finish_ends(probes)) {};

	AskForText *aft = new AskForText(this, "Name for new clique:", "", this);
	aft->setDefaultText("Custom clique");
	aft->setReturnJob([this, probes](std::string name)
	{
		Clique *clique = _network.newClique(probes);
		// setName(), not setDisplayName() - CliqueView::insertClique()
		// re-derives displayName from name() on insertion (add_clique's
		// clique->setDisplayName(clique->name())), which would otherwise
		// immediately overwrite a plain setDisplayName() with the
		// auto-generated "N protein atoms, M waters" fallback.
		clique->setName(name);
		_cv->insertClique(clique);
	});
	setModal(aft);
}

void ProtonNetworkView::askForSelectionPlan()
{
	AskForText *aft = new AskForText(this, "Enter selection plan:",
	                                 "", this);
	aft->allowCapitals(true);
	aft->setHelpText("Selection of residues by text input:\n\n"\
	                 "e.g. A157,253,109-111,B145,153\n"\
	                 "Comma-separated residues - a dash indicates a range,\n"\
	                 "and a chain letter applies until you give a new one.");
	aft->setReturnJob([this](std::string plan)
	{
		selectUsingPlan(plan);
	});
	setModal(aft);
}

void ProtonNetworkView::selectUsingPlan(std::string plan)
{
	int radius = -1;

	// optional "@N;" prefix overriding the search radius, stripped before
	// the rest of the plan reaches parseResidueRanges() - '@' can never
	// be confused with a chain letter the way e.g. "N15;" would be.
	if (plan.size() > 0 && plan[0] == '@')
	{
		size_t semi = plan.find(';');
		if (semi != std::string::npos)
		{
			radius = atoi(plan.substr(1, semi - 1).c_str());
			plan = plan.substr(semi + 1);
		}
	}

	std::vector<ResidueRangeToken> tokens;
	std::string error;
	if (!parseResidueRanges(plan, tokens, error))
	{
		BadChoice *bc = new BadChoice(this, error);
		setModal(bc);
		return;
	}

	int numResidues = 0;
	for (const ResidueRangeToken &token : tokens)
	{
		numResidues += (token.end - token.begin + 1);
	}

	auto build = [this, tokens, plan, numResidues](int n)
	{
		std::vector<OpSet<Probe *>> groups =
		CliqueFinder::probeGroupsForResidues(_network, tokens);
		OpSet<Probe *> connected =
		CliqueFinder::connectGroups(groups, n, false);

		// matches the manual "expand to clique" (H-bond only) + "complete
		// to C-alpha" (covalent) combination - the bridging search above
		// stays H-bond-only, but the residues it touches still need
		// their covalent-bonded backbone filled in. Those atoms carry no
		// correlation data themselves (isActiveAtom() excludes them from
		// ProbeResult regardless), but they're what SearchAll's own
		// covalent-inclusive _wider expansion needs to actually reach
		// nearby H-bond-active atoms that only connect via the backbone,
		// not any direct hydrogen bond.
		connected = CliqueFinder::completeToResidues(connected, true);

		while (Subdivide::finish_ends(connected)) {}

		selectProbes(connected);

		if (connected.size() == 0)
		{
			return;
		}

		// re-normalised with the radius that was actually used, whether
		// that came from the "@N;" prefix or the ChooseRange fallback
		// below - so the saved plan always reproduces this exact
		// selection on its own, without depending on a slider default
		// that could change later.
		std::string planText = "@" + std::to_string(n) + ";" + plan;
		std::string defaultName = std::to_string(numResidues) + " residues "\
		"connected by up to " + std::to_string(n) + " nodes";

		AskForText *aft = new AskForText(this, "Name for new clique:", "",
		                                 this);
		aft->setDefaultText(defaultName);
		aft->setReturnJob([this, connected, planText](std::string name)
		{
			ensureCliqueView();
			Clique *clique = _network.newClique(connected);
			clique->setPlanText(planText);
			// setName(), not setDisplayName() - CliqueView::insertClique()
		// re-derives displayName from name() on insertion (add_clique's
		// clique->setDisplayName(clique->name())), which would otherwise
		// immediately overwrite a plain setDisplayName() with the
		// auto-generated "N protein atoms, M waters" fallback.
		clique->setName(name);
			_cv->insertClique(clique);
		});
		setModal(aft);
	};

	if (radius >= 0)
	{
		build(radius);
		return;
	}

	ChooseRange *cr = new ChooseRange(this, "Choose search radius (nodes)",
	                                  "", this);
	cr->setDefault(15, 15);
	cr->setRange(2, 50, 48);
	cr->setReturn([build](float, float max)
	{
		build(lrint(max));
	});
	setModal(cr);
}

void ProtonNetworkView::highlightCliques()
{
	if (_cv)
	{
		OpSet<Probe *> probes = selected_probes(_textProbes);
		_cv->highlightCliquesWith(probes);
	}
}

void ProtonNetworkView::completeResidues(bool stop_at_alpha)
{
	OpSet<Probe *> done = selected_probes(_textProbes);
	OpSet<Probe *> ps = CliqueFinder::completeToResidues(done, stop_at_alpha);
	selectProbes(ps);
	highlightCliques();
}


void ProtonNetworkView::expandSelectionToNeighbours(int max_jumps)
{
	// get the initial selected probes into a set.
	OpSet<Probe *> done = selected_probes(_textProbes);

	OpSet<Probe *> ps = 
	CliqueFinder::expandSelectionToNeighbours(done, max_jumps);
	selectProbes(ps);
	highlightCliques();
}

void ProtonNetworkView::sendSelection(float t, float l, float b, float r,
                                      bool inverse)
{
	IndexResponseView::sendSelection(t, l, b, r, inverse);
	highlightCliques();
}

void ProtonNetworkView::sendClickSelection(double x, double y, bool inverse)
{
	IndexResponseView::sendClickSelection(x, y, inverse);
	highlightCliques();
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

void ProtonNetworkView::exportHBonds()
{
	std::ostringstream tsv;
	AtomGroup *write = _network.assignCertainHydrogens(tsv);
	write->writeToFile("tmp_h.pdb");
	delete write;

	std::ofstream file; 
	file.open("tmp_hbonds.txt");
	file << tsv.str();
	file.close();
}

void ProtonNetworkView::mouseReleaseEvent(double x, double y,
                                  SDL_MouseButtonEvent button)
{
	if (_shifter)
	{
		_shifter->setSkip(nullptr);
		_manual = nullptr;
	}

	if (_onClick)
	{
		_onClick();
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

		// NOT _manual->updatePosition() - that resyncs the render position
		// from _probe->position(), the physics position, which 2D mode
		// never touches (see arrangeFigure()'s own comment: PositionShifter
		// must never write Probe::_pos) and so stays frozen at wherever it
		// was before 2D mode was entered. Calling it here immediately
		// overwrote the position setPosition() just wrote, undoing every
		// single drag step - visible as the dragged atom snapping once (to
		// that frozen position) then staying frozen there for the rest of
		// the drag. forceRender() alone gets the redraw updatePosition()
		// was also providing, without the harmful resync.
		glm::vec3 curr = _shifter->getPosition(_manual);
		curr += move * 12.f;
		_shifter->setPosition(_manual, curr);
		_manual->FloatingText::forceRender(true, false);
		_lastX = x;
		_lastY = y;
	}
	else
	{
		Mouse3D::mouseMoveEvent(x, y);
	}
	
}
