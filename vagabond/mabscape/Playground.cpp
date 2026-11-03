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

#include "ContactPoint.h"
#include "Playground.h"
#include <algorithm>
#include <fstream>
#include <set>
#include "Positions.h"
#include "PosToComp.h"
#include "AbWatch.h"
#include "Mesh.h"
#include "AntibodyOrderingView.h"
#include "ResultsView.h"
#include "ResultsClusterView.h"
#include "ToggleFixedView.h"
#include "AlignmentView.h"
#include "ContactPoint.h"
#include "CrossCheckView.h"
#include "AntibodyPdb.h"
#include "GhostAb.h"
#include "Mab.h"
#include <vagabond/gui/elements/Icosahedron.h>
#include <vagabond/gui/elements/Box.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/Menu.h>
#include <vagabond/gui/elements/AskForText.h>
#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/gui/elements/Text.h>
#include <vagabond/gui/elements/TextEntry.h>
#include <vagabond/gui/elements/Window.h>
#include <vagabond/gui/MatrixPlot.h>
#include <vagabond/gui/ColourLegend.h>

// a text entry that reports the text after every key
class SearchEntry : public TextEntry
{
public:
	SearchEntry(const std::string &text, ButtonResponder *sender)
	: TextEntry(text, sender) {}

	void setChangedJob(const std::function<void(const std::string &)> &job)
	{
		_changed = job;
	}

	virtual void keyPressed(char key)
	{
		TextEntry::keyPressed(key);
		notify();
	}

	virtual void keyPressed(SDL_Keycode other)
	{
		TextEntry::keyPressed(other);

		if (other != SDLK_RETURN)
		{
			notify();
		}
	}
private:
	void notify()
	{
		if (_changed)
		{
			_changed(scratch());
		}
	}

	std::function<void(const std::string &)> _changed;
};

Playground::Playground(Scene *prev, Mab &mab) 
: Scene(prev), Display(prev), _mab(mab)
{
	_farSlab = 80;
	_slabbing = true;
	setPingPong(true);
	setMakesSelections();
	shiftToCentre({}, 80);

}

void Playground::showMesh(const Competition &comp)
{
	std::string antigen_name = comp.antigen;
	Antigen *antigen = _mab.antigens(antigen_name);

	Mesh *mesh = antigen->mesh();
	addObject(mesh);
	shiftToCentre(mesh->centroid(), 0);
	std::cout << "Mesh centre: " << mesh->centroid() << std::endl;
	_mesh = mesh;
	mesh->loadExclusions();

}

// the hovering and clicking of an antibody
void Playground::setupWatch(AbWatch *aw)
{
		addIndexResponder(aw);
		aw->setHoverJob([this, aw](const std::string &ab, int copy)
		{
			// A clicked antibody stays as it is, but others can still be hovered
			// over for their tags: only their labels and highlights, and only
			// one at a time, without disturbing the pinned antibody's.
			if (_pinned)
			{
				const std::string &pinned = _pinned->antibody();
				if (ab == pinned)
				{
					return false;
				}

				for (AbWatch *other : _watches)
				{
					if (other != aw && other->antibody() != pinned)
					{
						other->deleteTemps();
						unhighlight(other);
					}
				}

				return true;
			}

			// only one antibody at a time: sliding straight from one onto
			// another never passes over empty space to clear the last
			// (its other symmetry copies light up too, with no label)
			for (AbWatch *other : _watches)
			{
				if (other == aw)
				{
					continue;
				}

				other->deleteTemps();

				if (other->antibody() == ab)
				{
					other->highlightQuietly();
				}
				else
				{
					unhighlight(other);
				}
			}

			clearGhost();
			showGhost(aw);
			showHeat(aw);
			return true;
		});
		aw->setRightClickJob([this](AbWatch *clicked)
		{
			alignmentMenu(clicked);
		});
		aw->setClickJob([this](AbWatch *clicked)
		{
			// the colouring by a matrix carries over to the next antibody
			ColourBy keep = _colourBy;
			clearSelection();
			pin(clicked);

			if (keep != Nothing)
			{
				_colourBy = keep;
				applyColouring();
			}
		});
}

// Antibodies brought back from the naughty corner have no positions yet, nor
// anything to show them with: they start from random positions.
void Playground::syncAntibodies()
{
	if (!_positions || _mab.competitions.empty())
	{
		return;
	}

	for (const std::string &name : _mab.competitions.begin()->antibody_names())
	{
		if (Positions::ignored(name) || _positions->has(name))
		{
			continue;
		}

		if (!_positions->addAntibody(name))
		{
			std::cout << "No room to bring back " << name << " in this "
			"session: reopen the playground to include it." << std::endl;
			continue;
		}

		size_t before = _watches.size();
		_positions->loadAntibodyInto(this, _watches, name);
		for (size_t i = before; i < _watches.size(); i++)
		{
			setupWatch(_watches[i]);
		}
	}
}

void Playground::showFiducials(const Competition &comp)
{
	_positions = new Positions(*_mab.antigens(comp.antigen), comp, _mab,
	                           _mesh->vertexFinder(), _mesh->random());
	_watches = {};
	_positions->setMeshVertices(_mesh->vertexPositions());

	// how curved the mesh is at each vertex, for the radius of an antibody
	_positions->setCurvatureZ(_mesh->curvatureZ());

	// where each fixed antibody sits on the mesh (the models are loaded to
	// work it out the once, and it is then kept in the json)
	_positions->buildFootprints();

	// as toggled last time
	if (comp.fixed_saved)
	{
		_positions->setFixedAntibodies(comp.fixed_antibodies);
	}

	_positions->loadAntibodiesInto(this, _watches);
	
	for (AbWatch *aw : _watches)
	{
		setupWatch(aw);
	}
}

