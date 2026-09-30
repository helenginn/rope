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
#include <set>
#include "Positions.h"
#include "PosToComp.h"
#include "AbWatch.h"
#include "Mesh.h"
#include "AntibodyOrderingView.h"
#include "ResultsView.h"
#include "ResultsClusterView.h"
#include "ToggleFixedView.h"
#include "CrossCheckView.h"
#include "GhostAb.h"
#include "Mab.h"
#include <vagabond/gui/elements/Icosahedron.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/Menu.h>
#include <vagabond/gui/elements/Text.h>
#include <vagabond/gui/elements/TextEntry.h>
#include <vagabond/gui/elements/Window.h>
#include <vagabond/gui/MatrixPlot.h>

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

}

void Playground::showFiducials(const Competition &comp)
{
	_positions = new Positions(*_mab.antigens(comp.antigen), comp, _mab,
	                           _mesh->vertexFinder(), _mesh->random());
	_watches = {};
	_positions->loadAntibodiesInto(this, _watches);
	
	for (AbWatch *aw : _watches)
	{
		addIndexResponder(aw);
		aw->setHoverJob([this, aw](const std::string &ab, int copy)
		{
			// a clicked antibody stays as it is
			if (_pinned)
			{
				return false;
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
		aw->setClickJob([this](AbWatch *clicked)
		{
			clearSelection();
			pin(clicked);
		});
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
	
	TextButton *tb = new TextButton("Menu");
	tb->setCentre(0.9, 0.1);

	auto show_menu = [this, tb]()
	{
		Menu *menu = new Menu(this);
		menu->addOption("save", [this]() { saveResults(); });
		menu->addOption("competition data", [this]() { showCompetitionData(); });
		if (_positions->viewing())
		{
			menu->addOption("watch live positions", [this]() { resumeLive(); });
		}
		menu->addOption("toggle fixed", [this]() { showToggleFixed(); });
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
		if (_model && _model->isRefining())
		{
			menu->addOption("stop refining", [this]() { _model->stopRefining(); });
		}
		else
		{
			menu->addOption("refine", [this]() { refine(); });
		}
		if (_model && _model->results().size())
		{
			menu->addOption("view results", [this]() { viewResults(); });
		}
		menu->setup(tb, 1);
		setModal(menu);
	};
	
	tb->setReturnJob(show_menu);
	addObject(tb);
	
	Display::setup();
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

void Playground::pin(AbWatch *aw)
{
	_pinned = aw;

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
		_ghost->show(target);
		_ghostLine->show(here, target);
	}
}

void Playground::clearGhost()
{
	_ghost->hide();
	_ghostLine->hide();
}

void Playground::clearSelection()
{
	_pinned = nullptr;
	clearGhost();

	std::vector<AbWatch *> hits = _searchHits;
	_searchHits.clear();

	for (AbWatch *aw : _watches)
	{
		aw->setHighlighted(false);
		aw->deleteTemps();
	}

	clearHeat();
}

void Playground::interactedWithNothing(bool left, bool hover)
{
	if (!hover)
	{
		// a click on empty space lets go of everything
		clearSelection();
		return;
	}

	if (_pinned)
	{
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
	_selectedResults = results;

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
	for (size_t p = 0; p < clusters.points.size(); p++)
	{
		bool mine = (clusters.member[p] == nearest);
		(mine ? counts : others)[_mesh->nearestVertex(clusters.points[p])]++;
	}

	_heatAntibody = key;
	_mesh->setHeat(counts, others);
}

void Playground::clearHeat()
{
	if (_heatAntibody.length() && _mesh)
	{
		_mesh->clearHeat();
	}

	_heatAntibody.clear();
}

void Playground::viewResults()
{
	if (!_rv)
	{
		// refinement can carry on behind this: the antibodies stop following
		// the live positions and show the result instead
		auto apply = [this](const PosToComp::Result &result)
		{
			showMatrices();
			_model->showResult(result);
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
			                                                      key, results);
			std::string title = name + " - " + (key.length() ? 
			"fixed abs " + key : "no fixed abs");
			CrossCheckView *ccv = new CrossCheckView(this, title, stats);
			ccv->show();
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
	aov->setReturnJob([this]() { _model->reloadData(); });
	aov->show();
}
