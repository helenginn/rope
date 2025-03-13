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

#include "TorsionAxisView.h"
#include "Graph.h"
#include <vagabond/core/ObjectData.h>
#include <vagabond/c4x/ClusterSVD.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/utils/FileReader.h>

TorsionAxisView::TorsionAxisView(Scene *prev, ObjectData *data,
                                 ClusterSVD *cluster, int axis)
: Scene(prev)
{
	_data = data;
	_cluster = cluster;
	_axis = axis;

	setExportHandler(this);
}

void TorsionAxisView::loadIntoGraph(Graph *graph)
{
	// extremely slow but gets round the variable type problem
	std::ostringstream ss;
	_data->rawVectorToCSV(_cluster, _axis, ss);
	std::string result = ss.str();

	std::vector<std::string> lines = split(result, '\n');
	
	bool first = true;
	int n = 0;
	for (const std::string &line : lines)
	{
		if (first) { first = false; continue; }
		std::vector<std::string> bits = split(line, ',');
		if (bits.size() < 2)
		{
			continue;
		}

		bool first_bit = true;
		int m = 0;
		for (const std::string &bit : bits)
		{
			if (first_bit) { first_bit = false; continue; }
			float val = atof(bit.c_str());
			
			graph->addPoint(m, n, val);
			m++;
		}
		n++;
	}
}

void TorsionAxisView::setup()
{
	addTitle("Torsion axis view");
	
	TextButton *t = new TextButton("Export", this);
	t->setRight(0.9, 0.1);
	t->setReturnJob([this]()
	                { 
		               ExportsCSV::buttonPressed("export", nullptr);
		            });
	addObject(t);

	Graph *graph = new Graph();

	graph->setAxisLabel('x', _data->header_for_entry_in_csv_file());
	graph->setAxisLabel('y', _data->y_axis_label());
	
	loadIntoGraph(graph);
	graph->setSeriesColour(1, {0.6, 0.2, 0.6});
	graph->setSeriesColour(2, {0.6, 0.6, 0.2});

	graph->setup(0.6, 0.5);
	graph->addToGraphPosition(0.5, 0.5);

	addObject(graph);
}

void TorsionAxisView::supplyCSV(std::string indicator)
{
	_csv = "";
	_csv += _data->csvFirstLine() + "\n";
	
	std::ostringstream ss;
	_data->rawVectorToCSV(_cluster, _axis, ss);
	_csv += ss.str();

}

void TorsionAxisView::buttonPressed(std::string tag, Button *button)
{
	ExportsCSV::buttonPressed(tag, button);
	Scene::buttonPressed(tag, button);
}