void Playground::setup()
{
	Competition &comp = *_mab.competitions.begin();
	showMesh(comp);

	// before the antibodies, so that these never hide them from the mouse
	_ghost = new GhostAb();
	addObject(_ghost);
	_ghostLine = new GhostLine();
	addObject(_ghostLine);

	showFiducials(comp);

	// picks up any results saved in the json
	_model = new PosToComp(comp, *_positions);
	showMatrices();
	
	// up in the top right corner: Display, then Menu
	TextButton *tb = new TextButton("Menu");
	tb->setCentre(0.94, 0.04);

	TextButton *db = new TextButton("Display");
	db->setCentre(0.84, 0.04);

	auto show_display = [this, db]()
	{
		Menu *menu = new Menu(this);
		if (_positions->viewing())
		{
			menu->addOption("watch live positions", [this]() { resumeLive(); });
		}

		if (_matricesVisible)
		{
			menu->addOption("hide matrices",
			                [this]() { setMatricesVisible(false); });
		}
		else
		{
			menu->addOption("show matrices",
			                [this]() { setMatricesVisible(true); });
		}

		menu->addOption("check alignment", [this]() { checkAlignment(); });

		if (_mesh && _mesh->opaque())
		{
			menu->addOption("transparent antigen", 
			                [this]() { setOpaque(false); });
		}
		else
		{
			menu->addOption("opaque antigen", [this]() { setOpaque(true); });
		}

		menu->setup(db, 1);
		setModal(menu);
	};

	db->setReturnJob(show_display);
	addObject(db);

	auto show_menu = [this, tb]()
	{
		Menu *menu = new Menu(this);
		menu->addOption("save", [this]() { saveResults(); });
		menu->addOption("competition data", [this]() { showCompetitionData(); });
		menu->addOption("toggle fixed", [this]() { showToggleFixed(); });
		menu->addOption("antibody radius", [this]() { askForRadius(); });
		menu->addOption("curvature scale", [this]() { askForCurvatureScale(); });
		if (_model)
		{
			menu->addOption(std::string("footprints: ") + 
			                (_model->footprintsOn() ? "on" : "off"), [this]()
			{
				if (_model->isRefining())
				{
					BadChoice *bc = new BadChoice(this, "Stop refining before "
					                              "changing the footprints.");
					setModal(bc);
					return;
				}

				_model->setFootprints(!_model->footprintsOn());
				std::cout << "Footprints are now " 
				<< (_model->footprintsOn() ? "on" : "off") 
				<< "; re-score saved results to bring them in line" 
				<< std::endl;
			});
		}
		if (_model && _model->isRefining())
		{
			menu->addOption("stop refining", [this]() { _model->stopRefining(); });
		}
		else
		{
			menu->addOption("refine", [this]() { refine(); });
		}
		// always there: results come in while refining, and the list fills as
		// they do
		menu->addOption("view results", [this]() { viewResults(); });
		if (_model && _model->results().size())
		{
			menu->addOption("re-score results", [this]()
			{
				size_t n = _model->rescoreResults();
				std::cout << "Re-scored " << n << " results" << std::endl;

				if (_rv)
				{
					_rv->update(_model->results());
				}
			});
		}
		menu->setup(tb, 1);
		setModal(menu);
	};
	
	tb->setReturnJob(show_menu);
	addObject(tb);
	
	Display::setup();
}

// solid faces for the antigen, which hide what is behind them, or the wire
void Playground::setOpaque(bool opaque)
{
	if (!_mesh)
	{
		return;
	}

	_mesh->setOpaque(opaque);
	_ghost->setOverSolid(opaque);

	// Each antibody stands a quarter of its width out from the surface, so
	// that it is not so buried in the solid antigen
	for (AbWatch *aw : _watches)
	{
		if (opaque)
		{
			Mesh *mesh = _mesh;
			float lift = 0.25f * 2 * aw->radius();
			aw->setDisplace([mesh, lift](const glm::vec3 &p)
			{
				return mesh->outwardAt(p) * lift;
			});
		}
		else
		{
			aw->setDisplace({});
		}
	}
}

// how far the ghost, which stands for a position, is raised like the
// antibodies are over a solid antigen
glm::vec3 Playground::surfaceLift(const glm::vec3 &p)
{
	if (!_mesh || !_mesh->opaque() || _watches.empty())
	{
		return glm::vec3(0.f);
	}

	return _mesh->outwardAt(p) * (0.25f * 2 * _watches.front()->radius());
}

void Playground::unhighlight(AbWatch *aw)
{
	if (std::find(_searchHits.begin(), _searchHits.end(), aw) == 
	    _searchHits.end())
	{
		aw->setHighlighted(false);
	}
}

void Playground::keyPressEvent(SDL_Keycode pressed)
{
	// while text is being entered every key, "g" included, goes to the entry;
	// Display would otherwise take "g" to focus on a residue
	if (keyResponder())
	{
		Mouse2D::keyPressEvent(pressed);
		return;
	}

	if (pressed == SDLK_g && !lastModal())
	{
		startSearch();
		return;
	}

	Display::keyPressEvent(pressed);
}

