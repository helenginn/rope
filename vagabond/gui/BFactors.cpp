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

#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/ImageButton.h>
#include <vagabond/core/Entity.h>
#include "BFactors.h"
#include "MatrixPlot.h"
#include "ChooseHeader.h"

BFactors::BFactors(Scene *prev, Entity *entity) : Scene(prev)
{
	_entity = entity;
	_group = _entity->makeTorsionDataGroup();
}

void BFactors::setup()
{
	addTitle("B factor plot");

	float top = 0.2;

	{
		Text *t = new Text("Order molecules using header:");
		t->setLeft(0.2, top);
		t->addAltTag("Molecules will be ordered according to header value");
		addObject(t);
	}
	{
		ImageButton *t = ImageButton::arrow(-90., this);
		t->setReturnTag("choose_header");
		t->setCentre(0.8, top);
		addObject(t);
	}
	
	top += 0.08;

	{
		Text *t = new Text(_header);
		t->setRight(0.8, top);
		addObject(t);
		_headText = t;
	}

	{
		TextButton *t = new TextButton("Recalculate", this);
		t->setReturnTag("recalculate");
		t->setCentre(0.5, 0.9);
		addObject(t);
	}
	
	refresh();
}

void BFactors::refresh()
{
	_headText->setText(_header);
}

void BFactors::buttonPressed(std::string tag, Button *button)
{
	if (tag == "choose_header")
	{

		ChooseHeader *header = new ChooseHeader(this, true);
		header->setData(Environment::metadata(), &_group);
		header->setResponder(this);
		header->show();
	}
	
	if (tag == "recalculate")
	{
		drawPlot();
	}

	Scene::buttonPressed(tag, button);
}

void BFactors::sendObject(std::string tag, void *object)
{
	_header = tag;
	refresh();
}

std::vector<Instance *> findMolecules(Entity *entity, std::string header)
{
	std::vector<Instance *> instances = entity->instances();
	std::vector<Instance *> chosen;

	for (Instance *instance : instances)
	{
		const Metadata::KeyValues kv = instance->metadata();

		if (kv.count(header) == 0)
		{
			continue;
		}
		
		chosen.push_back(instance);
	}
	
	std::sort(chosen.begin(), chosen.end(), [header](const Instance *a,
	                                                 const Instance *b)
	{
		const Metadata::KeyValues a_vals = a->metadata();
		const Metadata::KeyValues b_vals = b->metadata();

		return (a_vals.at(header) < b_vals.at(header));
	});
	
	return chosen;
}

void insertBs(Entity *entity, Instance *instance, double *start,
              std::pair<Instance *, std::string> *mapping)
{
	instance->model()->load(Model::NoGeometry);

	int i = -1;
	float total_b = 0;
	float count = 0;
	for (Residue &master : entity->sequence()->residues())
	{
		i++;
		start[i] = NAN;

		Residue *local = instance->equivalentLocal(&master);

		if (!local)
		{
			continue;
		}
		
		AtomGroup *group = instance->currentAtoms();
		Atom *ca = group->atomByIdName(local->id(), "CA");
		
		if (!ca)
		{
			continue;
		}
		
		float b = ca->initialBFactor();
		start[i] = b;
		mapping[i] = {instance, ca->desc()};

		total_b += b;
		count++;
	}
	
	total_b /= count;
	
	for (size_t i = 0; i < entity->sequence()->size(); i++)
	{
//		start[i] /= total_b;
		start[i] /= 60;
		start[i] /= 2;
	}

	instance->unload();
}

void BFactors::drawPlot()
{
	std::unique_lock<std::mutex> lock(_mutex);

	if (_results.vals)
	{
		freeMatrix(&_results);
	}

	std::vector<Instance *> instances = findMolecules(_entity, _header);
	std::cout << "Found " << instances.size() << " instances." << std::endl;
	
	Sequence *master = _entity->sequence();
	int cols = master->size();
	int rows = instances.size();
	
	PCA::Matrix tmp;
	setupMatrix(&tmp, rows, cols);
	_mapping.resize(rows * cols);
	
	for (size_t i = 0; i < instances.size(); i++)
	{
		const Metadata::KeyValues kv = instances[i]->metadata();
		std::cout << "Header: " << kv.at(_header) << std::endl;

		insertBs(_entity, instances[i], tmp.ptrs[i], &_mapping[i * cols]);
	}
	
	_results = PCA::transpose(&tmp);
	
	lock.unlock();
	
	if (_plot)
	{
		removeObject(_plot);
		delete _plot;
	}

	_plot = new MatrixPlot(_results, _mutex);
	_plot->resize(2);
	_plot->update();
	_plot->setCentre(0.5, 0.6);
	
	glm::vec3 min, max;
	_plot->boundaries(&min, &max);
	std::cout << min << " to " << max << std::endl;
	
	float l = fabs(max.y - min.y);
	if (l > 1.0)
	{
		_plot->resize(1.0 / l);
	}

	addObject(_plot);
}

void BFactors::mousePressEvent(double x, double y, SDL_MouseButtonEvent button)
{
	sampleFromPlot(x, y);
	Scene::mousePressEvent(x, y, button);
}

void BFactors::sampleFromPlot(double x, double y)
{
	if (_plot == nullptr)
	{
		return;
	}

	double tx = x; double ty = y;
	convertToGLCoords(&tx, &ty);

	glm::vec3 v = glm::vec3(tx, ty, 0);
	glm::vec3 min, max;
	_plot->boundaries(&min, &max);

	v -= min;
	v /= (max - min);
	v.z = 0;
	
	std::cout << v << std::endl;

	if ((v.x <= 0 || v.x >= 1) || (v.y <= 0 || v.y >= 1))
	{
		return;
	}

	int x_idx = v.x * _results.rows;
	int y_idx = (1 - v.y) * _results.cols;
	int idx = y_idx * _results.rows + x_idx; 

	auto pair = _mapping[idx];
	Instance *instance = pair.first;
	std::string desc = pair.second;
	
	if (instance)
	{
		setInformation(instance->id() + ": " + desc);
	}
}

