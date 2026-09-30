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
#include "Positions.h"
#include "PosToComp.h"
#include "AbWatch.h"
#include "Mesh.h"
#include "AntibodyOrderingView.h"
#include "ResultsView.h"
#include "ResultsClusterView.h"
#include "Mab.h"
#include <vagabond/gui/elements/Icosahedron.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/Menu.h>
#include <vagabond/gui/elements/Text.h>
#include <vagabond/gui/MatrixPlot.h>

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
		aw->setHoverJob([this, aw](const std::string &ab)
		{
			// only one antibody at a time: sliding straight from one onto
			// another never passes over empty space to clear the last
			for (AbWatch *other : _watches)
			{
				if (other != aw)
				{
					other->setHighlighted(false);
					other->deleteTemps();
				}
			}

			showHeat(ab);
		});
	}
}

void Playground::setup()
{
	Competition &comp = *_mab.competitions.begin();
	showMesh(comp);
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

void Playground::interactedWithNothing(bool left, bool hover)
{
	clearHeat();

	for (AbWatch *aw : _watches)
	{
		aw->setHighlighted(false);
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

void Playground::refine()
{
	showMatrices();
	_model->refine();
}

void Playground::selectResults(const std::vector<RefinedResult> &results)
{
	clearHeat();
	_selectedResults = results;

	std::vector<const std::vector<Positions::Placement> *> all;
	for (const RefinedResult &r : _selectedResults)
	{
		all.push_back(&r.placements);
	}
	_selectedOps = Positions::alignOperators(all);

	if (results.empty())
	{
		return;
	}

	if (_model->isRefining())
	{
		std::cout << "Stop refining before moving to a group's average."
		<< std::endl;
		return;
	}

	showMatrices();
	_model->showAverage(results);
}

// how often the antibody sat on each mesh vertex across the selected group
void Playground::showHeat(const std::string &antibody)
{
	if (_selectedResults.empty() || antibody == _heatAntibody || !_mesh)
	{
		return;
	}

	std::map<int, int> counts;
	for (size_t i = 0; i < _selectedResults.size(); i++)
	{
		// the copy that lines up with the rest under the best symmetry
		// operator, rather than always the first
		for (const Positions::Placement &pl : _selectedResults[i].placements)
		{
			size_t op = _selectedOps[i];
			if (pl.name != antibody || pl.copies.empty())
			{
				continue;
			}

			op = (op < pl.copies.size() ? op : 0);
			counts[_mesh->nearestVertex(pl.copies[op])]++;
		}
	}

	_heatAntibody = antibody;
	_mesh->setHeat(counts);
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
		auto apply = [this](const PosToComp::Result &result)
		{
			if (_model->isRefining())
			{
				std::cout << "Stop refining before viewing a result."
				<< std::endl;
				return;
			}

			showMatrices();
			_model->showResult(result);
		};

		_rv = new ResultsView(this, apply);
		_rv->setClusterJob([this](const std::vector<PosToComp::Result> &results)
		{
			ResultsClusterView *cv = new ResultsClusterView(this, results);
			cv->setGroupJob([this](const std::vector<PosToComp::Result> &picked)
			{
				_rv->addGroup("best", picked);
			});
			cv->show();
		});

		_rv->setSelectJob([this](const std::vector<RefinedResult> &results)
		{
			selectResults(results);
		});

		_rv->setDeleteJob([this](size_t count)
		{
			_model->removeResults(count);
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