void Playground::startSearch()
{
	SearchEntry *se = new SearchEntry("search antibody", this);
	se->setValidationType(TextEntry::None);
	se->setChangedJob([this](const std::string &term) { updateSearch(term); });
	se->setReturnJob([this]() { finishSearch(); });
	se->setCentre(0.5, 0.3);
	addObject(se);
	_search = se;

	addMainThreadJob([se]() { se->click(); });
}

void Playground::updateSearch(const std::string &term)
{
	std::vector<AbWatch *> old = _searchHits;
	_searchHits.clear();

	for (AbWatch *aw : old)
	{
		aw->setHighlighted(false);
	}

	std::string lower = term;
	std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

	if (lower.length() == 0)
	{
		return;
	}

	// a name which is exactly what was typed wins over longer names which
	// merely contain it (AB4 over AB40)
	bool exact = false;
	std::vector<std::pair<AbWatch *, bool>> matches;

	for (AbWatch *aw : _watches)
	{
		std::string name = aw->antibody();
		std::transform(name.begin(), name.end(), name.begin(), ::tolower);

		if (name.find(lower) != std::string::npos)
		{
			matches.push_back({aw, name == lower});
			exact |= (name == lower);
		}
	}

	for (const auto &match : matches)
	{
		if (!exact || match.second)
		{
			_searchHits.push_back(match.first);
			match.first->setHighlighted(true);
		}
	}
}

void Playground::finishSearch()
{
	std::set<std::string> names;
	for (AbWatch *aw : _searchHits)
	{
		names.insert(aw->antibody());
	}

	// one antibody matched: its first copy
	bool centre = (names.size() == 1);
	glm::vec3 where = centre ? _searchHits[0]->centroid() : glm::vec3(0.f);

	if (_search)
	{
		removeObject(_search);
		Window::setDelete(_search);
		_search = nullptr;
	}

	// the matches stay highlighted until a click on empty space; a single
	// antibody is pinned, with its label and mesh faces
	if (centre)
	{
		pin(_searchHits[0]);
		shiftToCentre(where, 0);
	}
}

// as if the antibody had been searched for: all its copies highlighted, the
// first one pinned and centred
void Playground::showAntibody(const std::string &name)
{
	clearSelection();

	for (AbWatch *aw : _watches)
	{
		if (aw->antibody() == name)
		{
			_searchHits.push_back(aw);
			aw->setHighlighted(true);
		}
	}

	if (_searchHits.empty())
	{
		return;
	}

	pin(_searchHits[0]);
	shiftToCentre(_searchHits[0]->centroid(), 0);
}

void Playground::highlightRows()
{
	for (Box *quad : _rowQuads)
	{
		removeObject(quad);
		delete quad;
	}
	_rowQuads.clear();

	if (!_pinned || !_matricesVisible || _mab.competitions.empty())
	{
		return;
	}

	const std::vector<std::string> &names = 
	_mab.competitions.begin()->favoured_ordering;
	auto at = std::find(names.begin(), names.end(), _pinned->antibody());
	if (at == names.end())
	{
		return;
	}

	size_t row = at - names.begin();
	for (MatrixPlot *plot : {_mpData, _mpModel})
	{
		if (!plot)
		{
			continue;
		}

		// the plot's rows evenly divide its extent, with row 0 at the bottom
		glm::vec3 minC, maxC;
		plot->bounds(minC, maxC);
		float rowSpan = (maxC.y - minC.y) / (float)names.size();
		float bottomY = minC.y + (float)row * rowSpan;

		Box *quad = new Box();
		quad->makeQuad();
		quad->rescale(fabs(maxC.x - minC.x) / 2.f, rowSpan / 2.f);
		quad->setPosition(glm::vec3((maxC.x + minC.x) / 2.f, 
		                            bottomY + rowSpan / 2.f, minC.z + 0.05f));
		// as in the cluster view: a Box with no image samples whatever was
		// last drawn, and black gives the translucent band
		quad->setImage("assets/images/black.png");
		quad->setColour(0.1, 1.0, 0.2);
		quad->setAlpha(-0.6);
		addObject(quad);
		_rowQuads.push_back(quad);
	}
}

void Playground::pin(AbWatch *aw)
{
	_pinned = aw;
	highlightRows();

	for (AbWatch *other : _watches)
	{
		if (other != aw && other->antibody() == aw->antibody())
		{
			other->highlightQuietly();
		}
	}

	aw->highlight();
	showGhost(aw);
	showHeat(aw);
}

// for an antibody which has a structure but is not fixed to it just now: a
// ghost where the structure puts it, snapped to the mesh as it would be if
// fixed (the symmetry copy nearest to where this
// one is), and a line to it from where this one is
void Playground::showGhost(AbWatch *aw)
{
	const std::string &name = aw->antibody();
	std::vector<std::string> known = _positions->structuralAntibodies();

	if (_positions->isFixed(name) || 
	    std::find(known.begin(), known.end(), name) == known.end())
	{
		return;
	}

	glm::vec3 here = aw->centroid();
	glm::vec3 target = here;
	float nearestSq = FLT_MAX;
	for (const glm::vec3 &copy : _positions->structuralCopies(name, true))
	{
		glm::vec3 diff = copy - here;
		float sq = glm::dot(diff, diff);
		if (sq < nearestSq)
		{
			nearestSq = sq;
			target = copy;
		}
	}

	if (nearestSq < FLT_MAX)
	{
		target += surfaceLift(target);
		_ghost->show(target);
		_ghostLine->show(here, target);
	}
}

void Playground::alignmentMenu(AbWatch *clicked)
{
	const std::string &name = clicked->antibody();
	Fiducial *fid = _positions->fiducialFor(name);
	if (!fid || !fid->contact)
	{
		return;
	}

	Menu *menu = new Menu(this);

	if (fid->contact->orientations().empty())
	{
		menu->addOption("no alignment details kept for this antibody", 
		                []() {});
	}
	else
	{
		menu->addOption("show alignment", [this, clicked]()
		{
			showAlignment(clicked);
		});
	}

	float x, y;
	getFractionalPos(x, y);
	menu->setup(x, y, 1);
	setModal(menu);
}

// the antibody as the alignment of the copy clicked puts it on the mesh (the
// copies are in the order of the orientations), glowing like the others
void Playground::exportPdb(const std::string &key, const std::string &name,
                           const std::vector<RefinedResult> &results)
{
	Competition &comp = *_mab.competitions.begin();
	Antigen *antigen = _mab.antigens(comp.antigen);

	// the same numbers as the cross-check gives, for every antibody
	CrossCheckView::Stats stats = CrossCheckView::compute(*_positions, comp, 
	                                                      key, results);

	// a file name from the antigen and the group, not over one already there
	std::string base = comp.antigen + "_" + name;
	for (char &c : base)
	{
		if (!std::isalnum((unsigned char)c) && c != '-' && c != '_')
		{
			c = '_';
		}
	}

	std::string path = base + ".pdb";
	for (int i = 2; std::ifstream(path).good(); i++)
	{
		path = base + "_" + std::to_string(i) + ".pdb";
	}

	std::string reference = (antigen ? antigen->model.filename() : comp.antigen);
	std::string title = name + (key.length() ? " (fixed: " + key + ")" : 
	                            " (none fixed)");

	std::string message;
	try
	{
		AntibodyPdb::write(path, stats.everyone, reference, title, 
		                   results.size());
		message = "Wrote " + std::to_string(stats.everyone.size()) + 
		" antibodies to " + path;
	}
	catch (const std::exception &err)
	{
		message = std::string("Could not export: ") + err.what();
	}

	std::cout << message << std::endl;
	BadChoice *bc = new BadChoice(this, message);
	setModal(bc);
}

// The orientation which puts the antibody nearest to where the copy selected
// sits: by position, not by the copy's number, which need not line up. The
// structural positions are in the order of the orientations (any more are
// symmetry copies which have no orientation of their own).
size_t Playground::nearestOrientation(AbWatch *aw, Fiducial *fid)
{
	const std::vector<ContactPoint::Orientation> &orientations = 
	fid->contact->orientations();
	std::vector<glm::vec3> structural = 
	_positions->structuralCopies(aw->antibody(), true);
	glm::vec3 here = aw->centroid();
	size_t which = 0;
	float nearestSq = FLT_MAX;
	for (size_t i = 0; i < structural.size() && i < orientations.size(); i++)
	{
		glm::vec3 diff = structural[i] - here;
		float sq = glm::dot(diff, diff);
		if (sq < nearestSq)
		{
			nearestSq = sq;
			which = i;
		}
	}

	return which;
}

void Playground::checkAlignment()
{
	Fiducial *fid = (_pinned ? _positions->fiducialFor(_pinned->antibody()) : 
	                 nullptr);
	if (!fid || !fid->contact || fid->contact->orientations().empty())
	{
		BadChoice *bc = new BadChoice(this, "Click on an antibody with fixed "
		                              "fiducial data first.");
		setModal(bc);
		return;
	}

	Antigen *antigen = _mab.antigens(fid->antigen);
	if (!antigen)
	{
		return;
	}

	const std::vector<ContactPoint::Orientation> &orientations = 
	fid->contact->orientations();
	size_t which = nearestOrientation(_pinned, fid);
	const ContactPoint::Orientation &o = orientations[which];

	bool unknown = (o.rmsd != o.rmsd);
	std::string rmsd = unknown ? "unknown" : f_to_str(o.rmsd, 1) + " A";
	std::string text = "Orientation " + std::to_string(which + 1) + 
	" of antibody " + fid->name + " aligns with RMSD " + rmsd;
	bool poor = unknown || o.rmsd > ContactPoint::MAX_RMSD;

	AlignmentView *av = new AlignmentView(this, *antigen, *fid, o.transform,
	                                      text, poor);
	av->setComparison(_mab.fiducials);
	av->show();
}

void Playground::showAlignment(AbWatch *clicked)
{
	Fiducial *fid = _positions->fiducialFor(clicked->antibody());
	if (!fid || !fid->contact || !_mesh)
	{
		return;
	}

	Antigen *antigen = _mab.antigens(fid->antigen);
	const std::vector<ContactPoint::Orientation> &orientations = 
	fid->contact->orientations();
	if (!antigen || orientations.empty())
	{
		return;
	}

	// only one orientation of an antibody can be shown at once: the model
	// has only the one set of atoms, which are moved to where it is shown
	for (size_t i = 0; i < _alignments.size(); i++)
	{
		if (_alignments[i].fid == fid)
		{
			ShownAlignment gone = _alignments[i];
			_alignments.erase(_alignments.begin() + i);
			removeDisplayUnit(gone.unit);
			delete gone.atoms;
			fid->model.unload();
			break;
		}
	}

	size_t which = nearestOrientation(clicked, fid);
	glm::mat4x4 transform = orientations[which].transform;

	fid->model.load();

	// the antibody alone, glowing so that it stands out from the mesh
	std::vector<Instance *> chains;
	for (Instance *inst : fid->model.instances())
	{
		if (!antigen->entities.count(inst->entity_id()))
		{
			chains.push_back(inst);
		}
	}

	// Just the one antibody, not every copy the model holds. The reference
	// (the middle of the interface) is where the first orientation puts it on
	// the mesh, so it is taken back through that to the model as it was loaded,
	// where the nearest chain is the antibody making the interface.
	glm::vec3 reference = glm::vec3(glm::inverse(orientations.front().transform) *
	                                glm::vec4(fid->contact->reference(), 1.f));
	AtomGroup *atoms = new AtomGroup();
	for (Instance *inst : ContactPoint::antibodyAtInterface(chains, reference))
	{
		atoms->add(inst->currentAtoms());
		for (Atom *atom : inst->currentAtoms()->atomVector())
		{
			atom->setAddedColour(1.f);
		}
	}

	// It is shown first and moved afterwards, as the representations follow
	// the atoms through the unit's watch, which is only there once the unit
	// has been displayed.
	DisplayUnit *unit = new DisplayUnit(this);
	unit->loadAtoms(atoms);
	unit->displayAtoms();
	unit->startWatch();
	addDisplayUnit(unit);
	_alignments.push_back({fid, unit, atoms});
	markFootprints();

	// displaying the atoms re-centres the view on the antibody where it
	// started, not where the alignment puts it, so the view is brought back
	// to the antigen once it has been moved
	glm::vec3 centre = _mesh->centroid();
	addMainThreadJob([this, atoms, transform, centre]()
	{
		for (Atom *atom : atoms->atomVector())
		{
			glm::vec3 d = atom->derivedPosition();
			atom->setDerivedPosition(glm::vec3(transform * glm::vec4(d, 1.f)));
		}

		shiftToCentre(centre, 0);
	});
}

void Playground::clearAlignments()
{
	for (ShownAlignment &shown : _alignments)
	{
		removeDisplayUnit(shown.unit);
		delete shown.atoms;
		shown.fid->model.unload();
	}

	_alignments.clear();
	markFootprints();
}

void Playground::markFootprints()
{
	if (!_mesh || !_positions)
	{
		return;
	}

	std::set<int> marked;
	for (const ShownAlignment &shown : _alignments)
	{
		for (int vertex : _positions->footprintVertices(shown.fid->name))
		{
			marked.insert(vertex);
		}
	}

	_mesh->setFootprintMarks(marked);
}

Playground::~Playground()
{
	clearAlignments();
}

void Playground::clearGhost()
{
	_ghost->hide();
	_ghostLine->hide();
}

void Playground::colourByMatrix(ColourBy which)
{
	if (!_pinned)
	{
		return;
	}

	_colourBy = (_colourBy == which ? Nothing : which);

	if (_colourBy == Nothing)
	{
		refreshFixedStyles();
		return;
	}

	applyColouring();
}

void Playground::applyColouring()
{
	MatrixPlot *plot = (_colourBy == Data ? _mpData : _mpModel);
	std::map<std::string, std::pair<float, bool>> row;

	if (!_pinned || _colourBy == Nothing || !plot ||
	    !_model->rowValues(_pinned->antibody(), _colourBy == Model, row))
	{
		return;
	}

	const glm::vec4 grey = glm::vec4(0.6, 0.6, 0.6, 1.0);

	for (AbWatch *aw : _watches)
	{
		if (aw->antibody() == _pinned->antibody())
		{
			continue;
		}

		// the data has gaps, but the model has a value for every pair
		auto it = row.find(aw->antibody());
		bool present = (it != row.end() && (it->second.second || 
		                                    _colourBy == Model));
		aw->showColour(present ? plot->legend()->colour(it->second.first) : 
		               grey);
	}
}

void Playground::clearSelection()
{
	_pinned = nullptr;
	highlightRows();
	clearGhost();

	if (_colourBy != Nothing)
	{
		_colourBy = Nothing;
		refreshFixedStyles();
	}

	std::vector<AbWatch *> hits = _searchHits;
	_searchHits.clear();

	for (AbWatch *aw : _watches)
	{
		aw->setHighlighted(false);
		aw->deleteTemps();
	}

	clearHeat();
	dropNarrowing(true);
}

void Playground::sendSelection(float t, float l, float b, float r, 
                               bool inverse)
{
	if (!_mesh)
	{
		return;
	}

	convertToGLCoords(&l, &t);
	convertToGLCoords(&r, &b);

	// With an antibody pinned and its faces showing, it is the dark grey ones
	// which can be picked (to pick out the results which landed on them)
	bool heat = (_pinned && _heatAntibody.length() && _heatMain.size());

	std::set<int> picked = _mesh->selection();
	for (int idx : _mesh->verticesInBox(t, l, b, r))
	{
		if (heat && !_heatMain.count(idx))
		{
			continue;
		}

		if (inverse)
		{
			picked.erase(idx);
		}
		else
		{
			picked.insert(idx);
		}
	}

	_mesh->setSelection(picked);

	if (heat)
	{
		narrowToSelection();
	}
}

// the results with a point in the dark grey cluster on a vertex which is
// picked become the ones on show
void Playground::narrowToSelection()
{
	const std::set<int> &picked = _mesh->selection();
	std::set<size_t> keep;

	for (size_t p = 0; p < _heatClusters.points.size(); p++)
	{
		if (_heatClusters.member[p] == _heatNearest && 
		    picked.count(_heatVertex[p]))
		{
			keep.insert(_heatClusters.resultOf[p]);
		}
	}

	if (keep.empty())
	{
		// nothing of the dark grey picked: all of the results again
		bool was = _narrowed;
		_narrowed = false;
		_narrowedResults.clear();

		if (was && !_selectedResults.empty())
		{
			showMatrices();
			_model->showAverage(_selectedResults);
			applyColouring();
		}
		return;
	}

	_narrowedResults.clear();
	for (size_t r : keep)
	{
		if (r < _selectedResults.size())
		{
			_narrowedResults.push_back(_selectedResults[r]);
		}
	}

	_narrowed = true;
	std::cout << keep.size() << " of " << _selectedResults.size() 
	<< " results landed on the picked faces" << std::endl;

	showMatrices();
	_model->showAverage(_narrowedResults);
	applyColouring();

	if (_pinned)
	{
		clearGhost();
		showGhost(_pinned);
	}
}

void Playground::dropNarrowing(bool restoreAverage)
{
	if (!_narrowed)
	{
		return;
	}

	_narrowed = false;
	_narrowedResults.clear();

	if (_mesh)
	{
		_mesh->setSelection({});
	}

	if (restoreAverage && _model && !_selectedResults.empty())
	{
		showMatrices();
		_model->showAverage(_selectedResults);
		applyColouring();
	}
}

// shift+click rather than +drag: lets go of the vertices picked
void Playground::sendClickSelection(double x, double y, bool inverse)
{
	if (_mesh)
	{
		_mesh->setSelection({});
	}

	dropNarrowing(true);
}

void Playground::meshMenu()
{
	if (!_mesh)
	{
		return;
	}

	bool picked = _mesh->selection().size() > 0;
	bool excluded = _mesh->hasExclusions();
	bool group = (_narrowed && _narrowedResults.size() && _rv);
	if (!picked && !excluded && !group)
	{
		return;
	}

	Menu *menu = new Menu(this);

	if (group)
	{
		// the results which landed on the faces picked, into the results list
		menu->addOption("make group", [this]()
		{
			std::string name = (_pinned ? _pinned->antibody() + " site" : 
			                    "selected");
			_rv->addGroup(_selectedKey, name, _narrowedResults);
		});
	}

	if (picked)
	{
		menu->addOption("exclude from start", [this]()
		{
			_mesh->excludeSelection();
			_mab.save();
		});
	}

	if (excluded)
	{
		menu->addOption("clear exclusions", [this]()
		{
			_mesh->clearExclusions();
			_mab.save();
		});
	}

	float x, y;
	getFractionalPos(x, y);
	menu->setup(x, y, 1);
	setModal(menu);
}

void Playground::interactedWithNothing(bool left, bool hover)
{
	if (!hover && !left)
	{
		meshMenu();
		return;
	}

	if (!hover)
	{
		// A click on empty space first lets go of the alignments shown, and
		// leaves the antibody picked as it is: it is the next click which
		// lets go of everything else.
		if (_alignments.size())
		{
			clearAlignments();
			return;
		}

		clearSelection();
		if (_mesh)
		{
			_mesh->setSelection({});
		}
		return;
	}

	if (_pinned)
	{
		// only what was hovered over since goes; the pinned antibody stays
		for (AbWatch *aw : _watches)
		{
			if (aw->antibody() != _pinned->antibody())
			{
				unhighlight(aw);
				aw->deleteTemps();
			}
		}

		return;
	}

	clearGhost();
	clearHeat();

	for (AbWatch *aw : _watches)
	{
		unhighlight(aw);
		aw->deleteTemps();
	}
}

// text centred just under a matrix plot centred at (x, y)
void Playground::label(MatrixPlot *plot, float x, float y,
                       const std::string &text)
{
	// maximalHeight() is in GL units (the whole screen being 2 high)
	y += plot->maximalHeight() / 4 + 0.03;

	Text *t = new Text(text);
	t->resize(0.5);
	t->setCentre(x, y);
	addObject(t);
	_matrixLabels.push_back(t);
}

void Playground::setMatricesVisible(bool visible)
{
	showMatrices();
	_matricesVisible = visible;
	_mpModel->setDisabled(!visible);
	_mpData->setDisabled(!visible);
	highlightRows();

	for (Text *t : _matrixLabels)
	{
		t->setDisabled(!visible);
	}
}

void Playground::showMatrices()
{
	Competition &comp = *_mab.competitions.begin();
	if (!_model)
	{
		_model = new PosToComp(comp, *_positions);
	}

	if (!_mpModel)
	{
		std::mutex *mutex{};
		PCA::Matrix &display = _model->modelDisplay(&mutex);
	
		MatrixPlot *mp = new MatrixPlot(display, *mutex);
		Eigen::MatrixXf mat = _model->fromModel();

		mp->resize(0.4);
		mp->setCentre(0.9, 0.85);
		addObject(mp);
		_mpModel = mp;

		// a press only finds what is selectable (see MatrixBox)
		mp->setSelectable(true);
		mp->setClickJob([this]() { colourByMatrix(Model); });
		label(mp, 0.9, 0.85, "model");
	}

	if (!_mpData)
	{
		std::mutex *mutex{};
		PCA::Matrix &display = _model->dataDisplay(&mutex);

		MatrixPlot *mp = new MatrixPlot(display);
		mp->resize(0.4);
		mp->setCentre(0.75, 0.85);
		addObject(mp);
		_mpData = mp;

		mp->setSelectable(true);
		mp->setClickJob([this]() { colourByMatrix(Data); });
		label(mp, 0.75, 0.85, "data");
		_model->setDataPlot(mp);
	}
	
	_model->setPlot(_mpModel);
}

// the free/fixed colouring and selectability of the antibodies
void Playground::refreshFixedStyles()
{
	for (AbWatch *aw : _watches)
	{
		aw->setFixedStyle(_positions->isFixed(aw->antibody()));
	}
}

// how much an antibody's radius changes with the curvature of the mesh under
// it, typed in: Angstroms for each standard deviation (0 for none)
void Playground::askForCurvatureScale()
{
	if (!_model)
	{
		return;
	}

	if (_model->isRefining())
	{
		BadChoice *bc = new BadChoice(this, "Stop refining before changing the "
		                              "curvature scale.");
		setModal(bc);
		return;
	}

	AskForText *aft = new AskForText(this, "Radius change per standard "
	                                 "deviation of curvature (Angstroms, 0 for "
	                                 "none):", "", nullptr, TextEntry::Numeric);
	aft->setDefaultText(f_to_str(_model->curvatureScale(), 1));
	aft->setReturnJob([this](std::string text)
	{
		float scale = 0;
		try
		{
			scale = std::stof(text);
		}
		catch (const std::exception &err)
		{
			scale = 0;
		}

		if (!(scale >= 0) || scale > 20)
		{
			BadChoice *bc = new BadChoice(this, "The curvature scale must be "
			                              "from 0 to 20 Angstroms.");
			setModal(bc);
			return;
		}

		_model->setCurvatureScale(scale);
		std::cout << "Curvature scale is now " << scale << " A per standard "
		<< "deviation; re-score saved results to bring them in line" 
		<< std::endl;
	});
	setModal(aft);
}

// the antibody radius, typed in: how close two antibodies have to be to
// compete. A refinement in progress uses it, so not then.
void Playground::askForRadius()
{
	if (!_model)
	{
		return;
	}

	if (_model->isRefining())
	{
		BadChoice *bc = new BadChoice(this, "Stop refining before changing the "
		                              "antibody radius.");
		setModal(bc);
		return;
	}

	AskForText *aft = new AskForText(this, "Antibody radius (Angstroms):", "",
	                                 nullptr, TextEntry::Numeric);
	aft->setDefaultText(f_to_str(_model->radius(), 1));
	aft->setReturnJob([this](std::string text)
	{
		float radius = 0;
		try
		{
			radius = std::stof(text);
		}
		catch (const std::exception &err)
		{
			radius = 0;
		}

		if (!(radius > 0) || radius > 1000)
		{
			BadChoice *bc = new BadChoice(this, "The antibody radius must be a "
			                              "positive number of Angstroms.");
			setModal(bc);
			return;
		}

		_model->setRadius(radius);

		// results made with another radius are scored as they were
		std::cout << "Antibody radius is now " << radius << " A; re-score "
		<< "saved results to bring them in line" << std::endl;
	});
	setModal(aft);
}

// which antibodies are fixed now, for the json
void Playground::keepFixed()
{
	if (!_positions || _mab.competitions.empty())
	{
		return;
	}

	Competition &comp = *_mab.competitions.begin();
	comp.fixed_antibodies = _positions->fixedAntibodies();
	comp.fixed_saved = true;
}

void Playground::showToggleFixed()
{
	if (_model && _model->isRefining())
	{
		std::cout << "Stop refining before changing which antibodies are fixed."
		<< std::endl;
		return;
	}

	ToggleFixedView *tfv = new ToggleFixedView(this, *_positions);
	tfv->setChangedJob([this]()
	{
		clearSelection();
		refreshFixedStyles();

		// kept in the json, so that it is as it was left
		keepFixed();
		_mab.save();
	});
	tfv->show();
}

void Playground::refine()
{
	showMatrices();
	_model->refine();
}

void Playground::selectResults(const std::string &key,
                               const std::vector<RefinedResult> &results)
{
	clearHeat();
	dropNarrowing(false);
	_selectedResults = results;
	_selectedKey = key;

	if (results.empty())
	{
		// let go of the group: back to watching the live positions
		resumeLive();
		return;
	}

	// The antibodies fixed for these results are fixed now, and the rest of
	// those with a structure are free. Not while refining though: that
	// would pull the rug from under it, so the set stays as it is.
	if (!_model->isRefining())
	{
		std::vector<std::string> fixed = ResultsView::keyNames(key);
		for (const std::string &name : _positions->structuralAntibodies())
		{
			bool isFixed = (std::find(fixed.begin(), fixed.end(), name) != 
			                fixed.end());
			_positions->setFixed(name, isFixed);
		}
		refreshFixedStyles();
	}

	// refinement can carry on behind this, only the matrix follows it
	showMatrices();
	_model->showAverage(results);
	applyColouring();

	// an antibody already clicked on goes with the new group: its faces came
	// from the last one, and the average may have moved it
	if (_pinned)
	{
		clearGhost();
		showGhost(_pinned);
		showHeat(_pinned);
	}
}

void Playground::resumeLive()
{
	if (!_positions->viewing())
	{
		return;
	}

	_positions->resumeLive();
	clearSelection();

	if (!_model->isRefining())
	{
		_model->fromModel();
	}
}

// the cluster nearest to where the hovered symmetry copy actually sits, over
// the selected group's runs. Going by position rather than by the copy's
// number matters for a fixed antibody, whose copies sit at their structural
// positions, which need not line up with the clusters numbered to match.
void Playground::showHeat(AbWatch *aw)
{
	const std::string &antibody = aw->antibody();
	std::string key = antibody + "#" + std::to_string(aw->copy()) +
	(_positions->isFixed(antibody) ? "f" : "");
	if (_selectedResults.empty() || key == _heatAntibody || !_mesh)
	{
		return;
	}

	// a different antibody (or copy) is a different set of faces to pick from
	dropNarrowing(true);

	std::vector<const std::vector<Positions::Placement> *> all;
	for (const RefinedResult &r : _selectedResults)
	{
		all.push_back(&r.placements);
	}

	Positions::Clusters clusters;
	if (!Positions::cluster(all, antibody, clusters))
	{
		return;
	}

	glm::vec3 where = aw->centroid();
	size_t nearest = 0; float nearestSq = FLT_MAX;
	for (size_t k = 0; k < clusters.centres.size(); k++)
	{
		glm::vec3 diff = clusters.centres[k] - where;
		float sq = glm::dot(diff, diff);
		if (sq < nearestSq)
		{
			nearestSq = sq;
			nearest = k;
		}
	}

	std::map<int, int> counts, others;
	std::vector<int> vertexOf(clusters.points.size(), -1);
	for (size_t p = 0; p < clusters.points.size(); p++)
	{
		bool mine = (clusters.member[p] == nearest);
		int vertex = _mesh->nearestVertex(clusters.points[p]);
		vertexOf[p] = vertex;
		(mine ? counts : others)[vertex]++;
	}

	_heatAntibody = key;
	_heatClusters = clusters;
	_heatNearest = nearest;
	_heatMain = counts;
	_heatVertex = vertexOf;
	_mesh->setHeat(counts, others);
}

void Playground::clearHeat()
{
	if (_heatAntibody.length() && _mesh)
	{
		_mesh->clearHeat();
	}

	_heatAntibody.clear();
	_heatMain.clear();
	_heatVertex.clear();
	_heatClusters = Positions::Clusters{};
}

void Playground::viewResults()
{
	// the model holds the results, and is made with the matrices
	if (!_model)
	{
		showMatrices();
	}

	if (!_rv)
	{
		// refinement can carry on behind this: the antibodies stop following
		// the live positions and show the result instead
		auto apply = [this](const PosToComp::Result &result)
		{
			showMatrices();
			_model->showResult(result);
			applyColouring();
		};

		_rv = new ResultsView(this, apply);

		// the groups saved in the json
		for (const ResultGroup &group : _mab.competitions.begin()->groups)
		{
			_rv->addGroup(group.key, group.name, group.results);
		}

		_rv->setClusterJob([this](const std::string &key,
		                          const std::vector<PosToComp::Result> &results)
		{
			ResultsClusterView *cv = new ResultsClusterView(this, results);
			cv->setGroupJob([this, key](const std::vector<PosToComp::Result> &picked)
			{
				_rv->addGroup(key, "best", picked);
			});
			cv->show();
		});

		_rv->setCrossCheckJob([this](const std::string &key,
		                             const std::string &name,
		                             const std::vector<PosToComp::Result> &results)
		{
			CrossCheckView::Stats stats = CrossCheckView::compute(*_positions,
			                                                      *_mab.competitions.begin(),
			                                                      key, results);
			std::string title = name + " - " + (key.length() ? 
			"fixed abs " + key : "no fixed abs");
			CrossCheckView *ccv = new CrossCheckView(this, title, stats);

			// clicking a free fiducial comes back here, centred on it as if
			// it had been searched for
			ccv->setSelectJob([this](const std::string &name)
			{
				addMainThreadJob([this, name]() { showAntibody(name); });
			});
			ccv->show();
		});

		_rv->setExportJob([this](const std::string &key, const std::string &name,
		                         const std::vector<PosToComp::Result> &results)
		{
			exportPdb(key, name, results);
		});

		_rv->setSelectJob([this](const std::string &key,
		                         const std::vector<RefinedResult> &results)
		{
			selectResults(key, results);
		});

		_rv->setDeleteJob([this](const std::string &key, size_t count)
		{
			_model->removeResults([key](const PosToComp::Result &r)
			{
				return ResultsView::bucketKey(r) == key;
			});
			_rv->update(_model->results());
		});

		// keep the "All (n results)" list current as refinement goes on
		_model->setResultJob([this]()
		{
			addMainThreadJob([this]()
			{
				_rv->update(_model->results());
			});
		});

		_rv->setKillAndClean([this]() { removeObject(_rv); });
	}

	_rv->update(_model->results());

	if (!hasObject(_rv))
	{
		addObject(_rv);
	}
}

void Playground::saveResults()
{
	// results live in _model while running; only copy them across (under
	// its lock) here, on the main thread, so a refinement in progress can't
	// race the json being written.
	_mab.competitions.begin()->results = _model->results();
	keepFixed();

	// the groups only change once the list has been opened
	if (_rv)
	{
		_mab.competitions.begin()->groups = _rv->groups();
	}
	_mab.save();
}

void Playground::showCompetitionData()
{
	if (_model->isRefining())
	{
		std::cout << "Stop refining before changing the competition data."
		<< std::endl;
		return;
	}

	AntibodyOrderingView *aov = new AntibodyOrderingView(this, _mab);
	aov->setReturnJob([this]()
	{
		syncAntibodies();
		_model->reloadData();
		highlightRows();
	});
	aov->show();
}
